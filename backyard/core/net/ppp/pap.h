#ifndef __PAP_H__
#define __PAP_H__
/*
  www.mycal.com
  ---------------------------------------------------------------------------
  pap.h - pap header file 
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
 * $Id: pap.h,v 1.1 2007/05/26 07:14:40 oliverschmidt Exp $
 *
 */

#include "net/uip.h"

/* PAP state machine flags */
/* client only */
#define PAP_TX_UP		0x01
/* server only */
#define PAP_RX_UP		0x02

#define PAP_RX_AUTH_FAIL	0x10
#define PAP_TX_AUTH_FAIL	0x20
#define PAP_RX_TIMEOUT		0x80
#define PAP_TX_TIMEOUT		0x80

typedef struct _pappkt {
  u8_t code;
  u8_t id;
  u16_t len;
  u8_t data[0];	
} PAPPKT;

/* Export pap_state */
extern u8_t pap_state;

extern u8_t pap_username[];
extern u8_t pap_password[];

/* Function prototypes */
void	pap_init(void);
void	pap_rx(u8_t *, u16_t);
void	pap_task(u8_t *buffer);	

#endif /* __PAP_H__ */

