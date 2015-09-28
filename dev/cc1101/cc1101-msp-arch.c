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

#define CC1101_GDO0_PORT(type) P1##type
#define CC1101_GDO0_PIN        7

#define CC1101_GDO2_PORT(type) P1##type
#define CC1101_GDO2_PIN        3

#define CC1101_SPI_CSN_PORT(type)  P3##type
#define CC1101_SPI_CSN_PIN     0
#define CC1101_SPI_MOSI_PORT(type)  P3##type
#define CC1101_SPI_MOSI_PIN    1
#define CC1101_SPI_MISO_PORT(type)  P3##type
#define CC1101_SPI_MISO_PIN    2
#define CC1101_SPI_SCLK_PORT(type)  P3##type
#define CC1101_SPI_SCLK_PIN    3

#include "contiki.h"
#include "contiki-net.h"

#include "cc1101-arch.h"
#include "dev/spi.h"
#include "dev/leds.h"

#include "isr_compat.h"
#include <stdio.h>

int cc1101_rx_interrupt(void);

/*---------------------------------------------------------------------------*/
void
cc1101_arch_enable(void)
{
  /* Set CSn to low (0) */
  CC1101_SPI_CSN_PORT(OUT) &= ~BV(CC1101_SPI_CSN_PIN);

  /* The MISO pin should go high before chip is fully enabled. */
  while((CC1101_SPI_MISO_PORT(IN) & BV(CC1101_SPI_MISO_PIN)) != 0);
}
/*---------------------------------------------------------------------------*/
void
cc1101_arch_disable(void)
{
  /* Set CSn to high (1) */
  CC1101_SPI_CSN_PORT(OUT) |= BV(CC1101_SPI_CSN_PIN);
}
/*---------------------------------------------------------------------------*/
static int
spi_rw_byte(unsigned char c)
{
  SPI_WAITFORTx_BEFORE();
  SPI_TXBUF = c;
  SPI_WAITFOREOTx();
  SPI_WAITFOREORx();
  c = SPI_RXBUF;

  return c;
}
/*---------------------------------------------------------------------------*/
void
cc1101_arch_init(void) 
{
  spi_init();

  /* all input by default, set these as output */
  CC1101_SPI_CSN_PORT(DIR) |= BV(CC1101_SPI_CSN_PIN);

  /* Unselect radio. */
  cc1101_arch_disable();

  CC1101_GDO0_PORT(SEL) &= ~BV(CC1101_GDO0_PIN);
  CC1101_GDO0_PORT(DIR) &= ~BV(CC1101_GDO0_PIN);

  CC1101_GDO2_PORT(SEL) &= ~BV(CC1101_GDO2_PIN);
  CC1101_GDO2_PORT(DIR) &= ~BV(CC1101_GDO2_PIN);

  /*
    The following reset procedure is recommended by Section 19.1.2 in
    the CC1101 data sheet.

    * Set SCLK = 1 and SI = 0, to avoid
    potential problems with pin control mode
    (see Section 11.3).
    * Strobe CSn low / high.
    * Hold CSn low and then high for at least 40
    μs relative to pulling CSn low
    * Pull CSn low and wait for SO to go low
    (CHIP_RDYn).
    * Issue the SRES strobe on the SI line.
    * When SO goes low again, reset is
    complete and the chip is in the IDLE state.
  */

  CC1101_SPI_SCLK_PORT(OUT) |= BV(CC1101_SPI_SCLK_PIN);
  CC1101_SPI_MOSI_PORT(OUT) |= BV(CC1101_SPI_MOSI_PIN);

  CC1101_SPI_CSN_PORT(OUT) &= ~BV(CC1101_SPI_CSN_PIN);
  CC1101_SPI_CSN_PORT(OUT) |= BV(CC1101_SPI_CSN_PIN);

  clock_delay_usec(400);

  /* Rising edge interrupt - cc1101.h writes 0x07 to IOCFG0 */
  CC1101_GDO0_PORT(IES) &= ~BV(CC1101_GDO0_PIN);
  CC1101_GDO2_PORT(IES) &= ~BV(CC1101_GDO2_PIN);

  CC1101_SPI_CSN_PORT(OUT) &= ~BV(CC1101_SPI_CSN_PIN);
  while((CC1101_SPI_MISO_PORT(IN) & BV(CC1101_SPI_MISO_PIN)) != 0);
}
/*---------------------------------------------------------------------------*/
void
cc1101_arch_interrupt_enable(void)
{
  /* Reset interrupt trigger */
  CC1101_GDO0_PORT(IFG) &= ~BV(CC1101_GDO0_PIN);
  CC1101_GDO2_PORT(IFG) &= ~BV(CC1101_GDO2_PIN);

  /* Enable interrupt on the GDO0 pin */
  CC1101_GDO0_PORT(IE) |= BV(CC1101_GDO0_PIN);
  CC1101_GDO2_PORT(IE) |= BV(CC1101_GDO2_PIN);
}
/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t usec)
{
  clock_delay(usec / 100);
}
/*---------------------------------------------------------------------------*/
ISR(PORT1, cc1101_port1_interrupt)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  //  printf("0x%2x\n", P1IV);
  watchdog_start();
 /*  leds_toggle(LEDS_ALL);*/
  if(cc1101_rx_interrupt()) {
    LPM4_EXIT;
  }
  watchdog_stop();
  /* Reset interrupt trigger */
  CC1101_GDO2_PORT(IFG) &= ~BV(CC1101_GDO2_PIN);
  CC1101_GDO0_PORT(IFG) &= ~BV(CC1101_GDO0_PIN);
  /*  leds_toggle(LEDS_ALL);*/
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
int
cc1101_arch_write_command(uint8_t c)
{
  return spi_rw_byte(c);
}
/*---------------------------------------------------------------------------*/
int
cc1101_arch_write_data(uint8_t d)
{
  return spi_rw_byte(d);
}
/*---------------------------------------------------------------------------*/
uint8_t
cc1101_arch_read_data(void)
{
  return spi_rw_byte(0);
}
/*---------------------------------------------------------------------------*/
int
cc1101_arch_write_databuf(const uint8_t *buf, int len)
{
  int i;
  for(i = 0; i < len; i++) {
    SPI_WRITE(buf[i]);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
int
cc1101_arch_read_databuf(uint8_t *buf, int len)
{
  int i;
  for(i = 0; i < len; i++) {
    SPI_READ(buf[i]);
  }
  return len;
}
/*---------------------------------------------------------------------------*/

