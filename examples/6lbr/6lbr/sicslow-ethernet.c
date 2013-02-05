/**
 * \file sicslow_ethernet.c
 *         Routines to interface between Ethernet and 6LowPan
 *
 * \author
 *         Colin O'Flynn <coflynn@newae.com>
 *
 * \addtogroup usbstick 
 */

/* Copyright (c) 2008 by:
 * Colin O'Flynn coflynn@newae.com
 * Eric Gnoske egnoske@gmail.com
 * Blake Leverett bleverett@gmail.com
 * Mike Vidales mavida404@gmail.com
 * Kevin Brown kbrown3@uccs.edu
 * Nate Bohlmann nate@elfwerks.com
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * * Neither the name of the copyright holders nor the names of
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
   \ingroup usbstick
   \defgroup sicslowinterop 6LowPan Ethernet Interop
   @{
*/

#ifndef UIP_CONF_AUTO_SUBSTITUTE_LOCAL_MAC_ADDR
#define UIP_CONF_AUTO_SUBSTITUTE_LOCAL_MAC_ADDR	1
#endif // ifndef UIP_CONF_AUTO_SUBSTITUTE_LOCAL_MAC_ADDR


#include "uip.h"
#include "uip_arp.h" //For ethernet header structure

#include "net/rime.h"
#include "sicslowpan.h"
#include "sicslow-ethernet.h"

#include "enc28j60-drv.h"
#include "net/uip-ds6.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 0
#include "uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define ETHBUF(x) ((struct uip_eth_hdr *)x)

//For little endian, such as our friend mr. AVR
#ifndef LSB
#define LSB(u16)     (((uint8_t  *)&(u16))[0])  //!< Least significant byte of \a u16.
#define MSB(u16)     (((uint8_t  *)&(u16))[1])  //!< Most significant byte of \a u16.
#endif

extern uint8_t eth_mac_addr[6];

uint8_t mac_createSicslowpanLongAddr(uint8_t * ethernet, uip_lladdr_t * lowpan);
uint8_t mac_createEthernetAddr(uint8_t * ethernet, uip_lladdr_t * lowpan);
uint8_t mac_createDefaultEthernetAddr(uint8_t * ethernet);

//! Location of TRANSLATE (TR) bit in Ethernet address
#define TRANSLATE_BIT_MASK (1<<2) 
//! Location of LOCAL (GL) bit in Ethernet address
#define LOCAL_BIT_MASK     (1<<1)
//! Location of MULTICAST (MU) bit in Ethernet address
#define MULTICAST_BIT_MASK (1<<0)

#define PREFIX_BUFFER_SIZE 32

uint8_t prefixCounter;
uint8_t prefixBuffer[PREFIX_BUFFER_SIZE][3];

/* 6lowpan max size + ethernet header size + 1 */
uint8_t raw_buf[127+ UIP_LLH_LEN +1];

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
int8_t mac_translateIPLinkLayer(lltype_t target)
{

  if (UIP_IP_BUF->proto == UIP_PROTO_ICMP6) {
    PRINTF("translateIP: ICMP Message detected\n\r");
    return mac_translateIcmpLinkLayer(target);
  }
  return 0;
}

