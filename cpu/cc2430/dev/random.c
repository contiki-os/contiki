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
 *         Random number generator routines exploiting the cc2430 hardware
 *         capabilities.
 *
 *         This file overrides core/lib/random.c.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "cc2430_sfr.h"
#include "dev/cc2430_rf.h"
/*---------------------------------------------------------------------------*/
/**
 * \brief      Generates a new random number using the cc2430 RNG.
 * \return     The random number.
 */
unsigned short
random_rand(void)
{
  /* Clock the RNG LSFR once */
  ADCCON1 |= ADRCTRL0;

  return (RNDL | (RNDH << 8));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Seed the cc2430 random number generator.
 * \param seed Seed value for the RNG.
 *
 *             If the SEED argument is 0, seed the RNG with IF_ADC as
 *             discussed in the cc2430 datasheet (rev. 2.1), section 13.11.2.2,
 *             page 134. Seeding with this method should not be done during
 *             normal radio operation. Thus, use this function before
 *             initialising the network.
 *
 *             If the SEED is provided, seed with this value instead. This will
 *             result in the same sequence of random numbers each time the node
 *             reboots. So, don't use it unless you have a reason (e.g. tests)
 */
void
random_init(unsigned short seed)
{
  int i;

  /* Comment out this if() block to save a nice 16 bytes of code size */
  if(seed) {
    /* If the caller provides a seed, write the high-byte first and then
     * write the low byte */
    RNDL = seed >> 8;   /* High byte first */
    RNDL = seed & 0xFF;
    return;
  }

  /*
   * cc2430 Datasheet:
   * "When a true random value is required, the LFSR should be seeded by
   * writing RNDL with random values from the IF_ADC in the RF receive path."
   *
   * "To use this seeding method, the radio must first be powered on by
   * enabling the voltage regulator"
   */
  RFPWR &= ~RREG_RADIO_PD;        /* Turn on the voltage regulator */
  while(!(RFIF & IRQ_RREG_ON));   /* Wait for power up*/

  /* OK, it's powered. The respective interrupt flag has been set, clear it */
  RFIF &= ~IRQ_RREG_ON;

  /*
   * "The radio should be placed in infinite TX state, to avoid possible sync
   * detect in RX state."
   *
   * Judging by old chipcon cc2430 code examples as well as by the way cc2530
   * works, this is very likely to be "RX state" (i.e. a typo in the datasheet)
   *
   * With infinite TX, ADCTSTx always read as 0 so we'll use infinite RX
   */
  MDMCTRL1L = 0x02; /* RX mode 10 - RX_INFINITE state */

  /* "Enter RX State - Immediate" command strobe */
  cc2430_rf_command(ISRXON);

  /* Make sure the RNG is on */
  ADCCON1 &= ~(ADRCTRL1 | ADRCTRL0);

  /* Wait for IF_ADC I-branch and Q-branch values */
  while(!(ADCTSTH & ADCTSTL));

  /* 32 times as per the chipcon example. This seems to increase randomness */
  for(i = 0; i < 32; i++) {
    /* Seed the RNG by writing into RNDL twice with values from ADCTSTx */
    RNDL = ADCTSTH;
    RNDL = ADCTSTL;

    /* Clock the RNG LSFR once */
    ADCCON1 |= ADRCTRL0;
  }

  /*
   * Exit RX state. Just shut down, network initialisation will take care of
   * properly starting the radio for us.
   */
  RFPWR |= RREG_RADIO_PD;
}
