/**
 * \file
 *         RF test suite, receiver
 * \author
 *         Zach Shelby <zach@sensinode.com>
 */

#include "contiki.h"
#include "net/rime.h"
#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(rf_test_process, "RF test RX process");
AUTOSTART_PROCESSES(&rf_test_process);

static struct etimer et;
static struct broadcast_conn bc;
static const struct broadcast_callbacks broadcast_callbacks = {recv_bc};
static struct unicast_conn uc;
static const struct unicast_callbacks unicast_callbacks = {recv_uc};

static void
recv_bc(struct broadcast_conn *c, rimeaddr_t *from)
{
  printf("broadcast from %02x.%02x len = %d buf = %s\n",
		  from->u8[0],
		  from->u8[1],
		  packetbuf_datalen(),
		  (char *)packetbuf_dataptr());
}

static void
recv_uc(struct unicast_conn *c, rimeaddr_t *from)
{
  printf("unicast from %02x.%02x len = %d buf = %s\n",
		  from->u8[0],
		  from->u8[1],
		  packetbuf_datalen(),
		  (char *)packetbuf_dataptr());
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rf_test_process, ev, data)
{

  PROCESS_BEGIN();

  printf("\nStarting CC2430 RF test suite...\n");

  broadcast_open(&bc, 128, &broadcast_callbacks);
  unicast_open(&uc, 128, &unicast_callbacks);

  while(1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
