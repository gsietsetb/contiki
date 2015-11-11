/**
 * \file
 *         Testing the broadcast layer in Rime
 * \author
 *         Sierra Aiello, Guillermo <gsie.etsetb@gmail.com>
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/light-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(src_Bcast_process, "Source Broadcast Send");
AUTOSTART_PROCESSES(&src_Bcast_process);
static struct broadcast_conn broadcast;

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
    printf("Why da fuq I received a msg? :O from:  %d.%d: '%s'\n",
            from->u8[0], from->u8[1], (char *) packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(src_Bcast_process, ev, data) {
    PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
    PROCESS_BEGIN();
    SENSORS_ACTIVATE(button_sensor);
          SENSORS_ACTIVATE(light_sensor);
    broadcast_open(&broadcast, 129, &broadcast_call);
    while (1) {
        PROCESS_WAIT_EVENT();
        if (ev == sensors_event && data == &button_sensor) {
            packetbuf_copyfrom(light_sensor.value(0), 2);
            broadcast_send(&broadcast);
            printf("\nBroadcast message sent (LIGHT = %d Lumens)\n", light_sensor.value(0));
        }
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