#include "net/uip-icmp6.h"
#include "net/uip-nd6.h"

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
int8_t mac_translateIcmpLinkLayer(lltype_t target)
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
    case 131: //Multicast Listener Report
    case 132: //Multicast Listener Done
      return 0;
      break;

    default:
      return -1;
  }

  //Figure out length of options
  len -= icmp_opt_offset;

  //NS with Unspecified source address has no options !
  //Sanity check
  //if (len < 8) return -2;

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
      if (target == ll_802154_type) {
        //Current is 802.3, Hence current link-layer option is 6 extra bytes
        sizechange = 8;
        slide(UIP_ICMP_OPTS(icmp_opt_offset)->data + 6, len - 6, sizechange);
      } else if (target == ll_8023_type) {
        /* Current is 802.15.4, Hence current link-layer option is 14 extra
         * bytes.
         * (Actual LL is 8 bytes, but total option length is in multiples of
         * 8 Bytes, hence 8 + 2 = 10. Closest is 16 bytes, then 16 bytes for
         * total optional length - 2 bytes for type + length leaves 14 )
         */
        sizechange = -8;
        slide(UIP_ICMP_OPTS(icmp_opt_offset)->data + 14, len - 14, sizechange);
      } else {
        return -3; //Uh-oh!
      }
      
      //Translate addresses
      if (target == ll_802154_type) {
        mac_createSicslowpanLongAddr(llbuf, (uip_lladdr_t *)UIP_ICMP_OPTS(icmp_opt_offset)->data);
      } else {
        if(!mac_createEthernetAddr(UIP_ICMP_OPTS(icmp_opt_offset)->data, (uip_lladdr_t *)llbuf))
            mac_createDefaultEthernetAddr(UIP_ICMP_OPTS(icmp_opt_offset)->data);
      }
      
      //Adjust the length
      if (target == ll_802154_type) {
        UIP_ICMP_OPTS(icmp_opt_offset)->length = 2;
      } else {
        UIP_ICMP_OPTS(icmp_opt_offset)->length = 1;
      }

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
        PRINTF("Option in ND packet has length zero, error?\n\r");
        len = 0;
      }

      icmp_opt_offset += 8 * UIP_ICMP_OPTS(icmp_opt_offset)->length;
      
    } //If ICMP_OPT is one we care about
   
  } //while(len >= 8)

  return 0;

}


/**
 * \brief Create a 802.15.4 long address from a 802.3 address
 * \param ethernet   Pointer to ethernet address
 * \param lowpan     Pointer to 802.15.4 address
 */
uint8_t mac_createSicslowpanLongAddr(uint8_t * ethernet, uip_lladdr_t * lowpan)
{
#if UIP_CONF_AUTO_SUBSTITUTE_LOCAL_MAC_ADDR
  //Special case - if the address is our address, we just copy over what we know to be
  //our 802.15.4 address
  if (memcmp((uint8_t *)&eth_mac_addr, ethernet, 6) == 0)
  {
    memcpy((uint8_t *)lowpan, (uint8_t *)&uip_lladdr, UIP_LLADDR_LEN);
    return 1;
  }
#endif

  uint8_t index;

  //Check if translate bit is set, hence we have to look up the prefix
  if((ethernet[0]&(TRANSLATE_BIT_MASK|MULTICAST_BIT_MASK|LOCAL_BIT_MASK)) == (TRANSLATE_BIT_MASK|LOCAL_BIT_MASK)) {
    //Get top bits
    index = ethernet[0] >> 3;

    //Copy over prefix
    lowpan->addr[0] = prefixBuffer[index][0];
    lowpan->addr[1] = prefixBuffer[index][1];
    lowpan->addr[2] = prefixBuffer[index][2];
    lowpan->addr[3] = ethernet[1];
    lowpan->addr[4] = ethernet[2];

    //Check this is plausible...
    if (index >= prefixCounter)
        return 0;
  } else {
    lowpan->addr[0] = ethernet[0];
    lowpan->addr[1] = ethernet[1];
    lowpan->addr[2] = ethernet[2];
    lowpan->addr[3] = 0xff;
    lowpan->addr[4] = 0xfe;
  }

  lowpan->addr[5] = ethernet[3];
  lowpan->addr[6] = ethernet[4];
  lowpan->addr[7] = ethernet[5];
  return 1;
}


/**
 * \brief Create a 802.3 address from a 802.15.4 long address
 * \param ethernet   Pointer to ethernet address
 * \param lowpan     Pointer to 802.15.4 address
 */
