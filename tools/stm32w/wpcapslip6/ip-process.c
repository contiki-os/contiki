
#define NETSTACK_CONF_WITH_IPV6 1
#define UIP_CONF_LL_802154 1

#include <string.h>

#define DEBUG DEBUG_NONE

#include "net/ip/uip-debug.h"

#include "ip-process.h"

#include <stdio.h>

#undef uip_buf
extern unsigned char *uip_buf;
extern uint16_t uip_len;


#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])


uint8_t mac_createEthernetAddr(uint8_t * ethernet, uip_lladdr_t * lowpan);
int8_t mac_translateIcmpLinkLayer();
int8_t mac_translateIPLinkLayer();


/**
 * \brief Translate IP packet's possible link-layer addresses, passing
 *        the message to the appropriate higher level function for this
 *        packet (aka: ICMP)
 * \param target The target we want to end up with - either ll_8023_type
 *        for ethernet, or ll_802154_type for 802.15.4
 * \return    Returns how successful the translation was
 * \retval 0  Addresses, if present, were translated.
 * \retval <0 Negative return values indicate various errors, as defined
 *            by the higher level function.
 */
int8_t mac_translateIPLinkLayer()
{

  if (UIP_IP_BUF->proto == UIP_PROTO_ICMP6) {
    PRINTF("eth2low: ICMP Message detected\n");
    return mac_translateIcmpLinkLayer();
  }
  return 0;
}

#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/uip-nd6.h"

typedef struct {
  uint8_t type;
  uint8_t length;
  uint8_t data[16];
} icmp_opts_t;

#define UIP_ICMP_BUF     ((struct uip_icmp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])
#define UIP_ICMP_OPTS(x) ((icmp_opts_t *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN + x])

void slide(uint8_t * data, uint8_t length, int16_t slide);

/**
 * \brief Translate the link-layer (L2) addresses in an ICMP packet.
 *        This will just be NA/NS/RA/RS packets currently.
 * \param target The target we want to end up with - either ll_8023_type
 *        for ethernet, or ll_802154_type for 802.15.4
 * \return       Returns how successful the translation was
 * \retval 0     Addresses, if present, were translated.
 * \retval -1    ICMP message was unknown type, nothing done.
 * \retval -2    ICMP Length does not make sense?
 * \retval -3    Unknown 'target' type
 */
