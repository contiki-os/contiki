/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "contiki-conf.h"
#include "cc26xx-uart.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "sys_ctrl.h"
#include "prcm.h"
#include "ioc.h"
#include "uart.h"
#include "lpm.h"
#include "ti-lib.h"
#include "sys/energest.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* Which events to trigger a UART interrupt */
#define CC26XX_UART_RX_INTERRUPT_TRIGGERS (UART_INT_RX | UART_INT_RT)

/* All interrupt masks */
#define CC26XX_UART_INTERRUPT_ALL (UART_INT_OE | UART_INT_BE | UART_INT_PE | \
                                   UART_INT_FE | UART_INT_RT | UART_INT_TX | \
                                   UART_INT_RX | UART_INT_CTS)
/*---------------------------------------------------------------------------*/
#define cc26xx_uart_isr UART0IntHandler
/*---------------------------------------------------------------------------*/
static int (*input_handler)(unsigned char c);
/*---------------------------------------------------------------------------*/
static void
power_domain_on(void)
{
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON);
}
/*---------------------------------------------------------------------------*/
static void
configure_baud_rate(void)
{
  /*
   * Configure the UART for 115,200, 8-N-1 operation.
   * This function uses SysCtrlClockGet() to get the system clock
   * frequency. This could be also be a variable or hard coded value
   * instead of a function call.
   */
  ti_lib_uart_config_set_exp_clk(UART0_BASE,
                                 ti_lib_sys_ctrl_peripheral_clock_get(
                                   PRCM_PERIPH_UART0,
                                   SYSCTRL_SYSBUS_ON),
                                 CC26XX_UART_CONF_BAUD_RATE,
                                 (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                  UART_CONFIG_PAR_NONE));
}
/*---------------------------------------------------------------------------*/
static void
configure_registers(void)
{
  /*
   * Map UART signals to the correct GPIO pins and configure them as
   * hardware controlled.
   */
  ti_lib_ioc_pin_type_uart(UART0_BASE, BOARD_IOID_UART_RX, BOARD_IOID_UART_TX,
                           BOARD_IOID_UART_CTS, BOARD_IOID_UART_RTS);

  configure_baud_rate();

  /*
   * Generate an RX interrupt at FIFO 1/2 full.
   * We don't really care about the TX interrupt
   */
  ti_lib_uart_fifo_level_set(UART0_BASE, UART_FIFO_TX7_8, UART_FIFO_RX4_8);

  /* Configure which interrupts to generate: FIFO level or after RX timeout */
  ti_lib_uart_int_enable(UART0_BASE, CC26XX_UART_RX_INTERRUPT_TRIGGERS);
}
/*---------------------------------------------------------------------------*/
static void
uart_on(void)
{
  power_domain_on();

  /* Configure baud rate and enable */
  if((HWREG(UART0_BASE + UART_O_CTL) & UART_CTL_UARTEN) == 0) {
    configure_registers();

    /* Enable UART */
    ti_lib_uart_enable(UART0_BASE);
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t
lpm_permit_max_pm_handler(void)
{
  return LPM_MODE_MAX_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static void
lpm_drop_handler(uint8_t mode)
{
  /* Do nothing if the PD is off */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
     != PRCM_DOMAIN_POWER_ON) {
    return;
  }

  /* Wait for outstanding TX to complete */
  while(ti_lib_uart_busy(UART0_BASE));

  /*
   * Check our clock gate under Deep Sleep. If it's off, we can shut down. If
   * it's on, this means that some other code module wants UART functionality
   * during deep sleep, so we stay enabled
   */
  if((HWREG(PRCM_BASE + PRCM_O_UARTCLKGDS) & 1) == 0) {
    ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_UART_RX);
    ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_UART_TX);

    ti_lib_uart_disable(UART0_BASE);
  }
}
/*---------------------------------------------------------------------------*/
static void
lpm_wakeup_handler(void)
{
  uart_on();
}
/*---------------------------------------------------------------------------*/
/* Declare a data structure to register with LPM. */
LPM_MODULE(uart_module, lpm_permit_max_pm_handler,
           lpm_drop_handler, lpm_wakeup_handler);
/*---------------------------------------------------------------------------*/
void
cc26xx_uart_init()
{
  /* Exit without initialising if ports are misconfigured */
  if(BOARD_IOID_UART_RX == IOID_UNUSED ||
     BOARD_IOID_UART_TX == IOID_UNUSED) {
    return;
  }

  /* Enable the serial domain and wait for domain to be on */
  power_domain_on();

  /* Enable the UART clock when running and sleeping */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_UART0);

  /* Apply clock settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Disable Interrupts */
  ti_lib_int_master_disable();

  /* Make sure the peripheral is disabled */
  ti_lib_uart_disable(UART0_BASE);

  /* Disable all UART module interrupts */
  ti_lib_uart_int_disable(UART0_BASE, CC26XX_UART_INTERRUPT_ALL);

  configure_registers();

  /* Acknowledge UART interrupts */
  ti_lib_int_enable(INT_UART0);

  /* Re-enable processor interrupts */
  ti_lib_int_master_enable();

  /* Enable UART */
  ti_lib_uart_enable(UART0_BASE);

  /* Register ourselves with the LPM module */
  lpm_register_module(&uart_module);
}
/*---------------------------------------------------------------------------*/
void
cc26xx_uart_write_byte(uint8_t c)
{
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
     != PRCM_DOMAIN_POWER_ON) {
    return;
  }

  ti_lib_uart_char_put(UART0_BASE, c);
}
/*---------------------------------------------------------------------------*/
void
cc26xx_uart_set_input(int (*input)(unsigned char c))
{
  input_handler = input;
  return;
}
/*---------------------------------------------------------------------------*/
void
cc26xx_uart_isr(void)
{
  char the_char;
  uint32_t flags;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* Read out the masked interrupt status */
  flags = ti_lib_uart_int_status(UART0_BASE, true);

  /* Clear all UART interrupt flags */
  ti_lib_uart_int_clear(UART0_BASE, CC26XX_UART_INTERRUPT_ALL);

  if((flags & CC26XX_UART_RX_INTERRUPT_TRIGGERS) != 0) {
    /*
     * If this was a FIFO RX or an RX timeout, read all bytes available in the
     * RX FIFO.
     */
    while(ti_lib_uart_chars_avail(UART0_BASE)) {
      the_char = ti_lib_uart_char_get_non_blocking(UART0_BASE);

      if(input_handler != NULL) {
        input_handler((unsigned char)the_char);
      }
    }
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
