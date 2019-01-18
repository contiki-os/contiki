#ifndef __IPCP_H__
#define __IPCP_H__
/*  www.mycal.com
    ---------------------------------------------------------------------------
    IPCP.h - Internet Protocol Control Protocol header file 
    ---------------------------------------------------------------------------
    Version 
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
 * $Id: ipcp.h,v 1.1 2007/05/26 07:14:39 oliverschmidt Exp $
 *
 */
#include "net/uip.h"
#include "contiki-conf.h"

/* Config options (move to pppconfig)
   #define  IPCP_RETRY_COUNT	5
   #define  IPCP_TIMEOUT	5
*/

/* IPCP Option Types */
#define IPCP_IPADDRESS		0x03
#define IPCP_PRIMARY_DNS	0x81
#define IPCP_SECONDARY_DNS	0x83

/* IPCP state machine flags */
#define	IPCP_TX_UP		0x01
#define IPCP_RX_UP		0x02
#define IPCP_IP_BIT		0x04
#define IPCP_TX_TIMEOUT		0x08
#define IPCP_PRI_DNS_BIT	0x08
#define IPCP_SEC_DNS_BIT	0x10

typedef struct  _ipcp
{
  u8_t code;
  u8_t id;
  u16_t len;
  u8_t data[0];	
} IPCPPKT;

/*
 * Export IP addresses.
 */
#if 0	/* moved to mip.c */
extern	IPAddr our_ipaddr;
extern	IPAddr peer_ip_addr;
extern	IPAddr pri_dns_addr;
extern	IPAddr sec_dns_addr;
#endif

extern	u8_t ipcp_state;

void ipcp_init(void);
void ipcp_task(u8_t *buffer);
void ipcp_rx(u8_t *, u16_t);

#endif /* __IPCP_H__ */

