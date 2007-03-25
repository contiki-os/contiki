
#include "net/rime/uibc.h"
#include "contiki.h"
/*---------------------------------------------------------------------------*/
PROCESS(test_uibc_process, "");
AUTOSTART_PROCESSES(&test_uibc_process);
/*---------------------------------------------------------------------------*/
static void
recv(struct uibc_conn *c, rimeaddr_t *from)
{
  printf("recv '%s' from %d.%d\n", rimebuf_dataptr(),
	 from->u8[0], from->u8[1]);
}
static void
sent(struct uibc_conn *c)
{
  printf("sent\n");
}
static void
dropped(struct uibc_conn *c)
{
  printf("dropped\n");
}
static const struct uibc_callbacks callbacks = { recv, sent, dropped };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_uibc_process, ev, data)
{
  static struct uibc_conn c;

  PROCESS_EXITHANDLER(uibc_close(&c));
  
  PROCESS_BEGIN();

  uibc_open(&c, 128, &callbacks);

  while(1) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    rimebuf_copyfrom("Hej", 4);
    uibc_send(&c, CLOCK_SECOND * 4);
  
  
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
