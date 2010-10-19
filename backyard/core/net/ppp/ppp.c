/*															
 *---------------------------------------------------------------------------
 * ppp.c - PPP Processor/Handler											-
 *																			-
 *---------------------------------------------------------------------------
 * 
 * Version                                                                  -
 *		0.1 Original Version Jun 3, 2000									-        
 *																			-
 *---------------------------------------------------------------------------    
 */
/*
 * Copyright (c) 2003, Mike Johnson, Mycal Labs, www.mycal.net
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Mike Johnson/Mycal Labs
 *		www.mycal.net.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Mycal Modified uIP TCP/IP stack.
 *
 * $Id: ppp.c,v 1.2 2010/10/19 18:29:03 adamdunkels Exp $
 *
 */

/*			*/ 
/* include files 	*/
/*			*/ 


#include "lcp.h"
#include "pap.h"
#include "ipcp.h"
/*#include "time.h"*/
/*#include "mip.h"*/

#if 0
#define DEBUG1(x)
#else
#include <stdio.h>
#define DEBUG1(x) debug_printf x
#endif

/*
  Set the debug message level
*/
#define	PACKET_RX_DEBUG	1

/*
  Include stuff
*/
/*#include "mTypes.h"*/
#include "ppp.h"
#include "ahdlc.h"
#include "ipcp.h"
#include "lcp.h"

extern int ppp_arch_getchar(u8_t *p);


/*
  Buffers that this layer needs (this can be optimized out)
*/
u8_t ppp_rx_buffer[PPP_RX_BUFFER_SIZE];
/*u8_t ppp_tx_buffer[PPP_TX_BUFFER_SIZE];*/

/*
 * Other state storage (this can be placed in a struct and this could could
 *	support multiple PPP connections, would have to embedded the other ppp
 *	module state also)
 */
u8_t ppp_flags;
u8_t ppp_id;
u8_t ppp_retry;

#if PACKET_RX_DEBUG
u16_t ppp_rx_frame_count=0;
u16_t ppp_rx_tobig_error;
u8_t done;    /* temporary variable */
#endif

