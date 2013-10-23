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
 * \addtogroup cc2538-uart
 * @{
 *
 * \file
 * Implementation of the cc2538 UART driver
 */
#include "contiki.h"
#include "sys/energest.h"
#include "dev/sys-ctrl.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/uart.h"
#include "reg.h"

#include <stdint.h>
#include <string.h>

static int (* input_handler)(unsigned char c);
/*---------------------------------------------------------------------------*/
/*
 * Once we know what UART we're on, configure correct values to be written to
 * the correct registers
 */
#if UART_BASE==UART_1_BASE
/* Running, in sleep, in deep sleep, enable the clock for the correct UART */
#define SYS_CTRL_RCGCUART_UART SYS_CTRL_RCGCUART_UART1
#define SYS_CTRL_SCGCUART_UART SYS_CTRL_SCGCUART_UART1
#define SYS_CTRL_DCGCUART_UART SYS_CTRL_DCGCUART_UART1

#define NVIC_INT_UART          NVIC_INT_UART1
#define IOC_PXX_SEL_UART_TXD   IOC_PXX_SEL_UART1_TXD
#define IOC_UARTRXD_UART       IOC_UARTRXD_UART1
#else /* Defaults for UART0 */
#define SYS_CTRL_RCGCUART_UART SYS_CTRL_RCGCUART_UART0
#define SYS_CTRL_SCGCUART_UART SYS_CTRL_SCGCUART_UART0
#define SYS_CTRL_DCGCUART_UART SYS_CTRL_DCGCUART_UART0

#define NVIC_INT_UART          NVIC_INT_UART0

#define IOC_PXX_SEL_UART_TXD   IOC_PXX_SEL_UART0_TXD
#define IOC_UARTRXD_UART       IOC_UARTRXD_UART0
#endif
/*---------------------------------------------------------------------------*/
static void
reset(void)
{
  uint32_t lchr;

  /* Make sure the UART is disabled before trying to configure it */
  REG(UART_BASE | UART_CTL) = UART_CTL_TXE | UART_CTL_RXE;

  /* Clear error status */
  REG(UART_BASE | UART_ECR) = 0xFF;

  /* Store LCHR configuration */
  lchr = REG(UART_BASE | UART_LCRH);

  /* Flush FIFOs by clearing LCHR.FEN */
  REG(UART_BASE | UART_LCRH) = 0;

  /* Restore LCHR configuration */
  REG(UART_BASE | UART_LCRH) = lchr;

  /* UART Enable */
  REG(UART_BASE | UART_CTL) |= UART_CTL_UARTEN;
}
/*---------------------------------------------------------------------------*/
void
uart_init(void)
{
  /* Enable clock for the UART while Running, in Sleep and Deep Sleep */
  REG(SYS_CTRL_RCGCUART) |= SYS_CTRL_RCGCUART_UART;
  REG(SYS_CTRL_SCGCUART) |= SYS_CTRL_SCGCUART_UART;
  REG(SYS_CTRL_DCGCUART) |= SYS_CTRL_DCGCUART_UART;

  /* Run on SYS_DIV */
  REG(UART_BASE | UART_CC) = 0;

  /*
   * Select the UARTx RX pin by writing to the IOC_UARTRXD_UARTn register
   *
   * The value to be written will be on of the IOC_INPUT_SEL_Pxn defines from
   * ioc.h. The value can also be calculated as:
   *
   * (port << 3) + pin
   */
  REG(IOC_UARTRXD_UART) = (UART_RX_PORT << 3) + UART_RX_PIN;

  /*
   * Pad Control for the TX pin:
   * - Set function to UART0 TX
   * - Output Enable
   */
  ioc_set_sel(UART_TX_PORT, UART_TX_PIN, IOC_PXX_SEL_UART_TXD);
  ioc_set_over(UART_TX_PORT, UART_TX_PIN, IOC_OVERRIDE_OE);

  /* Set RX and TX pins to peripheral mode */
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(UART_TX_PORT), GPIO_PIN_MASK(UART_TX_PIN));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(UART_RX_PORT), GPIO_PIN_MASK(UART_RX_PIN));

  /*
   * UART Interrupt Masks:
   * Acknowledge RX and RX Timeout
   * Acknowledge Framing, Overrun and Break Errors
   */
  REG(UART_BASE | UART_IM) = UART_IM_RXIM | UART_IM_RTIM;
  REG(UART_BASE | UART_IM) |= UART_IM_OEIM | UART_IM_BEIM | UART_IM_FEIM;

  REG(UART_BASE | UART_IFLS) =
    UART_IFLS_RXIFLSEL_1_8 | UART_IFLS_TXIFLSEL_1_2;

  /* Make sure the UART is disabled before trying to configure it */
  REG(UART_BASE | UART_CTL) = UART_CTL_TXE | UART_CTL_RXE;

  /* Baud Rate Generation */
  REG(UART_BASE | UART_IBRD) = UART_CONF_IBRD;
  REG(UART_BASE | UART_FBRD) = UART_CONF_FBRD;

  /* UART Control: 8N1 with FIFOs */
  REG(UART_BASE | UART_LCRH) = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

  /* UART Enable */
  REG(UART_BASE | UART_CTL) |= UART_CTL_UARTEN;

  /* Enable UART0 Interrupts */
  nvic_interrupt_enable(NVIC_INT_UART);
}
/*---------------------------------------------------------------------------*/
void
uart_set_input(int (* input)(unsigned char c))
{
  input_handler = input;
}
/*---------------------------------------------------------------------------*/
void
uart_write_byte(uint8_t b)
{
  /* Block if the TX FIFO is full */
  while(REG(UART_BASE | UART_FR) & UART_FR_TXFF);

  REG(UART_BASE | UART_DR) = b;
}
/*---------------------------------------------------------------------------*/
void
uart_isr(void)
{
  uint16_t mis;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* Store the current MIS and clear all flags early, except the RTM flag.
   * This will clear itself when we read out the entire FIFO contents */
  mis = REG(UART_BASE | UART_MIS) & 0x0000FFFF;

  REG(UART_BASE | UART_ICR) = 0x0000FFBF;

  if(mis & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
    while(!(REG(UART_BASE | UART_FR) & UART_FR_RXFE)) {
      if(input_handler != NULL) {
        input_handler((unsigned char)(REG(UART_BASE | UART_DR) & 0xFF));
      } else {
        /* To prevent an Overrun Error, we need to flush the FIFO even if we
         * don't have an input_handler. Use mis as a data trash can */
        mis = REG(UART_BASE | UART_DR);
      }
    }
  } else if(mis & (UART_MIS_OEMIS | UART_MIS_BEMIS | UART_MIS_FEMIS)) {
    /* ISR triggered due to some error condition */
    reset();
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

/** @} */
