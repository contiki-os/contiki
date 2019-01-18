#include <stdio.h>

#include "contiki.h"

#include "net/uip.h"
#include "net/uip-fw.h"
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#include "dev/leds.h"

#include "dev/cc2420.h"

#define in_my_network(a) \
  (((a[0] ^ cc2420if.ipaddr.u16[0]) & cc2420if.netmask.u16[0]) == 0 && \
   ((a[1] ^ cc2420if.ipaddr.u16[1]) & cc2420if.netmask.u16[1]) == 0)

u8_t
cc2420_send_ip(void)
{
  int ret;
  struct hdr_802_15 h;

  u8_t color = (cc2420_ack_received) ? LEDS_BLUE : LEDS_RED;
  leds_toggle(color);

  h.len = MAC_HDR_LEN + uip_len + 2; /* Including footer[2]. */
  h.fc0 = FC0_TYPE_DATA | FC0_REQ_ACK | FC0_INTRA_PAN;
  h.fc1 = FC1_DST_16 | FC1_SRC_16;

  h.src = uip_hostaddr.u16[1];
  if (uip_ipaddr_cmp(&BUF->destipaddr, &uip_broadcast_addr))
    h.dst = 0xffff;
  else {
    uip_ipaddr_t *next_gw;

    if (in_my_network(BUF->destipaddr.u16))
      next_gw = &BUF->destipaddr;
    else
      next_gw = &uip_draddr;	/* Default router. */

    if (cc2420_check_remote(next_gw->u16[1]) == REMOTE_YES)
      h.dst = 0xffff;		/* remote, use bcast */
    else
      h.dst = next_gw->u16[1];	/* local or unknown, use ucast */
  }

  /* Don't request MAC level ACKs for broadcast packets. */
  if (h.dst == 0xffff)
    h.fc0 &= ~FC0_REQ_ACK;

  ret = cc2420_send(&h, 10, &uip_buf[UIP_LLH_LEN], uip_len);
  if (ret < 0) {
    printf("cc2420_send_ip failed uip_len=%d ret=%d\n", uip_len, ret);
    leds_toggle(color);
    return UIP_FW_TOOLARGE;
  }

  leds_toggle(color);

  return UIP_FW_OK;
}
