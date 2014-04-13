/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-random cc2538 Random Number Generator
 *
 * Driver for the cc2538 Hardware Random Number Generator
 * @{
 *
 * \file
 * Random number generator routines exploiting the cc2538 hardware
 * capabilities.
 *
 * This file overrides core/lib/random.c.
 */
#include "contiki.h"
#include "dev/rfcore.h"
#include "dev/cc2538-rf.h"
#include "dev/soc-adc.h"
#include "dev/sys-ctrl.h"
#include "reg.h"
/*---------------------------------------------------------------------------*/
/**
 * \brief      Generates a new random number using the cc2538 RNG.
 * \return     The random number.
 */
unsigned short
random_rand(void)
{
  uint32_t rv;

  /* Clock the RNG LSFR once */
  REG(SOC_ADC_ADCCON1) |= SOC_ADC_ADCCON1_RCTRL0;

  rv = REG(SOC_ADC_RNDL) | (REG(SOC_ADC_RNDH) << 8);
  return ((unsigned short)rv);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Seed the cc2538 random number generator.
 * \param seed Ignored. It's here because the function prototype is in core.
 *
 *             We form a seed for the RNG by sampling IF_ADC as
 *             discussed in the user guide.
 *             Seeding with this method should not be done during
 *             normal radio operation. Thus, use this function before
 *             initialising the network.
 *
 * \note       Must not be called after the RF driver has been initialised and is
 *             in normal operation. If it is absolutely necessary to do so, the
 *             radio will need re-initialised.
 */
void
random_init(unsigned short seed)
{
  int i;
  unsigned short s = 0;

  /* Make sure the RNG is on */
  REG(SOC_ADC_ADCCON1) &= ~(SOC_ADC_ADCCON1_RCTRL1 | SOC_ADC_ADCCON1_RCTRL0);

  /* Enable clock for the RF Core */
  REG(SYS_CTRL_RCGCRFC) = 1;

  /* Wait for the clock ungating to take effect */
  while(REG(SYS_CTRL_RCGCRFC) != 1);

  /* Infinite RX - FRMCTRL0[3:2] = 10
   * This will mess with radio operation - see note above */
  REG(RFCORE_XREG_FRMCTRL0) = 0x00000008;

  /* Turn RF on */
  CC2538_RF_CSP_ISRXON();

  /*
   * Wait until "the chip has been in RX long enough for the transients to
   * have died out. A convenient way to do this is to wait for the RSSI-valid
   * signal to go high."
   */
  while(!(REG(RFCORE_XREG_RSSISTAT) & RFCORE_XREG_RSSISTAT_RSSI_VALID));

  /*
   * Form the seed by concatenating bits from IF_ADC in the RF receive path.
   * Keep sampling until we have read at least 16 bits AND the seed is valid
   *
   * Invalid seeds are 0x0000 and 0x8003 and should not be used.
   */
  while(s == 0x0000 || s == 0x8003) {
    for(i = 0; i < 16; i++) {
      s |= (REG(RFCORE_XREG_RFRND) & RFCORE_XREG_RFRND_IRND);
      s <<= 1;
    }
  }

  /* High byte first */
  REG(SOC_ADC_RNDL) = (s >> 8) & 0x00FF;
  REG(SOC_ADC_RNDL) = s & 0xFF;

  /* RF Off. NETSTACK_RADIO.init() will sort out normal RF operation */
  CC2538_RF_CSP_ISRFOFF();
}

/**
 * @}
 * @}
 */
