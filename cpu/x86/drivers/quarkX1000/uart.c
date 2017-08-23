/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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

#include "uart.h"
#include "uart-16x50.h"
#include <assert.h>

PROT_DOMAINS_ALLOC(uart_16x50_driver_t, quarkX1000_uart0);
PROT_DOMAINS_ALLOC(uart_16x50_driver_t, quarkX1000_uart1);

/* UART base frequency from section 18.2.2 of Intel Quark SoC X1000
 * Datasheet.
 */
#define QUARK_X1000_UART_FBASE 44236800

/*---------------------------------------------------------------------------*/
/**
 * \brief Perform common initialization that must precede per-port
 *        initialization.
 */
void
quarkX1000_uart_init(void)
{
  uart_16x50_init();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief     Initialize a UART.
 * \param dev Device to initialize.
 */
void
quarkX1000_uart_init_port(quarkX1000_uart_dev_t dev, unsigned baud)
{
  uint16_t dl;
  pci_config_addr_t pci_addr;
  uart_16x50_driver_t ATTR_KERN_ADDR_SPACE *drv;

  assert((dev == QUARK_X1000_UART_0) || (dev == QUARK_X1000_UART_1));

  pci_addr.raw = 0;

  /* PCI addresses from section 18.4 of Intel Quark SoC X1000 Datasheet. */
  pci_addr.dev = 20;
  pci_addr.func = (dev == QUARK_X1000_UART_0) ? 1 : 5;
  pci_addr.reg_off = PCI_CONFIG_REG_BAR0;

  if(dev == QUARK_X1000_UART_0) {
    drv = &quarkX1000_uart0;
    PROT_DOMAINS_INIT_ID(quarkX1000_uart0);
  } else {
    drv = &quarkX1000_uart1;
    PROT_DOMAINS_INIT_ID(quarkX1000_uart1);
  }
  /* Divisor setting from section 18.2.2 of Intel Quark SoC X1000 Datasheet. */
  dl = QUARK_X1000_UART_FBASE / (16 * baud);
  uart_16x50_init_port(drv, pci_addr, dl);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief     Transmit a character via a UART.
 * \param dev Device to use.
 * \param c   Character to transmit.
 */
void
quarkX1000_uart_tx(quarkX1000_uart_dev_t dev, uint8_t c)
{
  uart_16x50_driver_t drv;
  assert((dev == QUARK_X1000_UART_0) || (dev == QUARK_X1000_UART_1));
  prot_domains_copy_dcd(&drv,
                        (dev == QUARK_X1000_UART_0) ?
                          &quarkX1000_uart0 : &quarkX1000_uart1);
  uart_16x50_tx(drv, c);
}
/*---------------------------------------------------------------------------*/
