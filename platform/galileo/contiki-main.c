/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
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

#include <stdio.h>

#include "contiki.h"
#include "contiki-net.h"
#include "cpu.h"
#include "eth.h"
#include "eth-conf.h"
#include "galileo-pinmux.h"
#include "gpio.h"
#include "helpers.h"
#include "i2c.h"
#include "imr-conf.h"
#include "interrupt.h"
#include "irq.h"
#include "pci.h"
#include "prot-domains.h"
#include "shared-isr.h"
#include "uart.h"

PROCINIT(  &etimer_process
         , &tcpip_process
#if WITH_DNS
         , &resolv_process
#endif
         );

extern int _sdata_kern_startup_func, _edata_kern_startup_func;

/*---------------------------------------------------------------------------*/
void
app_main(void)
{
  printf("Starting Contiki\n");

  process_init();
  procinit_init();
  ctimer_init();
  eth_init();

  autostart_start(autostart_processes);

  while(1) {
    process_run();
  }

  halt();
}
/*---------------------------------------------------------------------------*/
static void
gp_fault_handler(struct interrupt_context context)
{
  fprintf(stderr, "General protection exception @%08x (ec: %u)\n",
          context.eip, context.error_code);
  halt();
}
/*---------------------------------------------------------------------------*/
/* Kernel entrypoint */
int
main(void)
{
  uintptr_t *func_ptr;

#ifdef X86_CONF_RESTRICT_DMA
  quarkX1000_imr_conf();
#endif
  irq_init();
  quarkX1000_uart_init();
  /* Initialize UART connected to Galileo Gen1 3.5mm audio-style jack or
   * Galileo Gen2 FTDI header
   */
  quarkX1000_uart_init_port(QUARK_X1000_UART_1, 115200);
  SET_EXCEPTION_HANDLER(13, 1, gp_fault_handler);
  clock_init();
  rtimer_init();

  pci_root_complex_init();
  quarkX1000_eth_init();
  quarkX1000_i2c_init();
  quarkX1000_i2c_configure(QUARKX1000_I2C_SPEED_STANDARD,
                           QUARKX1000_I2C_ADDR_MODE_7BIT);
  /* The GPIO subsystem must be initialized prior to configuring pinmux, because
   * the pinmux configuration automatically performs GPIO configuration for the
   * relevant pins.
   */
  quarkX1000_gpio_init();
  /* use default pinmux configuration */
  if(galileo_pinmux_initialize() < 0) {
    fprintf(stderr, "Failed to initialize pinmux\n");
  }
  shared_isr_init();

  /* The ability to remap interrupts is not needed after this point and should
   * thus be disabled according to the principle of least privilege.
   */
  pci_root_complex_lock();

  func_ptr = (uintptr_t *)&_sdata_kern_startup_func;
  while(func_ptr != (uintptr_t *)&_edata_kern_startup_func) {
    ((void (*)(void))*func_ptr)();

    func_ptr++;
  }

  prot_domains_leave_main();

  return 0;
}
/*---------------------------------------------------------------------------*/
