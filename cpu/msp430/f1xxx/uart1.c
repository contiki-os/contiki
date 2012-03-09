/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: uart1.c,v 1.24 2011/01/19 20:44:20 joxe Exp $
 */

/*
 * Machine dependent MSP430 UART1 code.
 */

#include "contiki.h"
#include "sys/energest.h"
#include "dev/uart1.h"
#include "dev/watchdog.h"
#include "sys/ctimer.h"
#include "lib/ringbuf.h"
#include "isr_compat.h"

static int (*uart1_input_handler)(unsigned char c);
static volatile uint8_t rx_in_progress;

static volatile uint8_t transmitting;

#ifdef UART1_CONF_TX_WITH_INTERRUPT
#define TX_WITH_INTERRUPT UART1_CONF_TX_WITH_INTERRUPT
#else /* UART1_CONF_TX_WITH_INTERRUPT */
#define TX_WITH_INTERRUPT 0
#endif /* UART1_CONF_TX_WITH_INTERRUPT */

#ifdef UART1_CONF_RX_WITH_DMA
#define RX_WITH_DMA UART1_CONF_RX_WITH_DMA
#else /* UART1_CONF_RX_WITH_DMA */
#define RX_WITH_DMA 1
#endif /* UART1_CONF_RX_WITH_DMA */

#if TX_WITH_INTERRUPT
#define TXBUFSIZE 128

static struct ringbuf txbuf;
static uint8_t txbuf_data[TXBUFSIZE];
#endif /* TX_WITH_INTERRUPT */

#if RX_WITH_DMA
#define RXBUFSIZE 128

static uint8_t rxbuf[RXBUFSIZE];
static uint16_t last_size;
static struct ctimer rxdma_timer;

static void
handle_rxdma_timer(void *ptr)
{
  uint16_t size;
  size = DMA0SZ; /* Note: loop requires that size is less or eq to RXBUFSIZE */
  while(last_size != size) {
/*     printf("read: %c [%d,%d]\n", (unsigned char)rxbuf[RXBUFSIZE - last_size], */
/* 	   last_size, size); */
    uart1_input_handler((unsigned char)rxbuf[RXBUFSIZE - last_size]);
    last_size--;
    if(last_size == 0) last_size = RXBUFSIZE;
  }

  ctimer_reset(&rxdma_timer);
}
#endif /* RX_WITH_DMA */

/*---------------------------------------------------------------------------*/
uint8_t
uart1_active(void)
{
  return ((~ UTCTL1) & TXEPT) | rx_in_progress | transmitting;
}
/*---------------------------------------------------------------------------*/
void
uart1_set_input(int (*input)(unsigned char c))
{
#if RX_WITH_DMA /* This needs to be called after ctimer process is started */
  ctimer_set(&rxdma_timer, CLOCK_SECOND/64, handle_rxdma_timer, NULL);
#endif
  uart1_input_handler = input;
}
/*---------------------------------------------------------------------------*/
void
uart1_writeb(unsigned char c)
{
  watchdog_periodic();
#if TX_WITH_INTERRUPT

  /* Put the outgoing byte on the transmission buffer. If the buffer
     is full, we just keep on trying to put the byte into the buffer
     until it is possible to put it there. */
  while(ringbuf_put(&txbuf, c) == 0);

  /* If there is no transmission going, we need to start it by putting
     the first byte into the UART. */
  if(transmitting == 0) {
    transmitting = 1;

    /* Loop until the transmission buffer is available. */
    /*while((IFG2 & UTXIFG1) == 0);*/
    TXBUF1 = ringbuf_get(&txbuf);
  }

#else /* TX_WITH_INTERRUPT */

  /* Loop until the transmission buffer is available. */
  while((IFG2 & UTXIFG1) == 0);

  /* Transmit the data. */
  TXBUF1 = c;
#endif /* TX_WITH_INTERRUPT */
}
/*---------------------------------------------------------------------------*/
/**
 * Initalize the RS232 port.
 *
 */
