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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */

#include <apple2.h>

#include "contiki.h"
#include "sys/log.h"

/* The enhanced Apple//e doesn't have a hardware clock whatsoever. Therefore the
 * cc65 C-library for this target doesn't include an implementation of clock().
 */

static unsigned char tick = 14;
static clock_time_t time;

/*-----------------------------------------------------------------------------------*/
void
clock_init(void)
{
  if(get_ostype() >= APPLE_IIGS) {
    if(*(signed char *)0xC036 < 0) {
      /* 5 / 14 = 1.0MHz / 2.8MHz */
      tick = 5;
      log_message("Assuming 2.8 MHz ...", "");
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
clock_update(void)
{
  static unsigned int count;

  count += tick;
  if(count > 1000) {
    count = 0;
    ++time;
  }
}
/*-----------------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return time;
}
/*-----------------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return time / CLOCK_CONF_SECOND;
}
/*-----------------------------------------------------------------------------------*/
