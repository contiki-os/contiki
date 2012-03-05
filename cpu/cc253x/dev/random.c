/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 *         Random number generator routines exploiting the cc2530 hardware
 *         capabilities.
 *
 *         This file overrides core/lib/random.c.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "cc253x.h"
#include "sfr-bits.h"
#include "dev/cc2530-rf.h"
/*---------------------------------------------------------------------------*/
/**
 * \brief      Generates a new random number using the cc253x RNG.
 * \return     The random number.
 */
unsigned short
random_rand(void)
{
  /* Clock the RNG LSFR once */
  ADCCON1 |= ADCCON1_RCTRL0;

  return (RNDL | (RNDH << 8));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Seed the cc253x random number generator.
 * \param seed Ignored. It's here because the function prototype is in core.
 *
 *             We form a seed for the RNG by sampling IF_ADC as
 *             discussed in the user guide.
 *             Seeding with this method should not be done during
 *             normal radio operation. Thus, use this function before
 *             initialising the network.
 */
void
random_init(unsigned short seed)
{
  int i;

  /* Make sure the RNG is on */
  ADCCON1 &= ~(ADCCON1_RCTRL1 | ADCCON1_RCTRL0);

  /* Infinite RX */
  FRMCTRL0 = FRMCTRL0_RX_MODE1;

  /* Turn RF on */
  CC2530_CSP_ISRXON();

  /* Wait until (user guide sec. 23.12, p 239) "the chip has been in RX long
   * enough for the transients to have died out. A convenient way to do this is
   * to wait for the RSSI-valid signal to go high." */
  while(!(RSSISTAT & RSSISTAT_RSSI_VALID));

  /*
   * Form the seed by concatenating bits from IF_ADC in the RF receive path.
   * Keep sampling until we have read at least 16 bits AND the seed is valid
   *
   * Invalid seeds are 0x0000 and 0x8003 - User Guide (sec. 14.2.2 p. 146):
   * "Note that a seed value of 0x0000 or 0x8003 always leads to an unchanged
   * value in the LFSR after clocking, as no values are pushed in via in_bit
   * (see Figure 14-1); hence, neither of these seed values should not be used
   * for random-number generation."
   */
  i = 0;
  while(i < 16 || (seed == 0x0000 || seed == 0x8003)) {
    seed = (seed << 1) | (RFRND & RFRND_IRND);
    seed <<= 1;
    i++;
  }

  /* High byte first */
  RNDL = seed >> 8;
  RNDL = seed & 0xFF;

  /* RF Off. NETSTACK_RADIO.init() will sort out normal RF operation */
  CC2530_CSP_ISRFOFF();
}
