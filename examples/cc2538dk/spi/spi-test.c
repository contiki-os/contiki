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
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-timers cc2538dk Timer Test Project
 *
 *    This example tests the correct functionality of clocks and timers.
 *
 *    More specifically, it tests clock_seconds, rtimers, etimers and
 *    clock_delay_usec.
 *
 *    This is largely-based on the same example of the cc2530 port.
 * @{
 *
 * \file
 *         Tests related to clocks and timers
 */
#include "contiki.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "spi-arch.h"
#include "dev/spi.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define SIZE 3
/*---------------------------------------------------------------------------*/
static struct etimer et;

static uint16_t i;
static int8_t tx_data[SIZE];
static int8_t rx_data[SIZE];
/*---------------------------------------------------------------------------*/
PROCESS(spi_test_process, "SPI test process");
AUTOSTART_PROCESSES(&spi_test_process);

#if SSI_ISR_ENABLE
#if SSI_MODE_SLAVE
/*---------------------------------------------------------------------------*/
int
ssi_input(void)
{

  if(i < SIZE) {
    rx_data[i] = SPI_RXBUF;
    /*
     * You don't drive the transmission,
     *  just set new data to buffer*/
    SPI_TXBUF = tx_data[i];
    i++;
    printf("received %u", i);
  } else {
    return -1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
ssi_reset(void)
{

  i = 0;
  printf("reseting counter");
  return 0;
}
/*---------------------------------------------------------------------------*/
#else
/*---------------------------------------------------------------------------*/
int
ssi_input(void)
{
  /*Do something
   * TBD*/
  return 0;
}
/*---------------------------------------------------------------------------*/
int
ssi_reset(void)
{
  /*Do something
   * TBD*/
  return 0;
}
/*---------------------------------------------------------------------------*/
#endif
#endif

PROCESS_THREAD(spi_test_process, ev, data)
{

  PROCESS_BEGIN();

  etimer_set(&et, 2 * CLOCK_SECOND);

#if SSI_ISR_ENABLE
  ssi_set_input(ssi_input);
  ssi_set_reset(ssi_reset);
#endif

  memset(tx_data, 0xff, sizeof(tx_data));
  memset(rx_data, 0, sizeof(rx_data));

  spi_init();

  PROCESS_YIELD();

  printf("-----------------------------------------\n");
  printf("SPI will send data every %u\n",
         CLOCK_SECOND);

  while(1) {
    i = 0;
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

#if SSI_MODE_SLAVE
    /*
     * Do something*/
#if SSI_ISR_ENABLE
#else
    SPI_TXBUF = tx_data[i];
    printf("received %lu!\n", SPI_RXBUF);
#endif
#else
    while(i < SIZE) {
      SPI_WRITE(tx_data[i]);
      SPI_READ(rx_data[i]);
      i++;
    }
#endif

    printf("heart beat!\n");
    leds_toggle(LEDS_RED);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
