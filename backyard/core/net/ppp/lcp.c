/*  www.mycal.com
 *---------------------------------------------------------------------------
 *lcp.c - Link Configuration Protocol Handler.  - -
 *---------------------------------------------------------------------------
 *Version - 0.1 Original Version June 3, 2000 -
 *
 *---------------------------------------------------------------------------
 *- Copyright (C) 2000, Mycal Labs www.mycal.com - -
 *---------------------------------------------------------------------------
 *
 *
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
 * $Id: lcp.c,v 1.2 2010/10/19 18:29:03 adamdunkels Exp $
 *
 */

/*			*/ 
/* include files 	*/
/*			*/ 

#include "contiki-conf.h"

/*#include "time.h"*/
#include "ppp.h"
#include "ahdlc.h"
#include "lcp.h"

#if 0
#define DEBUG1(x)
#define DEBUG2(x)
#else
#include <stdio.h>
#define DEBUG1(x) debug_printf x
#define DEBUG2(x) debug_printf x
#endif

#define TIMER_expire()
#define TIMER_set()
#define TIMER_timeout(x) 1
static unsigned long	prev_lcp_seconds = 0;


/*u8_t				tflag;
u8_t				*lcp_buffer;
u16_t				lcp_tx_time;
u8_t				lcp_retry;
u16_t				lcp_timeout=5;*/

u8_t lcp_state;
u16_t ppp_tx_mru = 0;

/* We need this when we neg our direction.
   u8_t				lcp_tx_options; */

/*
 * Define the supported paramets for this module here.
 */
u8_t lcplist[] = {
  LPC_MAGICNUMBER,
  LPC_PFC,
  LPC_ACFC,
  LPC_AUTH,
  LPC_ACCM,
  LPC_MRU,
  0};	


/*---------------------------------------------------------------------------*/
/* lcp_init() - Initialize the LCP engine to startup values */
/*---------------------------------------------------------------------------*/
void
lcp_init(void)
{
  lcp_state = 0;
  ppp_retry = 0;
  TIMER_expire();
}
/*---------------------------------------------------------------------------*/
/* lcp_rx() - Receive an LCP packet and process it.  
 *	This routine receives a LCP packet in buffer of length count.
 *	Process it here, support for CONF_REQ, CONF_ACK, CONF_NACK, CONF_REJ or
 *	TERM_REQ.
 */
