/*
 * \file
 *         RICH node.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/rpl/rpl-private.h"
#include "net/ip/uip-debug.h"
#include "lib/random.h"
#include "tools/rich.h"
//#include "node-id.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RICH Node");
AUTOSTART_PROCESSES(&node_process);

/*---------------------------------------------------------------------------*/
/*static void net_init(uip_ipaddr_t *br_prefix) {
    uip_ipaddr_t global_ipaddr;
    if (br_prefix) { / We are RPL root. Will be set automatically as TSCH pan coordinator via the tsch-rpl module*
        memcpy(&global_ipaddr, br_prefix, 16);
        uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
        uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
        rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
        rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
        rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
    NETSTACK_MAC.on();
}*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data){
  PROCESS_BEGIN();
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    //rich_init(&prefix);
    //net_init(&prefix);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