uint8_t mac_createEthernetAddr(uint8_t * ethernet, uip_lladdr_t * lowpan)
{
#if UIP_CONF_AUTO_SUBSTITUTE_LOCAL_MAC_ADDR
	//Special case - if the address is our address, we just copy over what we know to be
	//our 802.3 address
	if (memcmp((uint8_t *)&uip_lladdr, (uint8_t *)lowpan, UIP_LLADDR_LEN) == 0) {
		memcpy(ethernet, eth_mac_addr, 6);
		return 1;
	} 
#endif  

  uint8_t index = 0;
  uint8_t i;

	//Check if we need to do anything:
	if ((lowpan->addr[3] == 0xff) && (lowpan->addr[4] == 0xfe)) {
	  //((lowpan->addr[0] & TRANSLATE_BIT_MASK) == 0) &&
	  //((lowpan->addr[0] & MULTICAST_BIT_MASK) == 0) &&
	  //(lowpan->addr[0] & LOCAL_BIT_MASK)) {
	    /** Nope: just copy over 6 bytes **/

		PRINTF("Low2Eth direct : ");
		PRINTLLADDR(lowpan);
	  PRINTF("\n");
	  if((lowpan->addr[0]&(TRANSLATE_BIT_MASK|MULTICAST_BIT_MASK|LOCAL_BIT_MASK)) == (TRANSLATE_BIT_MASK|LOCAL_BIT_MASK)) {
		  PRINTF("Low2Eth direct : ADDRESS PREFIX CONFLICT\n");
	  }

      ethernet[0] = lowpan->addr[0];
	  ethernet[1] = lowpan->addr[1];
	  ethernet[2] = lowpan->addr[2];
	  ethernet[3] = lowpan->addr[5];
	  ethernet[4] = lowpan->addr[6];
	  ethernet[5] = lowpan->addr[7];
	  
	
  } else {
	  PRINTF("Low2Eth translate : ");
	  PRINTLLADDR(lowpan);
	  PRINTF("\n");

    /** Yes: need to store prefix **/
    for (i = 0; i < prefixCounter; i++)	{	
      //Check the current prefix - if it fails, check next one
      if ((lowpan->addr[0] == prefixBuffer[i][0]) &&
         (lowpan->addr[1] == prefixBuffer[i][1]) &&
         (lowpan->addr[2] == prefixBuffer[i][2])) {
        break;
      }
  }
    index = i;

		if (index >= PREFIX_BUFFER_SIZE) {
		    PRINTF("Low2Eth buffer overflow\n");
			// Overflow. Fall back to simple translation.
			// TODO: Implement me!
			ethernet[0] = lowpan->addr[0];
			ethernet[1] = lowpan->addr[1];
			ethernet[2] = lowpan->addr[2];
			ethernet[3] = lowpan->addr[5];
			ethernet[4] = lowpan->addr[6];
			ethernet[5] = lowpan->addr[7];
			return 0;
		} else {	
			//Are we making a new one?
			if (index == prefixCounter) {
			    printf("Low2Eth adding prefix\n");
				prefixCounter++;
				prefixBuffer[index][0] = lowpan->addr[0];
				prefixBuffer[index][1] = lowpan->addr[1];
				prefixBuffer[index][2] = lowpan->addr[2];
			}

			//Create ethernet MAC address now
			ethernet[0] = TRANSLATE_BIT_MASK | LOCAL_BIT_MASK | (index << 3);
			ethernet[1] = lowpan->addr[3];
			ethernet[2] = lowpan->addr[4];
			ethernet[3] = lowpan->addr[5];
			ethernet[4] = lowpan->addr[6];
			ethernet[5] = lowpan->addr[7];
      
      printf("Low2Eth Lowpan addr : %d:%d:%d:%d:%d:%d:%d:%d\n", lowpan->addr[0], 
              lowpan->addr[1], lowpan->addr[2], lowpan->addr[3], lowpan->addr[4], 
              lowpan->addr[5], lowpan->addr[6], lowpan->addr[7]);
      printf("Low2Eth Ethernet addr : %d:%d:%d:%d:%d:%d\n", ethernet[0], 
              ethernet[1], ethernet[2], ethernet[3], ethernet[4], ethernet[5]);
		}
	}

  return 1;
}
/**
 * \brief Create a 802.3 address (default)
 * \param ethernet   Pointer to ethernet address
 */
uint8_t mac_createDefaultEthernetAddr(uint8_t * ethernet)
{
  memcpy(ethernet, &eth_mac_addr, 6);
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

/** @} */



/** @} */
