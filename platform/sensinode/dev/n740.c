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
 *   This file provides functions to control various chips on the
 *   Sensinode N740s:
 *
 *   - The 74HC595D is an 8-bit serial in-parallel out shift register.
 *     LEDs are connected to this chip. It also serves other functions such as
 *     enabling/disabling the Accelerometer (see n740.h).
 *   - The 74HC4053D is a triple, 2-channel analog mux/de-mux.
 *     It switches I/O between the USB and the D-Connector.
 *     It also controls P0_0 input source (Light Sensor / External I/O)
 *
 *   Mux/De-mux: Connected to P0_3 (set to output in models.c
 *     Changing the state of the mux/demux can have catastrophic (tm) results
 *     on our software. If we are not careful, we risk entering a state where
 *     UART1 RX interrupts are being generated non-stop. Only change its state
 *     via the function in this file.
 *
 *   Shift Register:
 *     For the shift register we can:
 *     - write a new instruction
 *     - remember and retrieve the last instruction sent
 *
 *     The chip is connected to CPU pins as follows:
 *     - P0_2: Serial Data Input
 *     - P1_3: Shift Register Clock Input
 *     - P1_1: Storage Register Clock
 *
 *   This file can be placed in any bank.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "dev/n740.h"
#include "dev/uart1.h"

/*
 * This variable stores the most recent instruction sent to the ser-par chip.
 * We declare it as static and return its value through n740_ser_par_get().
 */
static __data uint8_t ser_par_status;

/*---------------------------------------------------------------------------*/
/* Init the serial-parallel chip:
 *   - Set I/O direction for all 3 pins (P0_2, P1_1 and P1_3) to output
 */
void
n740_ser_par_init()
{
  /* bus_init and uart1_init also touch the I/O direction for those pins */
  P1DIR |= 0x0A;
  P0DIR |= 0x04;
}
/*---------------------------------------------------------------------------*/
/*
 * Send a command to the N740 serial-parallel chip. Each command is a single
 * byte, each bit controls a different feature on the sensor.
 */
void
n740_ser_par_set(uint8_t data)
{
  uint8_t i;
  uint8_t mask = 1;
  uint8_t temp = 0;

  DISABLE_INTERRUPTS();
  /* bit-by-bit */
  for(i = 0; i < 8; i++) {
    temp = (data & mask);
    /* Is the bit set? */
    if(i && temp) {
      /* If it was set, we want to send 1 */
      temp >>= i;
    }
    /* Send the bit */
    P0_2 = temp;
    /* Shift */
    P1_3 = 1;
    P1_3 = 0;
    mask <<= 1;
  }
  /* Move to Par-Out */
  P1_1 = 1;
  P1_1 = 0;
  ENABLE_INTERRUPTS();

  /* Right, we're done. Save the new status in ser_par_status */
  ser_par_status = data;
}
/*---------------------------------------------------------------------------*/
/* This function returns the last value sent to the ser-par chip on the N740.
 *
 * The caveat here is that we must always use n740_set_ser_par() to send
 * commands to the ser-par chip, never write directly.
 *
 * If any other function sends a command directly, ser_par_status and the
 * actual status will end up out of sync.
 */
uint8_t
n740_ser_par_get()
{
  return ser_par_status;
}
/*---------------------------------------------------------------------------*/
void
n740_analog_switch(uint8_t state)
{
  /* Turn off the UART RX interrupt before switching */
  DISABLE_INTERRUPTS();
  UART1_RX_INT(0);

  /* Switch */
  P0_3 = state;

  /* If P0_3 now points to the USB and nothing is flowing down P1_7,
   * enable the interrupt again */
  if(P1_7 == 1 && P0_3 == N740_ANALOG_SWITCH_USB) {
    UART1_RX_INT(1);
  }
  ENABLE_INTERRUPTS();
}
/*---------------------------------------------------------------------------*/
/*
 * Activate the the 74HC4053D analog switch U5 on the N740 and at the same
 * time set relevant pins to Peripheral I/O mode. This stops communications
 * with the serial flash and enables UART1 I/O
 */
void
n740_analog_activate()
{
  uint8_t ser_par = n740_ser_par_get();
  ser_par &= ~N740_SER_PAR_U5_ENABLE; /* Turn on */

  N740_PINS_PER_IO();

  n740_ser_par_set(ser_par);
}
/*---------------------------------------------------------------------------*/
/*
 * De-Activate the the 74HC4053D analog switch U5 on the N740 and at the same
 * time set relevant pins to GP I/O mode. This effectively prepares us to
 * start talking with the serial flash chip
 */
void
n740_analog_deactivate()
{
  uint8_t ser_par = n740_ser_par_get();
  ser_par |= N740_SER_PAR_U5_ENABLE; /* Turn off */

  n740_ser_par_set(ser_par);

  N740_PINS_GPIO();
}
/*---------------------------------------------------------------------------*/
