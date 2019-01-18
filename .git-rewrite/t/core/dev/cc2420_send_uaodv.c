#include <stdio.h>

#include "contiki.h"

#include "net/uip.h"
#include "net/uip-fw.h"
#define BUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

#include "dev/leds.h"

#include "dev/cc2420.h"

#include "net/uaodv.h"
#include "net/uaodv-rt.h"
#include "net/uaodv-def.h"

#define NDEBUG
#include "lib/assert.h"

#ifdef NDEBUG
#define PRINTF(...) do {} while (0)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#endif

#define in_my_network(a) \
  (((a[0] ^ cc2420if.ipaddr.u16[0]) & cc2420if.netmask.u16[0]) == 0 && \
   ((a[1] ^ cc2420if.ipaddr.u16[1]) & cc2420if.netmask.u16[1]) == 0)

u8_t
cc2420_send_uaodv(void)
{
  int ret;
  struct hdr_802_15 h;

  h.len = MAC_HDR_LEN + uip_len + 2; /* Including footer[2]. */
  h.fc0 = FC0_TYPE_DATA | FC0_REQ_ACK | FC0_INTRA_PAN;
  h.fc1 = FC1_DST_16 | FC1_SRC_16;

  h.src = uip_hostaddr.u16[1];
  if (uip_ipaddr_cmp(&BUF->destipaddr, &uip_broadcast_addr))
    h.dst = 0xffff;
  else if (BUF->proto == UIP_PROTO_UDP
	   && BUF->destport == HTONS(UAODV_UDPPORT)) {
    h.dst = BUF->destipaddr.u16[1]; /* Routing messages bypass routing! */
    PRINTF("UNICAST to %d.%d.%d.%d\n", uip_ipaddr_to_quad(&BUF->destipaddr));
  } else {
    uip_ipaddr_t *next_gw;

    if (in_my_network(BUF->destipaddr.u16))
      next_gw = &BUF->destipaddr;
    else
      next_gw = &uip_draddr;	/* Default router. */

    if (cc2420_check_remote(next_gw->u16[1]) == REMOTE_NO)
      h.dst = next_gw->u16[1];	/* local, use ucast */
    else {			/* remote or unknown */
      struct uaodv_rt_entry *route = uaodv_rt_lookup_any(next_gw);

      if (route == NULL || route->is_bad) {
	if (tcpip_is_forwarding && cc2420_is_input)
	  uaodv_bad_dest(next_gw);
	else
	  uaodv_request_route_to(next_gw);
	return UIP_FW_DROPPED;
      } else if (cc2420_check_remote(route->nexthop.u16[1]) == REMOTE_YES) {
	PRINTF("LOST %d.%d\n",
	       route->nexthop.u16[1] & 0xff, route->nexthop.u16[1] >> 8);
	route->is_bad = 1;
	if (tcpip_is_forwarding && cc2420_is_input)
	  uaodv_bad_dest(next_gw);
	else
	  uaodv_request_route_to(next_gw);
	return UIP_FW_DROPPED;
      } else {
	h.dst = route->nexthop.u16[1];
	uaodv_rt_lru(route);
      }

    }
  }

  u8_t color = (cc2420_ack_received) ? LEDS_BLUE : LEDS_RED;
  leds_toggle(color);

  /* Don't request MAC level ACKs for broadcast packets. */
  if (h.dst == 0xffff)
    h.fc0 &= ~FC0_REQ_ACK;

  ret = cc2420_send(&h, 10, &uip_buf[UIP_LLH_LEN], uip_len);
  if (ret < 0) {
    PRINTF("cc2420_send_uaodv failed uip_len=%d ret=%d\n", uip_len, ret);
    leds_toggle(color);
    return UIP_FW_TOOLARGE;
  }

  leds_toggle(color);

  return UIP_FW_OK;
}
