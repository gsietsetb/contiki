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
#if WITH_ORCHESTRA
#include "orchestra.h"
#endif /* WITH_ORCHESTRA */

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

/*---------------------------------------------------------------------------*/
static void print_network_status(void) {
    int i;
    uint8_t state;
    uip_ds6_defrt_t *default_route;
    uip_ds6_route_t *route;

    //PRINTA("--- Network status ---\n");

    /* Our IPv6 addresses */
    //PRINTA("- Server IPv6 addresses:\n");
    for (i = 0; i < UIP_DS6_ADDR_NB; i++) {
        state = uip_ds6_if.addr_list[i].state;
        if (uip_ds6_if.addr_list[i].isused &&
                (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
            //PRINTA("-- ");
            uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
            //PRINTA("\n");
        }
    }

    /* Our default route */
    //PRINTA("- Default route:\n");
    default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
    if (default_route != NULL) {
        //PRINTA("-- ");
        uip_debug_ipaddr_print(&default_route->ipaddr);;
        //PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
    } else {
        //PRINTA("-- None\n");
    }

    /* Our routing entries */
    //PRINTA("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
    route = uip_ds6_route_head();
    while (route != NULL) {
        //PRINTA("-- ");
        uip_debug_ipaddr_print(&route->ipaddr);
        //PRINTA(" via ");
        uip_debug_ipaddr_print(uip_ds6_route_nexthop(route));
        //PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
        route = uip_ds6_route_next(route);
    }

    //PRINTA("----------------------\n");
}

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

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data) {
    static struct etimer et;
    PROCESS_BEGIN();

    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    net_init(&prefix);


#if WITH_ORCHESTRA
    //orchestra_init();
#endif /* WITH_ORCHESTRA */

    /* Print out routing tables every minute */
    //etimer_set(&et, CLOCK_SECOND * 60);
    while (1) {
        print_network_status();
        PROCESS_YIELD_UNTIL(etimer_expired(&et));
        //etimer_reset(&et);
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
