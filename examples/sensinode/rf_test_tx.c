/**
 * \file
 *         RF test suite, transmitter
 * \author
 *         Zach Shelby <zach@sensinode.com>
 */

#include "contiki.h"
#include "net/rime.h"
#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(rf_test_process, "RF test TX process");
AUTOSTART_PROCESSES(&rf_test_process);

static struct etimer et;
static struct broadcast_conn bc;
static struct unicast_conn uc;
rimeaddr_t addr;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rf_test_process, ev, data)
{

  PROCESS_BEGIN();

  printf("\nStarting CC2430 RF test suite...\n");

  broadcast_open(&bc, 128, 0);
  unicast_open(&uc, 128, 0);

  while(1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    printf("Sending broadcast packet\n");
    packetbuf_copyfrom("Hello everyone", 14);
    broadcast_send(&bc);

 // TODO: Fix, freezes on unicast_send()
 //   printf("Sending unicast packet\n");
 //   addr.u8[0] = 0;
 //   addr.u8[1] = 2;
 //   packetbuf_copyfrom("Hello you", 9);
 //   unicast_send(&uc, &addr);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