/*---------------------------------------------------------------------------*/
static u8_t
check_ppp_errors(void)
{
  u8_t ret = 0;

  /* Check Errors */
  if(lcp_state & LCP_TX_TIMEOUT) {
    ret = 1;
  }
  if(lcp_state & LCP_RX_TIMEOUT) {
    ret = 2;
  }

  if(pap_state & PAP_TX_AUTH_FAIL) {
    ret = 3;
  }
  if(pap_state & PAP_RX_AUTH_FAIL) {
    ret = 4;
  }

  if(pap_state & PAP_TX_TIMEOUT) {
    ret = 5;
  }
  if(pap_state & PAP_RX_TIMEOUT) {
    ret = 6;
  }

  if(ipcp_state & IPCP_TX_TIMEOUT) {
    ret = 7;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
/*
 * Unknown Protocol Handler, sends reject
 */
static void
ppp_reject_protocol(u16_t protocol, u8_t *buffer, u16_t count)
{
  u16_t	i;
  u8_t *dptr, *sptr;
  LCPPKT *pkt;
	
  /* first copy rejected packet back, start from end and work forward,
     +++ Pay attention to buffer managment when updated. Assumes fixed
     PPP blocks. */
  DEBUG1(("Rejecting Protocol\n"));
  if((count + 6) > PPP_RX_BUFFER_SIZE) {
    /* This is a fatal error +++ do somthing about it. */
    DEBUG1(("Cannot Reject Protocol, PKT to big\n"));
    return;
  }
  dptr = buffer + count + 6;
  sptr = buffer + count;
  for(i = 0; i < count; ++i) {
    *dptr-- = *sptr--;
  }

  pkt = (LCPPKT *)buffer;
  pkt->code = PROT_REJ;		/* Write Conf_rej */
  /*pkt->id = tid++;*/			/* write tid */
  pkt->len = uip_htons(count + 6);
  *((u16_t *)(&pkt->data[0])) = uip_htons(protocol);

  ahdlc_tx(LCP, buffer, 0, (u16_t)(count + 6), 0);
}
/*---------------------------------------------------------------------------*/
#if PACKET_RX_DEBUG
void
dump_ppp_packet(u8_t *buffer, u16_t len)
{
  int i;

  DEBUG1(("\n"));
  for(i = 0;i < len; ++i) {
    if((i & 0x1f) == 0x10) {
      DEBUG1(("\n"));
    }
    DEBUG1(("0x%02x ",buffer[i]));
  }
  DEBUG1(("\n\n"));
}
#endif
/*---------------------------------------------------------------------------*/
/* Initialize and start PPP engine.  This just sets things up to
 * starting values.  This can stay a private method.
 */
/*---------------------------------------------------------------------------*/
void
ppp_init()
{
#if PACKET_RX_DEBUG
  ppp_rx_frame_count = 0;
  done = 0;
#endif
  ppp_flags = 0;
  pap_init();
  ipcp_init();
  lcp_init();
  ppp_flags = 0;
	
  ahdlc_init(ppp_rx_buffer, PPP_RX_BUFFER_SIZE);
  ahdlc_rx_ready();
}
/*---------------------------------------------------------------------------*/
/* raise_ppp() - This routine will try to bring up a PPP connection, 
 *  It is blocking. In the future we probably want to pass a
 *  structure with all the options on bringing up a PPP link, like
 *  server/client, DSN server, username password for PAP... +++ for
 *  now just use config and bit defines
 */
/*---------------------------------------------------------------------------*/
#if 0
u16_t
ppp_raise(u8_t config, u8_t *username, u8_t *password)
{
  u16_t	status = 0;

  /* Initialize PPP engine */
  /* init_ppp(); */
  pap_init();
  ipcp_init();
  lcp_init();

  /* Enable PPP */
  ppp_flags = PPP_RX_READY;

  /* Try to bring up the layers */
  while(status == 0) {
#ifdef SYSTEM_POLLER
    /* If the the serial interrupt is not hooked to ahdlc_rx, or the
       system needs to handle other stuff while were blocking, call
       the system poller.*/
      system_poller();
#endif		

      /* call the lcp task to bring up the LCP layer */
      lcp_task(ppp_tx_buffer);

      /* If LCP is up, neg next layer */
      if(lcp_state & LCP_TX_UP) {
	/* If LCP wants PAP, try to authenticate, else bring up IPCP */
	if((lcp_state & LCP_RX_AUTH) && (!(pap_state & PAP_TX_UP))) {
	  pap_task(ppp_tx_buffer,username,password);  
	} else {
	  ipcp_task(ppp_tx_buffer);
	}
      }


      /* If IPCP came up then our link should be up. */
      if((ipcp_state & IPCP_TX_UP) && (ipcp_state & IPCP_RX_UP)) {
	break;
      }
      
      status = check_ppp_errors();
    }
  
  return status;
}
#endif
/*---------------------------------------------------------------------------*/
void
ppp_connect(void)
{
  /* Initialize PPP engine */
  /* init_ppp(); */
  pap_init();
  ipcp_init();
  lcp_init();
  
  /* Enable PPP */
  ppp_flags = PPP_RX_READY;
}
/*---------------------------------------------------------------------------*/
void
ppp_send(void)
{
  /* If IPCP came up then our link should be up. */
  if((ipcp_state & IPCP_TX_UP) && (ipcp_state & IPCP_RX_UP)) {
    ahdlc_tx(IPV4, &uip_buf[ UIP_LLH_LEN ],        uip_appdata,
		   UIP_TCPIP_HLEN, uip_len - UIP_TCPIP_HLEN);
  }
}
/*---------------------------------------------------------------------------*/
void
ppp_poll(void)
{
  u8_t c;

  uip_len = 0;

  if(!(ppp_flags & PPP_RX_READY)) {
    return;
  }

  while(uip_len == 0 && ppp_arch_getchar(&c)) {
    ahdlc_rx(c);
  }

  /* If IPCP came up then our link should be up. */
  if((ipcp_state & IPCP_TX_UP) && (ipcp_state & IPCP_RX_UP)) {
    return;
  }

  /* call the lcp task to bring up the LCP layer */
  lcp_task( &uip_buf[ UIP_LLH_LEN ]);

  /* If LCP is up, neg next layer */
  if((lcp_state & LCP_TX_UP) && (lcp_state & LCP_RX_UP)) {
    /* If LCP wants PAP, try to authenticate, else bring up IPCP */
    if((lcp_state & LCP_RX_AUTH) && (!(pap_state & PAP_TX_UP))) {
      pap_task( &uip_buf[ UIP_LLH_LEN ]);  
    } else {
      ipcp_task( &uip_buf[ UIP_LLH_LEN ]);
    }
  }
}
/*---------------------------------------------------------------------------*/
/* ppp_upcall() - this is where valid PPP frames from the ahdlc layer are
 *	sent to be processed and demuxed.
 */
/*---------------------------------------------------------------------------*/
void
ppp_upcall(u16_t protocol, u8_t *buffer, u16_t len)
{
#if PACKET_RX_DEBUG
  ++ppp_rx_frame_count;
  dump_ppp_packet(buffer, len);
  if(ppp_rx_frame_count > 18) {
    done = 1;
  }
#endif

  /* check to see if we have a packet waiting to be processed */
  if(ppp_flags & PPP_RX_READY) {	
    /* demux on protocol field */
    switch(protocol) {
    case LCP:	/* We must support some level of LCP */
      DEBUG1(("LCP Packet - "));
      lcp_rx(buffer, len);
      DEBUG1(("\n"));
      break;
    case PAP:	/* PAP should be compile in optional */
      DEBUG1(("PAP Packet - "));
      pap_rx(buffer, len);
      DEBUG1(("\n"));
      break;
    case IPCP:	/* IPCP should be compile in optional. */
      DEBUG1(("IPCP Packet - "));
      ipcp_rx(buffer, len);
      DEBUG1(("\n"));
      break;
    case IPV4:	/* We must support IPV4 */
      DEBUG1(("IPV4 Packet---\n"));
      memcpy( &uip_buf[ UIP_LLH_LEN ], buffer, len);
      uip_len = len;
      DEBUG1(("\n"));
      break;
    default:
      DEBUG1(("Unknown PPP Packet Type 0x%04x - ",protocol));
      ppp_reject_protocol(protocol, buffer, len);
      DEBUG1(("\n"));
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
/* scan_packet(list,buffer,len)
 *
 * list = list of supported ID's
 * *buffer pointer to the first code in the packet
 * length of the codespace
 */
u16_t
scan_packet(u16_t protocol, u8_t *list, u8_t *buffer, u8_t *options, u16_t len)
{
  u8_t *tlist, *bptr;
  u8_t *tptr;
  u8_t bad = 0;
  u8_t i, j, good;

  bptr = tptr = options;
  /* scan through the packet and see if it has any unsupported codes */
  while(bptr < options + len) {
    /* get code and see if it matches somwhere in the list, if not
       we don't support it */
    i = *bptr++;
    
    /*    DEBUG2("%x - ",i);*/
    tlist = list;
      good = 0;
      while(*tlist) {
	/*	DEBUG2("%x ",*tlist);*/
	if(i == *tlist++) {
	  good = 1;
	  break;
	}
      }
      if(!good) {
	/* we don't understand it, write it back */
	DEBUG1(("We don't understand option 0x%02x\n",i));
	bad = 1;
	*tptr++ = i;
	j = *tptr++ = *bptr++;
	for(i = 0; i < j - 2; ++i) {
	  *tptr++ = *bptr++;
	}
      } else {
	/* advance over to next option */
	bptr += *bptr - 1;
      }
  }
  
  /* Bad? if we we need to send a config Reject */
  if(bad) {
    /* write the config Rej packet we've built above, take on the header */
    bptr = buffer;
    *bptr++ = CONF_REJ;		/* Write Conf_rej */
    bptr++;			/* skip over ID */
    *bptr++ = 0;
    *bptr = tptr - buffer;
    /* length right here? */
		
    /* write the reject frame */
    DEBUG1(("Writing Reject frame --\n"));
    ahdlc_tx(protocol, buffer, 0, (u16_t)(tptr - buffer), 0);
    DEBUG1(("\nEnd writing reject \n"));
    
  }		
  return bad;
}
/*---------------------------------------------------------------------------*/
