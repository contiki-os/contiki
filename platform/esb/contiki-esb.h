/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: contiki-esb.h,v 1.6 2010/03/02 22:40:39 nifi Exp $
 */
#ifndef __CONTIKI_ESB_H__
#define __CONTIKI_ESB_H__

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#include "lib/sensors.h"
#include "dev/hwconf.h"

#include "dev/lpm.h"

#include "dev/rs232.h"

#include "dev/serial-line.h"
#include "dev/slip.h"

#include "node-id.h"

#include "dev/vib-sensor.h"
#include "dev/pir-sensor.h"
#include "dev/button-sensor.h"
#include "dev/radio-sensor.h"
#include "dev/sound-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/temperature-sensor.h"
#include "dev/ctsrts-sensor.h"

#include "dev/beep.h"

#include "dev/ir.h"

#include "dev/leds.h"

#include "node-id.h"

void msp430_cpu_init(void);
void init_lowlevel(void);
void init_apps(void);

#endif /* __CONTIKI_ESB_H__ */
