/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: cxmac.h,v 1.5 2010/02/23 20:09:11 nifi Exp $
 */

/**
 * \file
 *         A simple power saving MAC protocol based on X-MAC [SenSys 2006]
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __CXMAC_H__
#define __CXMAC_H__

#include "sys/rtimer.h"
#include "net/mac/rdc.h"
#include "dev/radio.h"

#define CXMAC_RECEIVER "cxmac.recv"
#define CXMAC_STROBES "cxmac.strobes"
#define CXMAC_SEND_WITH_ACK "cxmac.send.ack"
#define CXMAC_SEND_WITH_NOACK "cxmac.send.noack"


struct cxmac_config {
  rtimer_clock_t on_time;
  rtimer_clock_t off_time;
  rtimer_clock_t strobe_time;
  rtimer_clock_t strobe_wait_time;
};

extern const struct rdc_driver cxmac_driver;


#endif /* __CXMAC_H__ */
