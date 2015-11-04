/*
 *
 * Simple application to indicate connectivity between two nodes:
 *
 * - Red led indicates a packet sent via tsch_radio (one packet sent each second)
 * - Green led indicates that this node can hear the other node but not
 *   necessary vice versa (unidirectional communication).
 * - Blue led indicates that both nodes can communicate with each
 *   other (bidirectional communication)
 */

#include "contiki.h"
//#include "net/rime/rime.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "cc2420.h"
#include <stdio.h>
#include <string.h>

//_____________________________________

#include "contiki-conf.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/rpl/rpl-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/ip/uip-debug.h"
#include "lib/random.h"
#include "net/mac/tsch/tsch-rpl.h"
#include "deployment.h"
#include "simple-udp.h"
#include "tools/orchestra.h"
#include <stdio.h>

PROCESS(tsch_radio_test_process, "TSCH test");
AUTOSTART_PROCESSES(&tsch_radio_test_process);

#define ON  1
#define OFF 0

#define HEADER "RTST"
#define PACKET_SIZE 20
#define PORT 9345

#define SEND_INTERVAL   (60*CLOCK_SECOND)
#define UDP_PORT 1234

struct indicator {
  int onoff;
  int led;
  clock_time_t interval;
  struct etimer timer;
};
static struct etimer send_timer;
static struct indicator recv, other, flash;

/*---------------------------------------------------------------------*/
static void set(struct indicator *indicator, int onoff) {
  if(indicator->onoff ^ onoff) {
    indicator->onoff = onoff;
    if(onoff) {
      leds_on(indicator->led);
    } else {
      leds_off(indicator->led);
    }
  }
  if(onoff) {
    etimer_set(&indicator->timer, indicator->interval);
  }
}
/*---------------------------------------------------------------------------*/
static void abc_recv(struct abc_conn *c){
  /* packet received */
  if(packetbuf_datalen() < PACKET_SIZE
     || strncmp((char *)packetbuf_dataptr(), HEADER, sizeof(HEADER))) {
    /* invalid message */

  } else {
    PROCESS_CONTEXT_BEGIN(&tsch_radio_test_process);
    set(&recv, ON);
    set(&other, ((char *)packetbuf_dataptr())[sizeof(HEADER)] ? ON : OFF);

    /* synchronize the sending to keep the nodes from sending
       simultaneously */

    etimer_set(&send_timer, CLOCK_SECOND);
    etimer_adjust(&send_timer, - (int) (CLOCK_SECOND >> 1));
    PROCESS_CONTEXT_END(&tsch_radio_test_process);
  }
}
static const struct abc_callbacks abc_call = {abc_recv};
static struct abc_conn abc;
/*---------------------------------------------------------------------*/
PROCESS_THREAD(tsch_radio_test_process, ev, data){
  static uint8_t txpower;
  PROCESS_BEGIN();

  txpower = CC2420_TXPOWER_MAX;

  /* Initialize the indicators */
  recv.onoff = other.onoff = flash.onoff = OFF;
  recv.interval = other.interval = CLOCK_SECOND;
  flash.interval = 1;
  flash.led = LEDS_RED;
  recv.led = LEDS_GREEN;
  other.led = LEDS_BLUE;

  abc_open(&abc, PORT, &abc_call);
  etimer_set(&send_timer, CLOCK_SECOND);
  SENSORS_ACTIVATE(button_sensor);

  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      if(data == &send_timer) {
	etimer_reset(&send_timer);

	/* send packet */
	packetbuf_copyfrom(HEADER, sizeof(HEADER));
	((char *)packetbuf_dataptr())[sizeof(HEADER)] = recv.onoff;
	/* send arbitrary data to fill the packet size */
	packetbuf_set_datalen(PACKET_SIZE);
	set(&flash, ON);
	abc_send(&abc);

      } else if(data == &other.timer) {
	set(&other, OFF);

      } else if(data == &recv.timer) {
	set(&recv, OFF);

      } else if(data == &flash.timer) {
	set(&flash, OFF);
      }
    } else if(ev == sensors_event && data == &button_sensor) {
      if(txpower > 5) {
	txpower -= 5;
      } else {
	txpower = CC2420_TXPOWER_MAX;
	leds_blink();
      }
      cc2420_set_txpower(txpower);
      printf("txpower set to %u\n", txpower);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*//*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*//*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*//*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*//*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*//*----------------/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/



static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_sender_process, "Collect-only Application");
AUTOSTART_PROCESSES(&unicast_sender_process);
/*---------------------------------------------------------------------------*/
static void receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen){
  LOGA((void*)data, "App: received");
}
/*---------------------------------------------------------------------------*/
int can_send_to(uip_ipaddr_t *ipaddr) {
  return uip_ds6_is_addr_onlink(ipaddr)
      || uip_ds6_route_lookup(ipaddr)
      || uip_ds6_defrt_choose();
}
/*---------------------------------------------------------------------------*/
int app_send_to(uint16_t id, uint32_t seqno, unsigned int to_send_cnt){

  struct app_data data;
  uip_ipaddr_t dest_ipaddr;

  data.magic = UIP_HTONL(LOG_MAGIC);
  data.seqno = UIP_HTONL(seqno);
  data.src = UIP_HTONS(node_id);
  data.dest = UIP_HTONS(id);
  data.hop = 0;

  set_ipaddr_from_id(&dest_ipaddr, id);

  if(can_send_to(&dest_ipaddr)) {
    LOGA(&data, "App: sending");
    simple_udp_sendto(&unicast_connection, &data, sizeof(data), &dest_ipaddr);
    return 1;
  } else {
    data.seqno = UIP_HTONL(seqno + to_send_cnt - 1);
    LOGA(&data, "App: could not send");
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_sender_process, ev, data){
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  uip_ipaddr_t global_ipaddr;
  static unsigned int cnt;
  static unsigned int to_send_cnt;
  static uint32_t seqno;

  PROCESS_BEGIN();

  if(deployment_init(&global_ipaddr, NULL, ROOT_ID)) {
    //LOG("App: %u start\n", node_id);
  } else {
    PROCESS_EXIT();
  }
  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);
  
  
  tsch_schedule_create_minimal();

/*#if WITH_TSCH
if WITH_ORCHESTRA
  orchestra_init();
#else
#endif
   tsch_schedule_create_minimal();
#endif*/

  if(node_id != ROOT_ID) {
    etimer_set(&periodic_timer, SEND_INTERVAL);
    while(1) {
      etimer_set(&send_timer, random_rand() % (SEND_INTERVAL));
      PROCESS_WAIT_UNTIL(etimer_expired(&send_timer));

      if(default_instance != NULL) {
        to_send_cnt++;
        while(to_send_cnt > 0) {
          seqno = ((uint32_t)node_id << 16) + cnt;
          if(app_send_to(ROOT_ID, seqno, to_send_cnt)) {
            cnt++;
            to_send_cnt--;
            if(to_send_cnt > 0) {
              etimer_set(&send_timer, CLOCK_SECOND);
              PROCESS_WAIT_UNTIL(etimer_expired(&send_timer));
            }
          } else {
            break;
          }
        }
      } else {
        LOG("App: no DODAG\n");
      }
      PROCESS_WAIT_UNTIL(etimer_expired(&periodic_timer));
      etimer_reset(&periodic_timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

