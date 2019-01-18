/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: cc2420_misc.h,v 1.1 2006/08/09 17:39:39 bg- Exp $
 */

#ifndef CC2420_MISC_H
#define CC2420_MISC_H

/*
 * Miscellaneous routines that illustrates how some of the CC2420
 * registers may be accessed.
 */

unsigned cc2420_pa_level(u16_t);
unsigned cc2420_get_pa_level(void);
int cc2420_get_current_rssi(void);

#define PA_LEVEL 0x1F

inline
unsigned
cc2420_pa_level(u16_t p)
{
  u16_t reg;

  /* 
   * All PA settings can be used, the ones given in the datasheet
   * correspond to 0, -1, -3, -5, -7, -10, -15, -25 dBm in the
   * reference design.
   */
  if (p > PA_LEVEL)
    p = PA_LEVEL;

  reg = cc2420_getreg(CC2420_TXCTRL);

  p |= reg & ~PA_LEVEL;

  cc2420_setreg(CC2420_TXCTRL, p);

  return reg & PA_LEVEL;
}

inline
unsigned
cc2420_get_pa_level(void)
{
  u16_t reg;

  reg = cc2420_getreg(CC2420_TXCTRL);

  return reg & PA_LEVEL;
}

inline
int
cc2420_get_current_rssi(void)
{
  u16_t reg;

  reg = cc2420_getreg(CC2420_RSSI);

  return (signed char) (reg & 0xff);
}

#endif /* CC2420_MISC_H */
