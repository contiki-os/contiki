
/* Various stub functions and uIP variables other code might need to
 * compile. Allows you to save needing to compile all of uIP in just
 * to get a few things */

#define UIP_CONF_IPV6 1

#include "net/uip.h"
#include <stdio.h>
#include <arpa/inet.h>

#undef uip_buf
unsigned char *uip_buf;
u16_t uip_len;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static u16_t
chksum(u16_t sum, const u8_t *data, u16_t len)
{
  u16_t t;
  const u8_t *dataptr;
  const u8_t *last_byte;

  dataptr = data;
  last_byte = data + len - 1;

  while(dataptr < last_byte) {   /* At least two more bytes */
    t = (dataptr[0] << 8) + dataptr[1];
    sum += t;
    if(sum < t) {
      sum++;      /* carry */
    }
    dataptr += 2;
  }

  if(dataptr == last_byte) {
    t = (dataptr[0] << 8) + 0;
    sum += t;
    if(sum < t) {
      sum++;      /* carry */
    }
  }

  /* Return sum in host byte order. */
  return sum;
}

static u16_t
upper_layer_chksum(u8_t proto)
{
  u16_t upper_layer_len;
  u16_t sum;

  upper_layer_len = (((u16_t)(UIP_IP_BUF->len[0]) << 8) + UIP_IP_BUF->len[1]) ;

  /* First sum pseudoheader. */
  /* IP protocol and length fields. This addition cannot carry. */
  sum = upper_layer_len + proto;
  /* Sum IP source and destination addresses. */
  sum = chksum(sum, (u8_t *)&UIP_IP_BUF->srcipaddr, 2 * sizeof(uip_ipaddr_t));

  /* Sum TCP header and data. */
  sum = chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN],
               upper_layer_len);

  return (sum == 0) ? 0xffff : htons(sum);
}

/*---------------------------------------------------------------------------*/
u16_t
uip_icmp6chksum(void)
{
  return upper_layer_chksum(UIP_PROTO_ICMP6);
}