/*---------------------------------------------------------------------------*/
void
lcp_rx(u8_t *buffer, u16_t count)
{
  u8_t *bptr = buffer, *tptr;
  u8_t error = 0;
  u8_t id;
  u16_t len, j;

  switch(*bptr++) {
  case CONF_REQ:			/* config request */
    /* parce request and see if we can ACK it */
    id = *bptr++;
    len = (*bptr++ << 8);
    len |= *bptr++;
    /*len -= 2;*/
    DEBUG1(("received [LCP Config Request id %u\n",id));
    if(scan_packet((u16_t)LCP, lcplist, buffer, bptr, (u16_t)(len-4))) {
      /* must do the -4 here, !scan packet */
      
      DEBUG1((" options were rejected\n"));
    } else {
      /* lets try to implement what peer wants */
      tptr = bptr = buffer;
      bptr += 4;			/* skip code, id, len */
      error = 0;
      /* first scan for unknown values */
      while(bptr < buffer+len) {
	switch(*bptr++) {
	case LPC_MRU:	/* mru */
	  j = *bptr++;
	  j -= 2;
	  if(j == 2) {
	    ppp_tx_mru = ((int)*bptr++) << 8;
	    ppp_tx_mru |= *bptr++;
	    DEBUG1(("<mru %d> ",ppp_tx_mru));
	  } else {
	    DEBUG1(("<mru ??> "));
	  }
	  break;
	case LPC_ACCM:	/*		*/
	  bptr++;		/* skip length */	
	  j = *bptr++;
	  j += *bptr++;
	  j += *bptr++;
	  j += *bptr++;
	  if(j==0) {
	    // ok
	    DEBUG1(("<asyncmap sum=0x%04x>",j));
	    //ahdlc_flags |= PPP_TX_ASYNC_MAP;
	  } else if(j!=0) {
	    // ok
	    DEBUG1(("<asyncmap sum=0x%04x>, assume 0xffffffff",j));
	  } else {
	    /*
	     * fail we only support default or all zeros
	     */
	    DEBUG1(("We only support default or all zeros for ACCM "));
	    error = 1;
	    *tptr++ = LPC_ACCM;
	    *tptr++ = 0x6;
	    *tptr++ = 0;
	    *tptr++ = 0;
	    *tptr++ = 0;
	    *tptr++ = 0;
	  }
	  break;
	case LPC_AUTH:
	  bptr++;
	  if((*bptr++ == 0xc0) && (*bptr++ == 0x23)) {
	    /* negotiate PAP */
	    if (strlen(pap_username) > 0) {
	      DEBUG1(("<auth pap> "));
	      lcp_state |= LCP_RX_AUTH;	
	    } else {
	      DEBUG1(("<rej auth pap> "));
	      
	      *tptr++ = CONF_REJ;
	      *tptr++;	// Keep ID
	      *tptr++ = 0;
	      *tptr++ = 8;
	      *tptr++ = LPC_AUTH;
	      *tptr++ = 0x4;
	      *tptr++ = 0xc0;
	      *tptr++ = 0x23;
	      ahdlc_tx(LCP, 0, buffer, 0, (u16_t)(tptr-buffer));
	      return;
	    }
	  } else {
	    /* we only support PAP */
	    DEBUG1(("<auth ??>"));
	    error = 1;
	    *tptr++ = LPC_AUTH;
	    *tptr++ = 0x4;
	    *tptr++ = 0xc0;
	    *tptr++ = 0x23;
	  }
	  break;
	case LPC_MAGICNUMBER:
	  DEBUG1(("<magic > "));
	  /*
	   * Compair incoming number to our number (not implemented)
	   */
	  bptr++;		/* for now just dump */
	  bptr++;
	  bptr++;
	  bptr++;
	  bptr++;
	  break;
	case LPC_PFC:
	  bptr++;
	  DEBUG1(("<pcomp> "));
	  /*tflag|=PPP_PFC;*/
	  break;
	case LPC_ACFC:
	  bptr++;
	  DEBUG1(("<accomp> "));
	  /*tflag|=PPP_ACFC;*/
	  break;
	  
	}
      }
      /* Error? if we we need to send a config Reject ++++ this is good for a subroutine */
      if(error) {
	/* write the config NAK packet we've built above, take on the header */
	bptr = buffer;
	*bptr++ = CONF_NAK;		/* Write Conf_rej */
	*bptr++;/*tptr++;*/		/* skip over ID */

	/* Write new length */
	*bptr++ = 0;
	*bptr = tptr - buffer;

	/* write the reject frame */
	DEBUG1(("\nWriting NAK frame \n"));
	// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);				
	ahdlc_tx(LCP, 0, buffer, 0, (u16_t)(tptr-buffer));
	DEBUG1(("- end NAK Write frame\n"));
	
      } else {
	/*
	 * If we get here then we are OK, lets send an ACK and tell the rest
	 * of our modules our negotiated config.
	 */
	DEBUG1(("\nSend ACK!\n"));
	bptr = buffer;
	*bptr++ = CONF_ACK;		/* Write Conf_ACK */
	bptr++;				/* Skip ID (send same one) */
	/*
	 * Set stuff
	 */
	/*ppp_flags|=tflag;*/
	/* DEBUG2("SET- stuff -- are we up? c=%d dif=%d \n", count, (u16_t)(bptr-buffer)); */
	
	/* write the ACK frame */
	DEBUG2(("Writing ACK frame \n"));
	/* Send packet ahdlc_txz(procol,header,data,headerlen,datalen);	*/
	ahdlc_tx(LCP, 0, buffer, 0, count /*bptr-buffer*/);
       DEBUG2(("- end ACK Write frame\n"));
	lcp_state |= LCP_RX_UP;		
	
	/* expire the timer to make things happen after a state change */
	/*timer_expire();*/
	
      }
    }
    break;
  case CONF_ACK:			/* config Ack   Anytime we do an ack reset the timer to force send. */
    DEBUG1(("LCP-ACK - "));
    /* check that ID matches one sent */
    if(*bptr++ == ppp_id) {	
      /* Change state to PPP up. */
      DEBUG1((">>>>>>>> good ACK id up! %d\n",ppp_id));
      /* copy negotiated values over */
      
      lcp_state |= LCP_TX_UP;		
      
      /* expire the timer to make things happen after a state change */
      TIMER_expire();
    }
    else
      DEBUG1(("*************++++++++++ bad id %d\n",ppp_id));
    break;
  case CONF_NAK:			/* Config Nack */
    DEBUG1(("LCP-CONF NAK\n"));
    ppp_id++;
    break;
  case CONF_REJ:			/* Config Reject */
    DEBUG1(("LCP-CONF REJ\n"));
    ppp_id++;
    break;
  case TERM_REQ:			/* Terminate Request */
    DEBUG1(("LCP-TERM-REQ -"));
    bptr = buffer;
    *bptr++ = TERM_ACK;			/* Write TERM_ACK */
    /* write the reject frame */
    DEBUG1(("Writing TERM_ACK frame \n"));
    /* Send packet ahdlc_txz(procol,header,data,headerlen,datalen); */
    ahdlc_tx(LCP, 0, buffer, 0, count);
    lcp_state &= ~LCP_TX_UP;	
    lcp_state |= LCP_TERM_PEER;
    break;
  case TERM_ACK:
    DEBUG1(("LCP-TERM ACK\n"));
    break;
  default:
    break;
  }
}


