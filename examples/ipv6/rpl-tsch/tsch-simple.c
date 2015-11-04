/*
 */
/**
 * \file
 *         A RPL+TSCH node able to act as either a simple node (6ln),
 *         DAG Root (6dr) or DAG Root with security (6dr-sec)
 *         Press use button at startup to configure.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "node-id.h"
#include "net/rpl/rpl.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "simple-udp.h"

#define UDP_PORT 1234

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define CONFIG_VIA_BUTTON PLATFORM_HAS_BUTTON
#if CONFIG_VIA_BUTTON
#include "button-sensor.h"
#endif /* CONFIG_VIA_BUTTON */

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
#if CONFIG_VIA_BUTTON
AUTOSTART_PROCESSES(&node_process, &sensors_process);
#else /* CONFIG_VIA_BUTTON */
AUTOSTART_PROCESSES(&node_process);
#endif /* CONFIG_VIA_BUTTON */

static struct simple_udp_connection broadcast_connection;

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
static void net_init(uip_ipaddr_t *br_prefix) {
    uip_ipaddr_t global_ipaddr;

    if (br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
        memcpy(&global_ipaddr, br_prefix, 16);
        uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
        uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
        rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
        rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
        rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
    NETSTACK_MAC.on();
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
PROCESS_THREAD(node_process, ev, data) {
    static struct etimer et;
    PROCESS_BEGIN();

    simple_udp_register(&broadcast_connection, UDP_PORT,
            NULL, UDP_PORT,
            receiver);
    uip_ipaddr_t addr;
    uip_ip6addr(&addr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    net_init(&addr);
    simple_udp_register(&broadcast_connection, UDP_PORT,
            NULL, UDP_PORT,
            receiver);

    /* Print out routing tables every 10sec <</>> minute */
    etimer_set(&et, CLOCK_SECOND * 10);
    while (1) {
        //print_network_status();
        printf("Sending broadcast\n");
        uip_create_linklocal_allnodes_mcast(&addr);
        simple_udp_sendto(&broadcast_connection, "Test", 4, &addr);
        //Timer
        PROCESS_YIELD_UNTIL(etimer_expired(&et));
        etimer_reset(&et);
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
