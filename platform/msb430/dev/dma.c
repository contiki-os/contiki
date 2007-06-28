/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 */

/**
 * \file
 *	DMA interrupt handling.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <io.h>
#include <signal.h>

#include "contiki-msb430.h"
#include "dev/cc1020.h"

interrupt(DACDMA_VECTOR) irq_dacdma(void)
{
  static unsigned char line;

  if (DMA0CTL & DMAIFG) {
    DMA0CTL &= ~(DMAIFG | DMAIE);
    line = 0;
    process_post(&cc1020_sender_process, cc1020_event, &line);
  }

  if (DMA1CTL & DMAIFG) {
    DMA1CTL &= ~(DMAIFG | DMAIE);
    line = 1;
    process_post(&cc1020_sender_process, cc1020_event, &line);
  }

  if (DAC12_0CTL & DAC12IFG) {
    DAC12_0CTL &= ~(DAC12IFG | DAC12IE);
  }

  if (DAC12_1CTL & DAC12IFG) {
    DAC12_1CTL &= ~(DAC12IFG | DAC12IE);
  }
}

void
dma_transfer(char *buf, unsigned len)
{
    // Configure DMA Channel 0 for UART0 TXIFG.
    DMACTL0 = DMA0TSEL_4;

    // No DMAONFETCH, ROUNDROBIN, ENNMI.
    DMACTL1 = 0x0000;

    /*
     * Single transfer mode, dstadr unchanged, srcadr 
     * incremented, byte access
     * Important to use DMALEVEL when using USART TX 
     * interrupts so first edge 
     * doesn't get lost (hangs every 50. - 100. time)!
     */
    DMA0CTL =
      DMADT_0 | DMADSTINCR_0 | DMASRCINCR_3 | DMASBDB | DMALEVEL | DMAIE;
    DMA0SA = (unsigned) buf;
    DMA0DA = (unsigned) &TXBUF0;
    DMA0SZ = len;
    DMA0CTL |= DMAEN;           // enable DMA
    U0CTL &= ~SWRST;            // enable UART, starts transfer
}