void
uart1_init(unsigned long ubr)
{
  /* RS232 */
  P3DIR &= ~0x80;			/* Select P37 for input (UART1RX) */
  P3DIR |= 0x40;			/* Select P36 for output (UART1TX) */
  P3SEL |= 0xC0;			/* Select P36,P37 for UART1{TX,RX} */

  UCTL1 = SWRST | CHAR;                 /* 8-bit character, UART mode */

#if 0
  U1RCTL &= ~URXEIE; /* even erroneous characters trigger interrupts */
#endif

  UTCTL1 = SSEL1;                       /* UCLK = MCLK */

  UBR01 = ubr;
  UBR11 = ubr >> 8;
  /*
   * UMCTL1 values calculated using
   * http://mspgcc.sourceforge.net/baudrate.html
   */
  switch(ubr) {

#if F_CPU == 3900000ul

  case UART1_BAUD2UBR(115200ul):
    UMCTL1 = 0xF7;
    break;
  case UART1_BAUD2UBR(57600ul):
    UMCTL1 = 0xED;
    break;
  case UART1_BAUD2UBR(38400ul):
    UMCTL1 = 0xD6;
    break;
  case UART1_BAUD2UBR(19200ul):
    UMCTL1 = 0x08;
    break;
  case UART1_BAUD2UBR(9600ul):
    UMCTL1 = 0x22;
    break;

#elif F_CPU == 2457600ul

  case UART1_BAUD2UBR(115200ul):
    UMCTL1 = 0x4A;
    break;
  case UART1_BAUD2UBR(57600ul):
    UMCTL1 = 0x5B;
    break;
  default:
    /* 9600, 19200, 38400 don't require any correction */
    UMCTL1 = 0x00;

#else

#error Unsupported CPU speed in uart1.c

#endif
  }

  ME2 &= ~USPIE1;			/* USART1 SPI module disable */
  ME2 |= (UTXE1 | URXE1);               /* Enable USART1 TXD/RXD */

  UCTL1 &= ~SWRST;

  /* XXX Clear pending interrupts before enable!!! */
  IFG2 &= ~URXIFG1;
  U1TCTL |= URXSE;

  rx_in_progress = 0;

  transmitting = 0;

  IE2 |= URXIE1;                        /* Enable USART1 RX interrupt  */
#if TX_WITH_INTERRUPT
  ringbuf_init(&txbuf, txbuf_data, sizeof(txbuf_data));
  IE2 |= UTXIE1;                        /* Enable USART1 TX interrupt  */
#endif /* TX_WITH_INTERRUPT */

#if RX_WITH_DMA
  IE2 &= ~URXIE1; /* disable USART1 RX interrupt  */
  /* UART1_RX trigger */
  DMACTL0 = DMA0TSEL_9;

  /* source address = RXBUF1 */
  DMA0SA = (unsigned int) &RXBUF1;
  DMA0DA = (unsigned int) &rxbuf;
  DMA0SZ = RXBUFSIZE;
  last_size = RXBUFSIZE;
  DMA0CTL = DMADT_4 + DMASBDB + DMADSTINCR_3 + DMAEN + DMAREQ;// DMAIE;

  msp430_add_lpm_req(MSP430_REQUIRE_LPM1);
#endif /* RX_WITH_DMA */
}
/*---------------------------------------------------------------------------*/
#if !RX_WITH_DMA
ISR(UART1RX, uart1_rx_interrupt)
{
  uint8_t c;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(!(URXIFG1 & IFG2)) {
    /* Edge detect if IFG not set? */
    U1TCTL &= ~URXSE; /* Clear the URXS signal */
    U1TCTL |= URXSE;  /* Re-enable URXS - needed here?*/
    rx_in_progress = 1;
    LPM4_EXIT;
  } else {
    rx_in_progress = 0;
    /* Check status register for receive errors. */
    if(URCTL1 & RXERR) {
      c = RXBUF1;   /* Clear error flags by forcing a dummy read. */
    } else {
      c = RXBUF1;
      if(uart1_input_handler != NULL) {
	if(uart1_input_handler(c)) {
	  LPM4_EXIT;
	}
      }
    }
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
#endif /* !RX_WITH_DMA */
/*---------------------------------------------------------------------------*/
#if TX_WITH_INTERRUPT
ISR(UART1TX, uart1_tx_interrupt)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(ringbuf_elements(&txbuf) == 0) {
    transmitting = 0;
  } else {
    TXBUF1 = ringbuf_get(&txbuf);
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
#endif /* TX_WITH_INTERRUPT */
/*---------------------------------------------------------------------------*/
