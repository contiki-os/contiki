#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include <string.h>
#include "ip64-eth-interface.h"

#define UIP_IP_BUF        ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

static void
init(void)
{
  PRINTF("eth-bridge: init\n");
  ip64_eth_interface.init();
}
/*---------------------------------------------------------------------------*/
static int
output()
{
  PRINTF("eth-bridge: src=");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" dst=");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");
  ip64_eth_interface.output();
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct uip_fallback_interface rpl_interface = {
  init, output
};
/*---------------------------------------------------------------------------*/
