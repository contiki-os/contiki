
#include "net/rime/uabc.h"
#include "contiki.h"
/*---------------------------------------------------------------------------*/
PROCESS(test_uabc_process, "");
AUTOSTART_PROCESSES(&test_uabc_process);
/*---------------------------------------------------------------------------*/
static void
recv(struct uabc_conn *c)
{
  printf("recv\n");
}
static void
sent(struct uabc_conn *c)
{
  printf("sent\n");
}
static void
dropped(struct uabc_conn *c)
{
  printf("dropped\n");
}
static const struct uabc_callbacks callbacks = { recv, sent, dropped };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_uabc_process, ev, data)
{
  static struct uabc_conn c;
  PROCESS_BEGIN();

  uabc_open(&c, 128, &callbacks);

  while(1) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    rimebuf_copyfrom("Hej", 4);
    uabc_send(&c, CLOCK_SECOND);
  
  
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
