#include "net/mac/rdc.h"
#include "net/mac/tsch/tsch.h"
#include "net/netstack.h"

/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  /* We never turn off TSCH.
   * However, when called with the flag keep_radio_on set,
   * interpret this as starting as PAN coordinator, as
   * expected by Contiki's upper layers. */
  if(keep_radio_on) {
    tsch_set_coordinator(1);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver tschrdc_driver = {
  "TSCH-rdc",
  init,
  send_packet,
  send_list,
  packet_input,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
