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
#include "lpm.h"
#include "reg.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef UART0_RX_PORT
#define UART0_RX_PORT            (-1)
#endif
#ifndef UART0_RX_PIN
#define UART0_RX_PIN             (-1)
#endif
#if UART0_RX_PORT >= 0 && UART0_RX_PIN < 0  || \
    UART0_RX_PORT < 0  && UART0_RX_PIN >= 0
#error Both UART0_RX_PORT and UART0_RX_PIN must be valid or invalid
#endif

#ifndef UART0_TX_PORT
#define UART0_TX_PORT            (-1)
#endif
#ifndef UART0_TX_PIN
#define UART0_TX_PIN             (-1)
#endif
#if UART0_TX_PORT >= 0 && UART0_TX_PIN < 0  || \
    UART0_TX_PORT < 0  && UART0_TX_PIN >= 0
#error Both UART0_TX_PORT and UART0_TX_PIN must be valid or invalid
#endif

#if UART0_RX_PORT >= 0 && UART0_TX_PORT < 0  || \
    UART0_RX_PORT < 0  && UART0_TX_PORT >= 0
#error Both UART0_RX and UART0_TX pads must be valid or invalid
#endif

#if UART_IN_USE(0) && UART0_RX_PORT < 0
#error Contiki is configured to use UART0, but its pads are not valid
#endif

#ifndef UART1_RX_PORT
#define UART1_RX_PORT            (-1)
#endif
#ifndef UART1_RX_PIN
#define UART1_RX_PIN             (-1)
#endif
#if UART1_RX_PORT >= 0 && UART1_RX_PIN < 0  || \
    UART1_RX_PORT < 0  && UART1_RX_PIN >= 0
#error Both UART1_RX_PORT and UART1_RX_PIN must be valid or invalid
#endif

#ifndef UART1_TX_PORT
#define UART1_TX_PORT            (-1)
#endif
#ifndef UART1_TX_PIN
#define UART1_TX_PIN             (-1)
#endif
#if UART1_TX_PORT >= 0 && UART1_TX_PIN < 0  || \
    UART1_TX_PORT < 0  && UART1_TX_PIN >= 0
#error Both UART1_TX_PORT and UART1_TX_PIN must be valid or invalid
#endif

#if UART1_RX_PORT >= 0 && UART1_TX_PORT < 0  || \
    UART1_RX_PORT < 0  && UART1_TX_PORT >= 0
#error Both UART1_RX and UART1_TX pads must be valid or invalid
#endif

#if UART_IN_USE(1) && UART1_RX_PORT < 0
#error Contiki is configured to use UART1, but its pads are not valid
#endif

#ifndef UART1_CTS_PORT
#define UART1_CTS_PORT           (-1)
#endif
#ifndef UART1_CTS_PIN
#define UART1_CTS_PIN            (-1)
#endif
#if UART1_CTS_PORT >= 0 && UART1_CTS_PIN < 0  || \
    UART1_CTS_PORT < 0  && UART1_CTS_PIN >= 0
#error Both UART1_CTS_PORT and UART1_CTS_PIN must be valid or invalid
#endif

#ifndef UART1_RTS_PORT
#define UART1_RTS_PORT           (-1)
#endif
#ifndef UART1_RTS_PIN
#define UART1_RTS_PIN            (-1)
#endif
#if UART1_RTS_PORT >= 0 && UART1_RTS_PIN < 0  || \
    UART1_RTS_PORT < 0  && UART1_RTS_PIN >= 0
#error Both UART1_RTS_PORT and UART1_RTS_PIN must be valid or invalid
#endif
/*---------------------------------------------------------------------------*/
/*
 * Baud rate defines used in uart_init() to set the values of UART_IBRD and
 * UART_FBRD in order to achieve the configured baud rates.
 */
#define UART_CLOCK_RATE       SYS_CTRL_SYS_CLOCK
#define UART_CTL_HSE_VALUE    0
#define UART_CTL_VALUE        (UART_CTL_RXE | UART_CTL_TXE | (UART_CTL_HSE_VALUE << 5))

/* DIV_ROUND() divides integers while avoiding a rounding error: */
#define DIV_ROUND(num, denom) (((num) + (denom) / 2) / (denom))

