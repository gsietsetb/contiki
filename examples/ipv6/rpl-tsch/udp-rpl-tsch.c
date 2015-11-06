/**
 * \file
 *         A RPL+TSCH node able to act as either a simple node (6ln),
 *         DAG Root (6dr) or DAG Root with security (6dr-sec)
 *         Press use button at startup to configure.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

/***************
 *   INCLUDES   |
 **************/

#include "contiki.h"
#include "node-id.h"
#include "net/rpl/rpl.h"
//#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "cc2420.h"
#include <stdio.h>
#include <string.h>
#include "simple-udp.h"

//NEEDED TO SPECIFY TO EACH FILE DIFRENT PAN-ID
/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0x001F

#define UDP_PORT 1234

static struct simple_udp_connection broadcast_connection;
uip_ipaddr_t global_ipaddr;
uip_ipaddr_t prefix;

/*---------------------------------------------------------------------------*/
PROCESS(Udp_Rpl_TSCH, "Init Root RPL Node");
PROCESS(check_Button, "Check button process");
AUTOSTART_PROCESSES(&Udp_Rpl_TSCH, &sensors_process);

/************************************************************************************************************
 *                                                 Functions                                                 *
 ************************************************************************************************************/
static void print_network_status(void) {
    int i;
    uint8_t state;
    uip_ds6_defrt_t *default_route;
    uip_ds6_route_t *route;

    //printf("--- Network status ---\n");
    /* Our IPv6 addresses */
    printf("- Server IPv6 addresses:\n");
    for (i = 0; i < UIP_DS6_ADDR_NB; i++) {
        state = uip_ds6_if.addr_list[i].state;
        if (uip_ds6_if.addr_list[i].isused &&
                (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
            //printf("-- ");
            uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
            //printf("\n");
        }
    }

    /* Our default route */
    //printf("- Default route:\n");
    default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
    if (default_route != NULL) {
        //printf("-- ");
        uip_debug_ipaddr_print(&default_route->ipaddr);
        printf(" (lifetime: %lu seconds)\n", (unsigned long) default_route->lifetime.interval);
    } else {
        //printf("-- None\n");
    }

    /* Our routing entries */
    //printf("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
    route = uip_ds6_route_head();
    while (route != NULL) {
        //printf("-- ");
        uip_debug_ipaddr_print(&route->ipaddr);
        printf(" via ");
        uip_debug_ipaddr_print(uip_ds6_route_nexthop(route));
        printf(" (lifetime: %lu seconds)\n", (unsigned long) route->state.lifetime);
        route = uip_ds6_route_next(route);
    }

    //printf("----------------------\n");
}

static void receiver(struct simple_udp_connection *c,
        const uip_ipaddr_t *sender_addr,
        uint16_t sender_port,
        const uip_ipaddr_t *receiver_addr,
        uint16_t receiver_port,
        const uint8_t *data,
        uint16_t datalen) {
    printf("Data received on port %d from port %d with length %d\n",
            receiver_port, sender_port, datalen);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(Udp_Rpl_TSCH, ev, data) {
    static struct etimer et;
    PROCESS_BEGIN();

    /************************************************************************************************************
     *                                                 INIT                                                    *
     ************************************************************************************************************/
    /*  UDP */
    simple_udp_register(&broadcast_connection, UDP_PORT, NULL, UDP_PORT, receiver);

    /* (uIP)   */

    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    memcpy(&global_ipaddr, &prefix, 16);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr); //set the last 64 bits of an IP address based on the MAC address 
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);

    /*  RPL    */
    rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(rpl_get_any_dag(), &prefix, 64);
    rpl_repair_root(RPL_DEFAULT_INSTANCE);

    /*  6LoWPAN  */
    /* 
     * A LoWPAN encapsulated IPv6 datagram:

      +---------------+-------------+---------+
      | IPv6 Dispatch | IPv6 Header | Payload |
      +---------------+-------------+---------+

   A LoWPAN encapsulated LOWPAN_HC1 compressed IPv6 datagram:

      +--------------+------------+---------+
      | HC1 Dispatch | HC1 Header | Payload |
      +--------------+------------+---------+

   A LoWPAN encapsulated LOWPAN_HC1 compressed IPv6 datagram that
   requires mesh addressing:

      +-----------+-------------+--------------+------------+---------+
      | Mesh Type | Mesh Header | HC1 Dispatch | HC1 Header | Payload |
      +-----------+-------------+--------------+------------+---------+

   A LoWPAN encapsulated LOWPAN_HC1 compressed IPv6 datagram that
   requires fragmentation:

      +-----------+-------------+--------------+------------+---------+
      | Frag Type | Frag Header | HC1 Dispatch | HC1 Header | Payload |
      +-----------+-------------+--------------+------------+---------+

   A LoWPAN encapsulated LOWPAN_HC1 compressed IPv6 datagram that
   requires both mesh addressing and fragmentation:

      +-------+-------+-------+-------+---------+---------+---------+
      | M Typ | M Hdr | F Typ | F Hdr | HC1 Dsp | HC1 Hdr | Payload |
      +-------+-------+-------+-------+---------+---------+---------+

   A LoWPAN encapsulated LOWPAN_HC1 compressed IPv6 datagram that
   requires both mesh addressing and a broadcast header to support mesh
   broadcast/multicast:

      +-------+-------+-------+-------+---------+---------+---------+
      | M Typ | M Hdr | B Dsp | B Hdr | HC1 Dsp | HC1 Hdr | Payload |
      +-------+-------+-------+-------+---------+---------+---------+

     * 
     */

    /*  6TiSCH    */
#if WITH_ORCHESTRA
    orchestra_init();
#endif /* WITH_ORCHESTRA */

    /*  TSCH    */
    //tsch_set_pan_secured(LLSEC802154_CONF_SECURITY_LEVEL && (node_role == role_6dr_sec));

    /*  MAC    */
    NETSTACK_MAC.on();
    process_start(&check_Button, NULL);

    while (1) {
        /* Print out routing tables every minute */
        etimer_set(&et, CLOCK_SECOND * 60);
        print_network_status();

        PROCESS_YIELD_UNTIL(etimer_expired(&et));
        etimer_reset(&et);
    }

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(check_Button, ev, data) {
    PROCESS_BEGIN();
    /************************************************************************************************************
     *                                                 SENDER                                                   *
     ************************************************************************************************************/

    /*Button management */
    SENSORS_ACTIVATE(button_sensor);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor); //wait till button is pressed
        leds_toggle(LEDS_ALL); //leds on
        printf("we here!");

        /* (uIP)   */
        uip_create_linklocal_allnodes_mcast(&global_ipaddr);

        /*  UDP    */
        simple_udp_sendto(&broadcast_connection, "Test", 4, &global_ipaddr);

        /*  RPL    */


        /*  6LoWPAN    */


        /*  6TiSCH    */


        /*  TSCH    */


        /*  MAC    */

        leds_toggle(LEDS_ALL); //leds on

    }

    PROCESS_END();

}