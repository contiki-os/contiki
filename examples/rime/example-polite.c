
#include "net/rime/polite.h"
#include "contiki.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_polite_process, "");
AUTOSTART_PROCESSES(&example_polite_process);
/*---------------------------------------------------------------------------*/
static void
recv(struct polite_conn *c)
{
  printf("recv '%s'\n", (char *)packetbuf_dataptr());
}
static void
sent(struct polite_conn *c)
{
  printf("sent\n");
}
static void
dropped(struct polite_conn *c)
{
  printf("dropped\n");
}
static const struct polite_callbacks callbacks = { recv, sent, dropped };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_polite_process, ev, data)
{
  static struct polite_conn c;

  PROCESS_EXITHANDLER(polite_close(&c));
  
  PROCESS_BEGIN();

  polite_open(&c, 136, &callbacks);

  while(1) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    packetbuf_copyfrom("Hej", 4);
    polite_send(&c, CLOCK_SECOND * 4, 4);
  
  
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
