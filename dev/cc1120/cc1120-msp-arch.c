/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 */

#include "contiki.h"
#include "contiki-net.h"

/*#include "dev/cc1120-arch.h"*/
#include "dev/spi.h"
#include "dev/leds.h"

#include "isr_compat.h"
#include <stdio.h>

#define CC1120_GDO0_PORT(type) P1##type
#define CC1120_GDO0_PIN        7

#define CC1120_GDO2_PORT(type) P1##type
#define CC1120_GDO2_PIN        3

#define CC1120_GDO3_PORT(type) P8##type
#define CC1120_GDO3_PIN        1

#define CC1120_SPI_CSN_PORT(type)  P3##type
#define CC1120_SPI_CSN_PIN     0
#define CC1120_SPI_MOSI_PORT(type)  P3##type
#define CC1120_SPI_MOSI_PIN    1
#define CC1120_SPI_MISO_PORT(type)  P3##type
#define CC1120_SPI_MISO_PIN    2
#define CC1120_SPI_SCLK_PORT(type)  P3##type
#define CC1120_SPI_SCLK_PIN    3

int cc1120_rx_interrupt(void);
/*---------------------------------------------------------------------------*/
void
cc1120_arch_spi_enable(void)
{
  /* Set CSn to low (0) */
  CC1120_SPI_CSN_PORT(OUT) &= ~BV(CC1120_SPI_CSN_PIN);

  /* The MISO pin should go high before chip is fully enabled. */
  while((CC1120_SPI_MISO_PORT(IN) & BV(CC1120_SPI_MISO_PIN)) != 0);
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_spi_disable(void)
{
  /* Set CSn to high (1) */
  CC1120_SPI_CSN_PORT(OUT) |= BV(CC1120_SPI_CSN_PIN);
}
/*---------------------------------------------------------------------------*/
int
cc1120_arch_spi_rw_byte(unsigned char c)
{
  SPI_WAITFORTx_BEFORE();
  SPI_TXBUF = c;
  SPI_WAITFOREOTx();
  SPI_WAITFOREORx();
  c = SPI_RXBUF;

  return c;
}
/*---------------------------------------------------------------------------*/
int
cc1120_arch_spi_rw(unsigned char *inbuf, unsigned char *outbuf, int len)
{
  int i;
  if(inbuf == NULL && outbuf == NULL) {
    /* error: both buffers are NULL */
    return 1;
  } else if(inbuf == NULL) {
    for(i = 0; i < len; i++) {
      SPI_WRITE(outbuf[i]);
    }
  } else if(outbuf == NULL) {
    for(i = 0; i < len; i++) {
      SPI_READ(inbuf[i]);
    }
  } else {
    for(i = 0; i < len; i++) {
      SPI_WAITFORTx_BEFORE();
      SPI_TXBUF = outbuf[i];
      SPI_WAITFOREOTx();
      SPI_WAITFOREORx();
      inbuf[i] = SPI_RXBUF;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_init(void)
{
  spi_init();

  /* all input by default, set these as output */
  CC1120_SPI_CSN_PORT(DIR) |= BV(CC1120_SPI_CSN_PIN);

  /* Unselect radio. */
  cc1120_arch_spi_disable();

  CC1120_GDO0_PORT(SEL) &= ~BV(CC1120_GDO0_PIN);
  CC1120_GDO0_PORT(DIR) &= ~BV(CC1120_GDO0_PIN);

  CC1120_GDO2_PORT(SEL) &= ~BV(CC1120_GDO2_PIN);
  CC1120_GDO2_PORT(DIR) &= ~BV(CC1120_GDO2_PIN);

  CC1120_GDO3_PORT(SEL) &= ~BV(CC1120_GDO3_PIN);
  CC1120_GDO3_PORT(DIR) &= ~BV(CC1120_GDO3_PIN);

  /* Reset procedure */
  CC1120_SPI_SCLK_PORT(OUT) |= BV(CC1120_SPI_SCLK_PIN);
  CC1120_SPI_MOSI_PORT(OUT) |= BV(CC1120_SPI_MOSI_PIN);

  CC1120_SPI_CSN_PORT(OUT) &= ~BV(CC1120_SPI_CSN_PIN);
  CC1120_SPI_CSN_PORT(OUT) |= BV(CC1120_SPI_CSN_PIN);

  clock_delay_usec(400);

  /* Rising edge interrupt; note that GPIO-pins are hardwired to 0/1/tristate
   * (depending on settings) when cc1120 is in SLEEP state (SPWD strobe). See
   * CC1120 datasheet, GPIO section.
   */
  CC1120_GDO0_PORT(IES) &= ~BV(CC1120_GDO0_PIN);
  //CC1120_GDO2_PORT(IES) &= ~BV(CC1120_GDO2_PIN);
  //CC1120_GDO3_PORT(IES) &= ~BV(CC1120_GDO3_PIN);

  CC1120_SPI_CSN_PORT(OUT) &= ~BV(CC1120_SPI_CSN_PIN);
  while((CC1120_SPI_MISO_PORT(IN) & BV(CC1120_SPI_MISO_PIN)) != 0);
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_enable(void)
{
  /* Reset interrupt trigger */
  CC1120_GDO0_PORT(IFG) &= ~BV(CC1120_GDO0_PIN);
  //CC1120_GDO2_PORT(IFG) &= ~BV(CC1120_GDO2_PIN);
  //CC1120_GDO3_PORT(IFG) &= ~BV(CC1120_GDO3_PIN);

  /* Enable interrupt on the GDO0 pin */
  CC1120_GDO0_PORT(IE) |= BV(CC1120_GDO0_PIN);
  //CC1120_GDO2_PORT(IE) |= BV(CC1120_GDO2_PIN);
  //CC1120_GDO3_PORT(IE) |= BV(CC1120_GDO3_PIN);
}
/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t usec)
{
  clock_delay(usec / 100);
}
/*---------------------------------------------------------------------------*/
ISR(PORT1, cc1120_port1_interrupt)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(CC1120_GDO0_PORT(IFG) & BV(CC1120_GDO0_PIN)) {
    if(cc1120_rx_interrupt()) {
      LPM4_EXIT;
    }
  }

  /* Reset interrupt trigger */
  CC1120_GDO0_PORT(IFG) &= ~BV(CC1120_GDO0_PIN);
  //CC1120_GDO2_PORT(IFG) &= ~BV(CC1120_GDO2_PIN);
  //CC1120_GDO3_PORT(IFG) &= ~BV(CC1120_GDO3_PIN);
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/

