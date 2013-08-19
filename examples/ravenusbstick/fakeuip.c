
/* Various stub functions and uIP variables other code might need to
 * compile. Allows you to save needing to compile all of uIP in just
 * to get a few things.  This file is included in the build by the contiki
 * Makefile.include only when CONTIKI_NO_NET is defined */
 
#include "uip.h"
#include <string.h>

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

uip_buf_t uip_aligned_buf;

uint16_t uip_len;

struct uip_stats uip_stat;

uip_lladdr_t uip_lladdr;

static uint8_t (* output)(uip_lladdr_t *);
extern void mac_LowpanToEthernet(void);
void tcpip_input( void )
{
//  printf("tcpip_input");
  mac_LowpanToEthernet();
}

uint8_t tcpip_output(uip_lladdr_t * lladdr){
  if(output != NULL) {
    return output(lladdr);
  }
 return 0;
}
//Called from  sicslowpan.c
void tcpip_set_outputfunc(uint8_t (* f)(uip_lladdr_t *)) {
  output = f;
}

uint16_t uip_htons(uint16_t val) { return UIP_HTONS(val);}


#if THEOLDWAY
/********** UIP_NETIF.c **********/
void
uip_netif_addr_autoconf_set(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
#else
/********** uip-ds6.c ************/
void
uip_ds6_set_addr_iid(uip_ipaddr_t * ipaddr, uip_lladdr_t * lladdr)
#endif /* THEOLDWAY */
{
  /* We consider only links with IEEE EUI-64 identifier or
     IEEE 48-bit MAC addresses */
#if (UIP_LLADDR_LEN == 8)
  memcpy(ipaddr->u8 + 8, lladdr, UIP_LLADDR_LEN);
  ipaddr->u8[8] ^= 0x02;
#elif (UIP_LLADDR_LEN == 6)
  memcpy(ipaddr->u8 + 8, lladdr, 3);
  ipaddr->u8[11] = 0xff;
  ipaddr->u8[12] = 0xfe;
  memcpy(ipaddr->u8 + 13, lladdr + 3, 3);
  ipaddr->u8[8] ^= 0x02;
#else
#error cannot build interface address when UIP_LLADDR_LEN is not 6 or 8
  /*
    UIP_LOG("CAN NOT BUIL INTERFACE IDENTIFIER");
    UIP_LOG("THE STACK IS GOING TO SHUT DOWN");
    UIP_LOG("THE HOST WILL BE UNREACHABLE");
    exit(-1);
  */
#endif
}

/********** UIP.c ****************/

static uint16_t
chksum(uint16_t sum, const uint8_t *data, uint16_t len)
{
  uint16_t t;
  const uint8_t *dataptr;
  const uint8_t *last_byte;

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

static uint16_t
upper_layer_chksum(uint8_t proto)
{
  uint16_t upper_layer_len;
  uint16_t sum;
  
  upper_layer_len = (((uint16_t)(UIP_IP_BUF->len[0]) << 8) + UIP_IP_BUF->len[1]) ;
  
  /* First sum pseudoheader. */
  /* IP protocol and length fields. This addition cannot carry. */
  sum = upper_layer_len + proto;
  /* Sum IP source and destination addresses. */
  sum = chksum(sum, (uint8_t *)&UIP_IP_BUF->srcipaddr, 2 * sizeof(uip_ipaddr_t));

  /* Sum TCP header and data. */
  sum = chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN],
               upper_layer_len);
    
  return (sum == 0) ? 0xffff : uip_htons(sum);
}

/*---------------------------------------------------------------------------*/
uint16_t
uip_icmp6chksum(void)
{
  return upper_layer_chksum(UIP_PROTO_ICMP6); 
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_link_neighbor_callback(int status, int numtx)
{
}
