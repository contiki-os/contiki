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
#include "reg.h"
#include "cpu.h"
#include "dev/sys-ctrl.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
void
sys_ctrl_init()
{
  uint32_t val;

  /*
   * Desired Clock Ctrl configuration:
   * 32KHz source: RC
   * System Clock: 32 MHz
   * Power Down Unused
   * I/O Div: 16MHz
   * Sys Div: 16MHz
   * Rest: Don't care
   */

  val = SYS_CTRL_CLOCK_CTRL_OSC32K | SYS_CTRL_CLOCK_CTRL_OSC_PD
      | SYS_CTRL_CLOCK_CTRL_IO_DIV_16MHZ | SYS_CTRL_CLOCK_CTRL_SYS_DIV_16MHZ;
  REG(SYS_CTRL_CLOCK_CTRL) = val;

  while((REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_OSC) != 0);
}
/*---------------------------------------------------------------------------*/
void
sys_ctrl_reset()
{
  REG(SYS_CTRL_PWRDBG) = SYS_CTRL_PWRDBG_FORCE_WARM_RESET;
}

/**
 * @}
 * @}
 */