#define BAUD2BRD(baud)        DIV_ROUND(UART_CLOCK_RATE << (UART_CTL_HSE_VALUE + 2), (baud))
#define BAUD2IBRD(baud)       (BAUD2BRD(baud) >> 6)
#define BAUD2FBRD(baud)       (BAUD2BRD(baud) & 0x3f)
/*---------------------------------------------------------------------------*/
typedef struct {
  int8_t port;
  int8_t pin;
} uart_pad_t;
typedef struct {
  uint32_t sys_ctrl_rcgcuart_uart;
  uint32_t sys_ctrl_scgcuart_uart;
  uint32_t sys_ctrl_dcgcuart_uart;
  uint32_t base;
  uint32_t ioc_uartrxd_uart;
  uint32_t ioc_pxx_sel_uart_txd;
  uint32_t ibrd;
  uint32_t fbrd;
  uart_pad_t rx;
  uart_pad_t tx;
  uart_pad_t cts;
  uart_pad_t rts;
  uint8_t nvic_int;
} uart_regs_t;
/*---------------------------------------------------------------------------*/
static const uart_regs_t uart_regs[UART_INSTANCE_COUNT] = {
  {
    .sys_ctrl_rcgcuart_uart = SYS_CTRL_RCGCUART_UART0,
    .sys_ctrl_scgcuart_uart = SYS_CTRL_SCGCUART_UART0,
    .sys_ctrl_dcgcuart_uart = SYS_CTRL_DCGCUART_UART0,
    .base = UART_0_BASE,
    .ioc_uartrxd_uart = IOC_UARTRXD_UART0,
    .ioc_pxx_sel_uart_txd = IOC_PXX_SEL_UART0_TXD,
    .ibrd = BAUD2IBRD(UART0_CONF_BAUD_RATE),
    .fbrd = BAUD2FBRD(UART0_CONF_BAUD_RATE),
    .rx = {UART0_RX_PORT, UART0_RX_PIN},
    .tx = {UART0_TX_PORT, UART0_TX_PIN},
    .cts = {-1, -1},
    .rts = {-1, -1},
    .nvic_int = NVIC_INT_UART0
  }, {
    .sys_ctrl_rcgcuart_uart = SYS_CTRL_RCGCUART_UART1,
    .sys_ctrl_scgcuart_uart = SYS_CTRL_SCGCUART_UART1,
    .sys_ctrl_dcgcuart_uart = SYS_CTRL_DCGCUART_UART1,
    .base = UART_1_BASE,
    .ioc_uartrxd_uart = IOC_UARTRXD_UART1,
    .ioc_pxx_sel_uart_txd = IOC_PXX_SEL_UART1_TXD,
    .ibrd = BAUD2IBRD(UART1_CONF_BAUD_RATE),
    .fbrd = BAUD2FBRD(UART1_CONF_BAUD_RATE),
    .rx = {UART1_RX_PORT, UART1_RX_PIN},
    .tx = {UART1_TX_PORT, UART1_TX_PIN},
    .cts = {UART1_CTS_PORT, UART1_CTS_PIN},
    .rts = {UART1_RTS_PORT, UART1_RTS_PIN},
    .nvic_int = NVIC_INT_UART1
  }
};
static int (* input_handler[UART_INSTANCE_COUNT])(unsigned char c);
/*---------------------------------------------------------------------------*/
static void
reset(uint32_t uart_base)
{
  uint32_t lchr;

  /* Make sure the UART is disabled before trying to configure it */
  REG(uart_base + UART_CTL) = UART_CTL_VALUE;

  /* Clear error status */
  REG(uart_base + UART_ECR) = 0xFF;

  /* Store LCHR configuration */
  lchr = REG(uart_base + UART_LCRH);

  /* Flush FIFOs by clearing LCHR.FEN */
  REG(uart_base + UART_LCRH) = 0;

  /* Restore LCHR configuration */
  REG(uart_base + UART_LCRH) = lchr;

  /* UART Enable */
  REG(uart_base + UART_CTL) |= UART_CTL_UARTEN;
}
/*---------------------------------------------------------------------------*/
static bool
permit_pm1(void)
{
  const uart_regs_t *regs;

  for(regs = &uart_regs[0]; regs < &uart_regs[UART_INSTANCE_COUNT]; regs++) {
    if((REG(regs->base + UART_FR) & UART_FR_BUSY) != 0) {
      return false;
    }
  }

  return true;
}
/*---------------------------------------------------------------------------*/
void
uart_init(uint8_t uart)
{
  const uart_regs_t *regs;

  if(uart >= UART_INSTANCE_COUNT) {
    return;
  }
  regs = &uart_regs[uart];
  if(regs->rx.port < 0 || regs->tx.port < 0) {
    return;
  }

  lpm_register_peripheral(permit_pm1);

  /* Enable clock for the UART while Running, in Sleep and Deep Sleep */
  REG(SYS_CTRL_RCGCUART) |= regs->sys_ctrl_rcgcuart_uart;
  REG(SYS_CTRL_SCGCUART) |= regs->sys_ctrl_scgcuart_uart;
  REG(SYS_CTRL_DCGCUART) |= regs->sys_ctrl_dcgcuart_uart;

  /* Run on SYS_DIV */
  REG(regs->base + UART_CC) = 0;

  /*
   * Select the UARTx RX pin by writing to the IOC_UARTRXD_UARTn register
   *
   * The value to be written will be on of the IOC_INPUT_SEL_Pxn defines from
   * ioc.h. The value can also be calculated as:
   *
   * (port << 3) + pin
   */
  REG(regs->ioc_uartrxd_uart) = (regs->rx.port << 3) + regs->rx.pin;

  /*
   * Pad Control for the TX pin:
   * - Set function to UARTn TX
   * - Output Enable
   */
  ioc_set_sel(regs->tx.port, regs->tx.pin, regs->ioc_pxx_sel_uart_txd);
  ioc_set_over(regs->tx.port, regs->tx.pin, IOC_OVERRIDE_OE);

  /* Set RX and TX pins to peripheral mode */
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(regs->tx.port),
                          GPIO_PIN_MASK(regs->tx.pin));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(regs->rx.port),
                          GPIO_PIN_MASK(regs->rx.pin));

  /*
   * UART Interrupt Masks:
   * Acknowledge RX and RX Timeout
   * Acknowledge Framing, Overrun and Break Errors
   */
  REG(regs->base + UART_IM) = UART_IM_RXIM | UART_IM_RTIM;
  REG(regs->base + UART_IM) |= UART_IM_OEIM | UART_IM_BEIM | UART_IM_FEIM;

  REG(regs->base + UART_IFLS) =
    UART_IFLS_RXIFLSEL_1_8 | UART_IFLS_TXIFLSEL_1_2;

  /* Make sure the UART is disabled before trying to configure it */
  REG(regs->base + UART_CTL) = UART_CTL_VALUE;

  /* Baud Rate Generation */
  REG(regs->base + UART_IBRD) = regs->ibrd;
  REG(regs->base + UART_FBRD) = regs->fbrd;

  /* UART Control: 8N1 with FIFOs */
  REG(regs->base + UART_LCRH) = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

  /*
   * Enable hardware flow control (RTS/CTS) if requested.
   * Note that hardware flow control is available only on UART1.
   */
  if(regs->cts.port >= 0) {
    REG(IOC_UARTCTS_UART1) = ioc_input_sel(regs->cts.port, regs->cts.pin);
    GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(regs->cts.port), GPIO_PIN_MASK(regs->cts.pin));
    ioc_set_over(regs->cts.port, regs->cts.pin, IOC_OVERRIDE_DIS);
    REG(UART_1_BASE + UART_CTL) |= UART_CTL_CTSEN;
  }

  if(regs->rts.port >= 0) {
    ioc_set_sel(regs->rts.port, regs->rts.pin, IOC_PXX_SEL_UART1_RTS);
    GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(regs->rts.port), GPIO_PIN_MASK(regs->rts.pin));
    ioc_set_over(regs->rts.port, regs->rts.pin, IOC_OVERRIDE_OE);
    REG(UART_1_BASE + UART_CTL) |= UART_CTL_RTSEN;
  }

  /* UART Enable */
  REG(regs->base + UART_CTL) |= UART_CTL_UARTEN;

  /* Enable UART0 Interrupts */
  nvic_interrupt_enable(regs->nvic_int);
}
/*---------------------------------------------------------------------------*/
void
uart_set_input(uint8_t uart, int (* input)(unsigned char c))
{
  if(uart >= UART_INSTANCE_COUNT) {
    return;
  }

  input_handler[uart] = input;
}
/*---------------------------------------------------------------------------*/
void
uart_write_byte(uint8_t uart, uint8_t b)
{
  uint32_t uart_base;

  if(uart >= UART_INSTANCE_COUNT) {
    return;
  }
  uart_base = uart_regs[uart].base;

  /* Block if the TX FIFO is full */
  while(REG(uart_base + UART_FR) & UART_FR_TXFF);

  REG(uart_base + UART_DR) = b;
}
/*---------------------------------------------------------------------------*/
static void
uart_isr(uint8_t uart)
{
  uint32_t uart_base;
  uint16_t mis;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  uart_base = uart_regs[uart].base;

  /* Store the current MIS and clear all flags early, except the RTM flag.
   * This will clear itself when we read out the entire FIFO contents */
  mis = REG(uart_base + UART_MIS) & 0x0000FFFF;

  REG(uart_base + UART_ICR) = 0x0000FFBF;

  if(mis & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
    while(!(REG(uart_base + UART_FR) & UART_FR_RXFE)) {
      if(input_handler[uart] != NULL) {
        input_handler[uart]((unsigned char)(REG(uart_base + UART_DR) & 0xFF));
      } else {
        /* To prevent an Overrun Error, we need to flush the FIFO even if we
         * don't have an input_handler. Use mis as a data trash can */
        mis = REG(uart_base + UART_DR);
      }
    }
  } else if(mis & (UART_MIS_OEMIS | UART_MIS_BEMIS | UART_MIS_FEMIS)) {
    /* ISR triggered due to some error condition */
    reset(uart_base);
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
#define UART_ISR(u)  void uart##u##_isr(void) { uart_isr(u); }
UART_ISR(0)
UART_ISR(1)

/** @} */
