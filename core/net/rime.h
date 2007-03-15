/**
 * \defgroup rime Rime - a Lightweight Layered Communication Stack for
 * Contiki
 *
 * Rime is a communication stack for Contiki that consists of a number
 * of modules that individually are very simple, but together form a
 * feature-rich communication stack.
 *
 * Rime modules:
 *
 * abc: Anonymous link-local BroadCast
 * ibc: Identified link-local BroadCast
 * uc: link-local UniCast
 * suc: Stubborn link-local UniCast
 * ruc: Reliable link-local UniCast
 * sibc: Stubborn Identified link-local BroadCast
 *
 * sabc: Stubborn Anonymous link-local BroadCast
 * nf: Network Flooding
 * @{
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: rime.h,v 1.1 2007/03/15 21:24:11 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RIME_H__
#define __RIME_H__

#include "net/rime/ruc.h"
#include "net/rime/sibc.h"
#include "net/rime/nf.h"
#include "net/rime/mesh.h"
#include "net/rime/tree.h"
#include "net/rime/ctimer.h"

#include "net/rime/rimebuf.h"

#include "net/rime/rime-types.h"

#include "net/rime/channel-assignments.h"

#include "net/rime/rime-debug.h"

void rime_init(void);
void rime_input(void);

/**
 * \brief      Rime calls this function to send out a packet
 *
 *             This function must be implemented by the driver running
 *             below Rime. It is called by abRime to send out a
 *             packet. The packet is consecutive in the rimebuf. A
 *             pointer to the first byte of the packet is obtained
 *             with the rimebuf_hdrptr() function. The length of the
 *             packet to send is obtained with the rimebuf_totlen()
 *             function.
 *
 *             The driver, which typically is a MAC protocol, may
 *             queue the packet by using the queuebuf functions.
 */
void rime_driver_send(void);

#endif /* __RIME_H__ */
