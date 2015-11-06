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
//#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "cc2420.h"
#include <stdio.h>
#include <string.h>

#if WITH_ORCHESTRA
#include "orchestra.h"
#endif /* WITH_ORCHESTRA */

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define CONFIG_VIA_BUTTON PLATFORM_HAS_BUTTON
#if CONFIG_VIA_BUTTON
#include "button-sensor.h"
#endif /* CONFIG_VIA_BUTTON */

//NEEDED TO SPECIFY TO EACH FILE DIFRENT PAN-ID
/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0x001F

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
#if CONFIG_VIA_BUTTON
AUTOSTART_PROCESSES(&node_process, &sensors_process);
#else /* CONFIG_VIA_BUTTON */
AUTOSTART_PROCESSES(&node_process);
#endif /* CONFIG_VIA_BUTTON */

/*---------------------------------------------------------------------------*/
static void
print_network_status(void) {
    int i;
    uint8_t state;
    uip_ds6_defrt_t *default_route;
    uip_ds6_route_t *route;

    //printf("--- Network status ---\n");
    /* Our IPv6 addresses */
    //printf("- Server IPv6 addresses:\n");
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
        //printf(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
    } else {
        //printf("-- None\n");
    }

    /* Our routing entries */
    //printf("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
    route = uip_ds6_route_head();
    while (route != NULL) {
        //printf("-- ");
        uip_debug_ipaddr_print(&route->ipaddr);
        //printf(" via ");
        uip_debug_ipaddr_print(uip_ds6_route_nexthop(route));
        //printf(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
        route = uip_ds6_route_next(route);
    }

    //printf("----------------------\n");
}

/*---------------------------------------------------------------------------*/
static void net_init(uip_ipaddr_t *br_prefix) {
    uip_ipaddr_t global_ipaddr;
    if (br_prefix) { /* We are RPL root. Will be set automatically as TSCH pan coordinator via the tsch-rpl module */
        memcpy(&global_ipaddr, br_prefix, 16);
        uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
        uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
/////////////////////////////////////⁷((((((((((((((((((((((((((((((((((        
	rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
        rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
        rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
    NETSTACK_MAC.on();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data) {
    static struct etimer et;
    PROCESS_BEGIN();

    //Button management
    SENSORS_ACTIVATE(button_sensor);
    
    /* 3 possible roles:
     * - role_6ln: simple node, will join any network, secured or not
     * - role_6dr: DAG root, will advertise (unsecured) beacons
     * - role_6dr_sec: DAG root, will advertise secured beacons
     * */
    static int is_coordinator = 0;

    static enum {
        role_6ln, role_6dr, role_6dr_sec
    } node_role;
    node_role = role_6ln;

    /* Set node with ID == 1 as coordinator, convenient in Cooja. */
    if (node_id == 1) {
        if (LLSEC802154_CONF_SECURITY_LEVEL) {
            node_role = role_6dr_sec;
        } else {
            node_role = role_6dr;
        }
    } else {
        node_role = role_6ln;
    }

    /*
    #if CONFIG_VIA_BUTTON
      {
    #define CONFIG_WAIT_TIME 5
 /*
        SENSORS_ACTIVATE(button_sensor);
        etimer_set(&et, CLOCK_SECOND * CONFIG_WAIT_TIME);

        while(!etimer_expired(&et)) {
          //printf("Init: current role: %s. Will start in %u seconds. Press user button to toggle mode.\n",
                    node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec",
                    CONFIG_WAIT_TIME);
          PROCESS_WAIT_EVENT_UNTIL(((ev == sensors_event) &&
                                    (data == &button_sensor) && button_sensor.value(0) > 0)
                                   || etimer_expired(&et));
          if(ev == sensors_event && data == &button_sensor && button_sensor.value(0) > 0) {
            node_role = (node_role + 1) % 3;
            if(LLSEC802154_CONF_SECURITY_LEVEL == 0 && node_role == role_6dr_sec) {
              node_role = (node_role + 1) % 3;
            }
            etimer_restart(&et);
          }
        }
      }

    #endif */
    /* CONFIG_VIA_BUTTON */


    //printf("Init: node starting with role %s\n", node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec");

    tsch_set_pan_secured(LLSEC802154_CONF_SECURITY_LEVEL && (node_role == role_6dr_sec));
    is_coordinator = node_role > role_6ln;

    if (is_coordinator) {
        uip_ipaddr_t prefix;
        uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
        net_init(&prefix);
    } else {
        net_init(NULL);
    }

#if WITH_ORCHESTRA
    orchestra_init();
#endif /* WITH_ORCHESTRA */

    /* Print out routing tables every minute */
    etimer_set(&et, CLOCK_SECOND * 60);
    while (1) {
        print_network_status();
        
        PROCESS_YIELD_UNTIL(etimer_expired(&et));
        etimer_reset(&et);
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
