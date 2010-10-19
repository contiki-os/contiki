/*www.mycal.net
 *---------------------------------------------------------------------------
 *pap.c - PAP processor for the PPP module - -
 *---------------------------------------------------------------------------
 *Version - 0.1 Original Version Jun 3, 2000 - -
 *---------------------------------------------------------------------------
 *- Copyright (C) 2000, Mycal Labs www.mycal.com - -
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
 * $Id: pap.c,v 1.2 2010/10/19 18:29:03 adamdunkels Exp $
 *
 */

/*			*/ 
/* include files 	*/
/*			*/ 

#include	<string.h>
#include	"ppp.h"
#include	"pap.h"
#include	"lcp.h"

#if 0
#define DEBUG1(x)
#else
#include <stdio.h>
#define DEBUG1(x) debug_printf x
#endif

/*#include	"time.h"*/
/*#include	"utils.h" */
#define TIMER_expire()
#define TIMER_set()
#define TIMER_timeout(x) 1


u8_t pap_state;

#ifdef PAP_USERNAME
u8_t pap_username[] = PAP_USERNAME;
#else
u8_t pap_username[PAP_USERNAME_SIZE];
#endif

#ifdef PAP_PASSWORD
u8_t pap_password[] = PAP_PASSWORD;
#else
u8_t pap_password[PAP_PASSWORD_SIZE];
#endif

/*u16_t			pap_tx_time;
  u8_t			pap_timeout;*/

/*---------------------------------------------------------------------------*/
void
pap_init(void)
{
  ppp_retry = 0;			/* We reuse ppp_retry */
  pap_state = 0;
}
/*---------------------------------------------------------------------------*/
/* pap_rx() - PAP RX protocol Handler */
/*---------------------------------------------------------------------------*/
void
pap_rx(u8_t *buffer, u16_t count)
{
  u8_t *bptr=buffer;
  u8_t len;

  switch(*bptr++) {
  case CONF_REQ:	
    DEBUG1(("CONF ACK - only for server, no support\n"));
    break;
  case CONF_ACK:			/* config Ack */
    DEBUG1(("CONF ACK - PAP good - "));
    /* Display message if debug */
    len = *bptr++;
    *(bptr + len) = 0;
    DEBUG1((" %s \n",bptr));
    pap_state |= PAP_TX_UP;
    /* expire the timer to make things happen after a state change */
    TIMER_expire();
    break;
  case CONF_NAK:
    DEBUG1(("CONF NAK - Failed Auth - "));
    pap_state |= PAP_TX_AUTH_FAIL;
    /* display message if debug */
    len = *bptr++;
    *(bptr + len)=0;
    DEBUG1((" %s \n",bptr));
    break;
  }
}
/*---------------------------------------------------------------------------*/
/* pap_task() - This task needs to be called every so often during the PAP
 *	negotiation phase.  This task sends PAP REQ packets.
 */
/*---------------------------------------------------------------------------*/
void
pap_task(u8_t *buffer)	
{
  u8_t *bptr;
  u16_t t;
  PAPPKT *pkt;

  /* If LCP is up and PAP negotiated, try to bring up PAP */
  if(!(pap_state & PAP_TX_UP) && !(pap_state & PAP_TX_TIMEOUT)) {
    /* Do we need to send a PAP auth packet?
       Check if we have a request pending*/
    if(1 == TIMER_timeout(PAP_TIMEOUT)) {
      /* Check if we have a request pending */
      /* t=get_seconds()-pap_tx_time;
	 if(	t > pap_timeout)
      {
      */
      /* We need to send a PAP authentication request */
      DEBUG1(("\nSending PAP Request packet - "));

      /* Build a PAP request packet */
      pkt = (PAPPKT *)buffer;		
      
      /* Configure-Request only here, write id */
      pkt->code = CONF_REQ;
      pkt->id = ppp_id;
      bptr = pkt->data;
      
      /* Write options */
      t = strlen(pap_username);
      /* Write peer length */
      *bptr++ = (u8_t)t;	
      bptr = memcpy(bptr, pap_username, t);

      t = strlen(pap_password);
      *bptr++ = (u8_t)t;
      bptr = memcpy(bptr, pap_password, t);
			
      /* Write length */
      t = bptr - buffer;
      /* length here -  code and ID +  */
      pkt->len = uip_htons(t);	
      
      DEBUG1((" Len %d\n",t));
      
      /* Send packet */
      ahdlc_tx(PAP, buffer, 0, t, 0);

      /* Set timer */
      TIMER_set();
      
      ppp_retry++;

      /* Have we failed? */
      if(ppp_retry > 3) {
	DEBUG1(("PAP - timout\n"));
	pap_state &= PAP_TX_TIMEOUT;
	
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
