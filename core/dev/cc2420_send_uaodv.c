#include <stdio.h>

#include "contiki.h"

#include "net/uip.h"
#include "net/uip-fw.h"
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#include "dev/leds.h"

#include "dev/cc2420.h"

#include "net/uaodv.h"
#include "net/uaodv-rt.h"

#define in_my_network(a) \
  (((a[0] ^ cc2420if.ipaddr.u16[0]) & cc2420if.netmask.u16[0]) == 0 && \
   ((a[1] ^ cc2420if.ipaddr.u16[1]) & cc2420if.netmask.u16[1]) == 0)

u8_t
cc2420_send_uaodv(void)
{
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

    if (cc2420_check_remote(next_gw->u16[1]) == 0)
      h.dst = next_gw->u16[1];	/* local, use ucast */
    else {			/* remote or unknown */
      struct uaodv_rt_entry *route = uaodv_request_route_to(next_gw);

      if (route == NULL) {
	h.dst = next_gw->u16[1]; /* try local while waiting for route */
      } else {
	if (cc2420_check_remote(route->nexthop.u16[1]) == 1) {
	  printf("LOST 0x%04x\n", route->nexthop.u16[1]);
	  /* Send bad route notification? */
#ifdef UAODV_BAD_ROUTE
	  uaodv_bad_route(route);
#endif
	  uaodv_rt_remove(route);  
	  h.dst = 0xffff;	/* revert to bcast */
	} else /* unknown */ {
	  /* This will implicitly update neigbour table. */
	  h.dst = route->nexthop.u16[1];
	}
      }
    }
  }

  /* Don't request MAC level ACKs for broadcast packets. */
  if (h.dst == 0xffff)
    h.fc0 &= ~FC0_REQ_ACK;

  if (cc2420_send(&h, 10, &uip_buf[UIP_LLH_LEN], uip_len) < 0) {
    printf("cc2420_send_uaodv too big uip_len=%d\n", uip_len);
    leds_toggle(color);
    return UIP_FW_TOOLARGE;
  }

  leds_toggle(color);

  return UIP_FW_OK;
}
