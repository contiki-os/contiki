/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         Model-specific functions for Sensinode devices.
 *
 *         Bankable
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "dev/models.h"
#include "dev/uart1.h"
#include "dev/m25p16.h"
/*---------------------------------------------------------------------------*/
void
model_init()
{
#ifdef MODEL_N740
  /*
   * We want to prevent the dongle from controlling the state of the
   * analog switch on the N740s.
   *
   * Set P0_3 as out and start with P0_3=0 (USB and Light selected)
   */
  P0DIR |= 0x08; /* P0_3 out */
  P0_3 = 0;

  /* Init the serial-parallel chip for N740s. This will also 'init' LEDs */
  n740_ser_par_init();

  /* Put the Serial Flash in Deep Power mode */
  n740_analog_deactivate();

#if M25P16_CONF_ON
  m25p16_dp();
#endif /* SERIAL_FLASH_CONF_ON */

  n740_ser_par_set(0);
#endif
}
/*---------------------------------------------------------------------------*/
void
model_uart_intr_en()
{
#ifdef MODEL_N740
  /*
   * Dirty, ugly hack for the N740 USART1 RX issue:
   * When the USB is for whatever reason disabled (either disconnected or the
   * analog switch has switched to the D-connector), RX starts flowing down
   * pin 1.7 (and the line stays low), resulting in non-stop UART1_RX
   * interrupts. So, we only acknowledge the interrupt when the line is
   * high and the dongle is connected (thus we are on USB).
   *
   * For all other models, just turn the interrupt on
   *
   * Interrupts will only turn on if UART_ONE_CONF_WITH_INPUT is defined 1
   */
  if(P1_7 == 1 && P0_3 == 0) {
    UART1_RX_INT(1);
  }
#else
  UART1_RX_INT(1);
#endif
}
