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
#include "dev/leds.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
static struct broadcast_conn broadcast;

/*---------------------------------------------------------------------------*/
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
    printf("broadcast message received from %d.%d: '%s'\n",
            from->u8[0], from->u8[1], (char *) packetbuf_dataptr());
    /*char * new_str;
    new_str[0] = '\0'; // ensures the memory is an empty string
    strcat(new_str, packetbuf_dataptr());
    strcat(new_str, (char *) from->u8);
    printf("Going to forward (bcst): '%s'\n", new_str);
    */packetbuf_copyfrom((char *) packetbuf_dataptr(), packetbuf_datalen());
    broadcast_send(&broadcast);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data) {
    PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
    PROCESS_BEGIN();
    broadcast_open(&broadcast, 129, &broadcast_call);
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
