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
 * \addtogroup cc2538-sys-ctrl
 * @{
 *
 * \file
 * Implementation of the cc2538 System Control driver
 */
#include "contiki.h"
#include "reg.h"
#include "cpu.h"
#include "dev/sys-ctrl.h"
#include "dev/gpio.h"
#include "dev/ioc.h"

#include <stdint.h>

#if SYS_CTRL_OSC32K_USE_XTAL
#define SYS_CTRL_OSCS   0
#else
#define SYS_CTRL_OSCS   SYS_CTRL_CLOCK_CTRL_OSC32K
#endif
/*---------------------------------------------------------------------------*/
int
sys_ctrl_get_reset_cause(void)
{
  return (REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_RST) >>
         SYS_CTRL_CLOCK_STA_RST_S;
}
/*---------------------------------------------------------------------------*/
const char *
sys_ctrl_get_reset_cause_str(void)
{
  static const char *reset_cause[] = {
    "POR",
    "External reset",
    "WDT",
    "CLD or software reset"
  };

  return reset_cause[sys_ctrl_get_reset_cause()];
}
/*---------------------------------------------------------------------------*/
void
sys_ctrl_init()
{
  uint32_t val;

#if SYS_CTRL_OSC32K_USE_XTAL
  /* Set the XOSC32K_Q pads to analog for crystal */
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(6));
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(6));
  ioc_set_over(GPIO_D_NUM, 6, IOC_OVERRIDE_ANA);
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(7));
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(7));
  ioc_set_over(GPIO_D_NUM, 7, IOC_OVERRIDE_ANA);
#endif

  /*
   * Desired Clock Ctrl configuration:
   * 32KHz source: RC or crystal, according to SYS_CTRL_OSC32K_USE_XTAL
   * System Clock: 32 MHz
   * Power Down Unused
   * I/O Div: according to SYS_CTRL_IO_DIV
   * Sys Div: according to SYS_CTRL_SYS_DIV
   * Rest: Don't care
   */

  val = SYS_CTRL_OSCS | SYS_CTRL_CLOCK_CTRL_OSC_PD
    | SYS_CTRL_IO_DIV | SYS_CTRL_SYS_DIV;
  REG(SYS_CTRL_CLOCK_CTRL) = val;

  while((REG(SYS_CTRL_CLOCK_STA) 
        & (SYS_CTRL_CLOCK_STA_OSC32K | SYS_CTRL_CLOCK_STA_OSC)) 
        != SYS_CTRL_OSCS);

#if SYS_CTRL_OSC32K_USE_XTAL
  /* Wait for the 32-kHz crystal oscillator to stabilize */
  while(REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_SYNC_32K);
  while(!(REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_SYNC_32K));
#endif
}
/*---------------------------------------------------------------------------*/
void
sys_ctrl_reset()
{
  REG(SYS_CTRL_PWRDBG) = SYS_CTRL_PWRDBG_FORCE_WARM_RESET;
}
/*---------------------------------------------------------------------------*/
uint32_t
sys_ctrl_get_sys_clock(void)
{
  return SYS_CTRL_32MHZ >> (REG(SYS_CTRL_CLOCK_STA) &
                            SYS_CTRL_CLOCK_STA_SYS_DIV);
}
/*---------------------------------------------------------------------------*/
uint32_t
sys_ctrl_get_io_clock(void)
{
  return SYS_CTRL_32MHZ >> ((REG(SYS_CTRL_CLOCK_STA) &
                             SYS_CTRL_CLOCK_STA_IO_DIV) >> 8);
}
/**
 * @}
 * @}
 */
