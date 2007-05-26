#ifndef __LCP_H__
#define __LCP_H__
/* www.mycal.com
   ---------------------------------------------------------------------------
   LCP.h - LCP header file
   ---------------------------------------------------------------------------
   Version                                                                  -
   0.1 Original Version June 3, 2000
   (c)2000 Mycal Labs, All Rights Reserved
   ---------------------------------------------------------------------------
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
 * $Id: lcp.h,v 1.1 2007/05/26 07:14:39 oliverschmidt Exp $
 *
 */

#include "net/uip.h"
#include "contiki-conf.h"

/* LCP Option Types */
#define LPC_VENDERX		0x0
#define	LPC_MRU			0x1
#define LPC_ACCM		0x2
#define LPC_AUTH		0x3
#define LPC_QUALITY		0x4
#define LPC_MAGICNUMBER		0x5
#define LPC_PFC			0x7
#define LPC_ACFC		0x8

/* LCP Negotiated options flag equates */
#define LCP_OPT_ACCM		0x1
#define LCP_OPT_AUTH		0x2
#define LCP_OPT_PFC		0x4
#define LCP_OPT_ACFC		0x4

/* LCP state machine flags */
#define	LCP_TX_UP		0x1
#define LCP_RX_UP		0x2

#define LCP_RX_AUTH		0x10
/* LCP request for auth */
#define LCP_TERM_PEER		0x20
/* LCP Terminated by peer */
#define LCP_RX_TIMEOUT		0x40
#define LCP_TX_TIMEOUT		0x80

typedef struct _lcppkt
{
  u8_t code;
  u8_t id;
  u16_t len;
  u8_t data[0];	
} LCPPKT;

/* Exported Vars */
extern	u8_t lcp_state;

void	lcp_init(void);
void	lcp_rx(u8_t *, u16_t);
void	lcp_task(u8_t *buffer);
void	lcp_disconnect(u8_t id);

#endif /* __LCP_H__ */
