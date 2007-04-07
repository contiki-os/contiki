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
 * $Id: xmac.c,v 1.1 2007/04/07 05:43:31 adamdunkels Exp $
 */

/**
 * \file
 *         A simple power saving MAC protocol based on X-MAC [SenSys 2006]
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "sys/pt.h"
#include "net/mac/xmac.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "net/rime.h"
#include "dev/radio.h"

struct powercycle {
  struct pt pt;
  struct rtimer rt;
  rtimer_clock_t ontime, offtime;
  int num_strobes;
};

struct xmac_hdr {
  rimeaddr_t receiver;
};

static struct rtimer rt;
static struct pt pt;

#define BASE_TIME (RTIMER_ARCH_SECOND / 100)

#define OFF_TIME BASE_TIME * 10
#define ON_TIME BASE_TIME
#define STROBE_WAIT_TIME ON_TIME / 10

static volatile unsigned char should_be_awake = 0;
static volatile unsigned char radio_is_on = 0;

static int (*send_func)(const u8_t *buf, u8_t len);

#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE

#define DEBUG 0
#if DEBUG
#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#define LEDS_TOGGLE(x) leds_toggle(x)
#else
#define LEDS_ON(x)
#define LEDS_OFF(x)
#define LEDS_TOGGLE(x)
#endif

/*---------------------------------------------------------------------------*/
static void
powercycle(struct rtimer *t, void *ptr)
{
  static int awake_periods = 0;
  PT_BEGIN(&pt);

  while(1) {
    if(should_be_awake == 0) {
      awake_periods = 0;
      radio_off();
      radio_is_on = 0;
      LEDS_OFF(LEDS_RED);
    } else {
      awake_periods++;
      /*      if(awake_periods > 4) {
	awake_periods = 0;
	should_be_awake = 0;
	}*/
    }
    rtimer_set(t, RTIMER_TIME(t) + OFF_TIME, 1, powercycle, ptr);
    PT_YIELD(&pt);
    
    if(radio_is_on == 0) {
      radio_on();
      radio_is_on = 1;
      LEDS_ON(LEDS_RED);
    }
    rtimer_set(t, RTIMER_TIME(t) + ON_TIME, 1, powercycle, ptr);
    PT_YIELD(&pt);

  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
void
xmac_init(void)
{
  radio_is_on = 0;
  should_be_awake = 0;
  PT_INIT(&pt);
  rtimer_set(&rt, RTIMER_NOW() + OFF_TIME, 1, powercycle, NULL);

  rime_set_output(xmac_send);
}
/*---------------------------------------------------------------------------*/
void
xmac_send(void)
{
  rtimer_clock_t t0, t1;
  int strobes;

  /*  printf("xmac_send\n");*/
    
  rimebuf_hdralloc(sizeof(struct xmac_hdr));
  rimebuf_compact();

  should_be_awake = 1;
  while(!radio_is_on);

  t0 = RTIMER_NOW();
  strobes = 0;

  do {
    struct xmac_hdr msg;
    rtimer_clock_t t;

    t = RTIMER_NOW();
    
    LEDS_TOGGLE(LEDS_GREEN);
    msg.receiver.u16[0] = 0;
    if(send_func) {
      send_func((const u8_t *)&msg, sizeof(struct xmac_hdr));
    }
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t + STROBE_WAIT_TIME));

    /*    printf("Strobe %d\n", strobes);*/
    ++strobes;
  } while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + OFF_TIME + ON_TIME));
  
  if(send_func) {
    send_func(rimebuf_hdrptr(), rimebuf_totlen());
  }
  LEDS_OFF(LEDS_GREEN);
  
  should_be_awake = 0;

  /*  printf("xmac_send after\n");*/
}
/*---------------------------------------------------------------------------*/
void
xmac_set_sendfunc(int (*f)(const u8_t *, u8_t))
{
  send_func = f;
}
/*---------------------------------------------------------------------------*/
void
xmac_input(void)
{
  struct xmac_hdr *hdr = rimebuf_dataptr();
  
  rimebuf_hdrreduce(sizeof(struct xmac_hdr));
  if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_node_addr)) {
    /* XXX Send an ACK and wait for packet . */
    should_be_awake = 1;
  } else if(rimeaddr_cmp(&hdr->receiver, &rimeaddr_null)) {
    /* Wait for the actual packet to arrive. */
    should_be_awake = 1;
  } else {
    /* Go back to sleep. */
    should_be_awake = 0;
  }

  if(rimebuf_totlen() > 0) {
    
    /* We have received the final packet, so we can go back to being
       asleep. */
    
    should_be_awake = 0;
    rime_input();
  }
}
/*---------------------------------------------------------------------------*/
