/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Jim Paris <jim.paris@rigado.com>
 */

#include <contiki.h>
#include "rtimer-arch.h"
#include "aducrf101-contiki.h"

/* rtimer on the ADuCRF101 is implemented with the "wakeup" timer.
   (timer 2).  It should be clocked from an external crystal,
   but if that doesn't seem to be present, this code will select the
   imprecise internal 32.768 KHz oscillator instead. */

static void
_timer2_enable(int enable)
{
  T2CON_ENABLE_BBA = enable;
  clock_time_t now = clock_time();
  while(T2STA_CON_BBA) {
    /* Synchronizing settings may fail if the chosen clock isn't running;
       wait no more than 1ms for it */
    if((clock_time() - now) > (CLOCK_SECOND / 1000)) {
      break;
    }
  }
}
static uint32_t
_timer2_val(void)
{
  /* This is atomic because the FREEZE bit is set in T2CON. */
  uint32_t now;
  now = pADI_WUT->T2VAL0;
  now |= pADI_WUT->T2VAL1 << 16;
  return now;
}
static uint32_t
_timer2_measure_freq(void)
{
  const int test_usec = 10000;
  uint32_t now = _timer2_val();
  clock_delay_usec(test_usec);
  return (_timer2_val() - now) * (1000000 / test_usec);
}
void
rtimer_arch_init(void)
{
  uint32_t freq;
  const char *timer = "LFXTAL";

  _timer2_enable(0);
  pADI_WUT->T2CON = T2CON_PRE_DIV1 | T2CON_MOD_FREERUN | T2CON_FREEZE_EN |
    T2CON_WUEN_EN;

  /* Try 32.768 KHz crystal */
  pADI_WUT->T2CON |= T2CON_CLK_LFXTAL;
  _timer2_enable(1);
  freq = _timer2_measure_freq();

  if(freq < 20000 || freq > 40000) {
    /* No good; use 32.768 KHz internal oscillator */
    _timer2_enable(0);
    pADI_WUT->T2CON &= ~T2CON_CLK_MSK;
    pADI_WUT->T2CON |= T2CON_CLK_LFOSC;
    _timer2_enable(1);
    freq = _timer2_measure_freq();
    timer = "LFOSC";
  }

  printf("Using %s for rtimer (%ld Hz)\n", timer, freq);

  /* Enable interrupt in NVIC, but disable in WUT for now. */
  pADI_WUT->T2IEN = 0;
  NVIC_EnableIRQ(WUT_IRQn);
}
rtimer_clock_t
rtimer_arch_now(void)
{
  /* This is atomic because the FREEZE bit is set in T2CON. */
  return _timer2_val();
}
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  uint32_t now = _timer2_val();

  /* Minimum of 5 wakeup timer ticks */
  if((int32_t)(t - now) < 5) {
    t = now + 5;
  }

  /* Set T2WUFB to match at target time */
  T2IEN_WUFB_BBA = 0;
  pADI_WUT->T2WUFB0 = (t & 0xffff);
  pADI_WUT->T2WUFB1 = (t >> 16);
  T2IEN_WUFB_BBA = 1;
}
void
WakeUp_Int_Handler(void)
{
  /* clear interrupt */
  T2CLRI_WUFB_BBA = 1;
  /* disable T2WUFB match */
  T2IEN_WUFB_BBA = 0;
  rtimer_run_next();
}