void lcp_disconnect(u8_t id) {
  u8_t	buffer[4];
  u8_t	*bptr = buffer;
  *bptr++ = TERM_REQ;
  *bptr++ = id;
  *bptr++ = 0;
  *bptr++ = 4;
  ahdlc_tx(LCP, 0, buffer, 0, bptr - buffer);
}

/*---------------------------------------------------------------------------*/
/* lcp_task(buffer) - This routine see if a lcp request needs to be sent
 *	out.  It uses the passed buffer to form the packet.  This formed LCP
 *	request is what we negotiate for sending options on the link.
 *
 *	Currently we negotiate : Magic Number Only, but this will change.
 */
/*---------------------------------------------------------------------------*/
void
lcp_task(u8_t *buffer)
{
  u8_t *bptr;
  u16_t t;
  LCPPKT *pkt;

  /* lcp tx not up and hasn't timed out then lets see if we need to send a request */
  if(!(lcp_state & LCP_TX_UP) && !(lcp_state & LCP_TX_TIMEOUT)) {
    /* Check if we have a request pending */
    /*t=get_seconds()-lcp_tx_time;*/
#if 0
    if(1 == TIMER_timeout(LCP_TX_TIMEOUT)) {
#else
    if((clock_seconds() - prev_lcp_seconds) > LCP_TIMEOUT) {
      prev_lcp_seconds = clock_seconds();
#endif
      DEBUG1(("\nSending LCP request packet - "));
      /*
       * No pending request, lets build one
       */
      pkt = (LCPPKT *)buffer;		

      /* Configure-Request only here, write id */
      pkt->code = CONF_REQ;
      pkt->id = ppp_id;
      
      bptr = pkt->data;
      
      /* Write options */

      /* ACCM */
      //if((lcp_tx_options & LCP_OPT_ACCM) & 0) {
	*bptr++ = LPC_ACCM;
	*bptr++ = 0x6;
	*bptr++ = 0xff;
	*bptr++ = 0xff;
	*bptr++ = 0xff;
	*bptr++ = 0xff;
      //}
      
#if 0
      /* Write magic number */
      DEBUG1(("LPC_MAGICNUMBER -"));
      *bptr++ = LPC_MAGICNUMBER;
      *bptr++ = 0x6;
      /*
      *bptr++ = random_rand() & 0xff;
      *bptr++ = random_rand() & 0xff;
      *bptr++ = random_rand() & 0xff;
      *bptr++ = random_rand() & 0xff;
      */
      *bptr++ = 0x11;
      *bptr++ = 0x11;
      *bptr++ = 0x11;
      *bptr++ = 0x11;
#endif
      
#if 0
      /*
       * Authentication protocol
       */
      if((lcp_tx_options & LCP_OPT_AUTH) && 0) {
	/*
	 * If turned on, we only negotiate PAP
	 */
	*bptr++ = LPC_AUTH;
	*bptr++ = 0x4;
	*bptr++ = 0xc0;
	*bptr++ = 0x23;
      }
      /*
       * PFC
       */
      if((lcp_tx_options & LCP_OPT_PFC) && 0) {
	/*
	 * If turned on, we only negotiate PAP
	 */
	*bptr++ = LPC_PFC;
	*bptr++ = 0x2;
      }
      /*
       * ACFC
       */
      if((lcp_tx_options & LCP_OPT_ACFC) && 0) {
	/*
	 * If turned on, we only negotiate PAP
	 */
	*bptr++ = LPC_ACFC;
	*bptr++ = 0x2;
      }
#endif
      /* Write length */
      t = bptr - buffer;
      pkt->len = uip_htons(t);			/* length here -  code and ID + */
      
      DEBUG1((" len %d\n",t));
      
      /* Send packet */
      /* Send packet ahdlc_txz(procol,header,data,headerlen,datalen); */
      ahdlc_tx(LCP, 0, buffer, 0, t);
      
      /* Set timer */
      TIMER_set();
      /*lcp_tx_time=get_seconds();*/
      /* Inc retry */
      ppp_retry++;
      /*
       * Have we timed out?
       */
      if(ppp_retry > LCP_RETRY_COUNT) {
	lcp_state &= LCP_TX_TIMEOUT;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