int8_t mac_translateIcmpLinkLayer()
{
  uint16_t icmp_opt_offset = 0;
  int16_t len = UIP_IP_BUF->len[1] | (UIP_IP_BUF->len[0] << 8);

  uint16_t iplen;

  uint8_t i;

  int16_t sizechange;

  uint8_t llbuf[16];

  //Figure out offset to start of options
  switch(UIP_ICMP_BUF->type) {
    case ICMP6_NS:
    case ICMP6_NA:
      icmp_opt_offset = 24;
      break;

    case ICMP6_RS:
      icmp_opt_offset = 8;
      break;

    case ICMP6_RA:
      icmp_opt_offset = 16;
      break;

    case ICMP6_REDIRECT:
      icmp_opt_offset = 40;
      break;

      /** Things without link-layer */
    case ICMP6_DST_UNREACH:
    case ICMP6_PACKET_TOO_BIG:
    case ICMP6_TIME_EXCEEDED:
    case ICMP6_PARAM_PROB:
    case ICMP6_ECHO_REQUEST:
    case ICMP6_ECHO_REPLY:
      return 0;
      break;

    default:
      return -1;
  }

  //Figure out length of options
  len -= icmp_opt_offset;

  //Sanity check
  if (len < 8) return -2;

  //While we have options to do...
  while (len >= 8){

    //If we have one of these, we have something useful!
    if (((UIP_ICMP_OPTS(icmp_opt_offset)->type) == UIP_ND6_OPT_SLLAO) ||
        ((UIP_ICMP_OPTS(icmp_opt_offset)->type) == UIP_ND6_OPT_TLLAO) ) {

      /* Shrinking the buffer may thrash things, so we store the old
         link-layer address */
      for(i = 0; i < (UIP_ICMP_OPTS(icmp_opt_offset)->length*8 - 2); i++) {
        llbuf[i] = UIP_ICMP_OPTS(icmp_opt_offset)->data[i];
      }


      //Shrink/grow buffer as needed
      /* Current is 802.15.4, Hence current link-layer option is 14 extra
       * bytes.
       * (Actual LL is 8 bytes, but total option length is in multiples of
       * 8 Bytes, hence 8 + 2 = 10. Closest is 16 bytes, then 16 bytes for
       * total optional length - 2 bytes for type + length leaves 14 )
       */
      sizechange = -8;
      slide(UIP_ICMP_OPTS(icmp_opt_offset)->data + 14, len - 14, sizechange);


      mac_createEthernetAddr(UIP_ICMP_OPTS(icmp_opt_offset)->data, (uip_lladdr_t *)llbuf);

      //Adjust the length
      UIP_ICMP_OPTS(icmp_opt_offset)->length = 1;

      //Adjust the IP header length, as well as uIP length
      iplen = UIP_IP_BUF->len[1] | (UIP_IP_BUF->len[0]<<8);
      iplen += sizechange;
      len += sizechange;

      UIP_IP_BUF->len[1] = (uint8_t)iplen;
      UIP_IP_BUF->len[0] = (uint8_t)(iplen >> 8);

      uip_len += sizechange;

      //We broke ICMP checksum, be sure to fix that
      UIP_ICMP_BUF->icmpchksum = 0;
      UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

      //Finally set up next run in while loop
      len -= 8 * UIP_ICMP_OPTS(icmp_opt_offset)->length;
      icmp_opt_offset += 8 * UIP_ICMP_OPTS(icmp_opt_offset)->length;
    } else {

      //Not an option we care about, ignore it
      len -= 8 * UIP_ICMP_OPTS(icmp_opt_offset)->length;

      //This shouldn't happen!
      if (UIP_ICMP_OPTS(icmp_opt_offset)->length == 0) {
        PRINTF("Option in ND packet has length zero, error?\n");
        len = 0;
      }

      icmp_opt_offset += 8 * UIP_ICMP_OPTS(icmp_opt_offset)->length;

    } //If ICMP_OPT is one we care about

  } //while(len >= 8)

  return 0;

}

/**
 * \brief Create a 802.3 address from a 802.15.4 long address
 * \param ethernet   Pointer to ethernet address
 * \param lowpan     Pointer to 802.15.4 address
 */
uint8_t mac_createEthernetAddr(uint8_t * ethernet, uip_lladdr_t * lowpan)
{
 /*   uint8_t j, match; */

	uint8_t tmp[8];

	memcpy(tmp,lowpan,sizeof(uip_lladdr_t));

    memcpy(ethernet, tmp, 3);
    memcpy(ethernet+3, tmp+5, 3);
    ethernet[0] |= 0x02;

  return 1;
}

/**
 * \brief        Slide the pointed to memory up a certain amount,
 *               growing/shrinking a buffer
 * \param data   Pointer to start of data buffer
 * \param length Length of the data buffer
 * \param slide  How many bytes to slide the buffer up in memory (if +) or
 *               down in memory (if -)
 */
void slide(uint8_t * data, uint8_t length, int16_t slide)
{
  //Sanity checks
  if (!length) return;
  if (!slide) return;

  uint8_t i = 0;

  while(length) {
    length--;

    //If we are sliding up, we do from the top of the buffer down
    if (slide > 0) {
      *(data + length + slide) = *(data + length);

      //If we are sliding down, we do from the bottom of the buffer up
    } else {
      *(data + slide + i) = *(data + i);
    }

    i++;
  }
}





uint16_t ip_process(unsigned char *buf, unsigned int len)
{
	uip_buf = buf;
	uip_len = len;

	mac_translateIPLinkLayer();

	return uip_len;

}



