/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dma.h"
#include "imr.h"
#include "msg-bus.h"

/*---------------------------------------------------------------------------*/
void
quarkX1000_imr_conf(void)
{
  quarkX1000_imr_t imr;
  int imr_idx = 0;

  imr.lo.raw = 0;
  imr.hi.raw = 0;
  imr.rdmsk.raw = 0;
  imr.wrmsk.raw = 0;

  imr.lo.lock = 1;

  imr.rdmsk.cpu0 = imr.rdmsk.cpu_0 = 1;
  imr.wrmsk.cpu0 = imr.wrmsk.cpu_0 = 1;

  quarkX1000_msg_bus_init();

  imr.lo.addr = 0;
  imr.hi.addr = (((uint32_t)&_sbss_dma_addr) - 1) >> QUARKX1000_IMR_SHAMT;
  quarkX1000_imr_write(imr_idx, imr);
  imr_idx++;

  imr.lo.addr = ((uint32_t)&_ebss_dma_addr) >> QUARKX1000_IMR_SHAMT;
  imr.hi.addr = ~0;
  quarkX1000_imr_write(imr_idx, imr);
  imr_idx++;

  imr.lo.addr = 0;
  imr.hi.addr = 0;
  imr.rdmsk.raw = ~0;
  imr.wrmsk.raw = ~0;

  /* Lock the other IMRs open */
  while(imr_idx < QUARKX1000_IMR_CNT) {
    quarkX1000_imr_write(imr_idx, imr);
    imr_idx++;
  }

#ifndef DBG_IMRS
  /* The IMRs are locked by the hardware, but the message bus could still
   * provide access to other potentially-sensitive functionality.
   */
  quarkX1000_msg_bus_lock();
#endif
}
/*---------------------------------------------------------------------------*/
