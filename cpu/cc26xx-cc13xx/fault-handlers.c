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
/*---------------------------------------------------------------------------*/
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_cpu_scs.h"
/*---------------------------------------------------------------------------*/
#define fault_handlers_hard_fault_isr FaultISR
/*---------------------------------------------------------------------------*/
void
fault_handlers_hard_fault_isr(void)
{
  /*
   * Workaround for (Im)precise Bus Faults caused under unknown circumstances,
   * likely by access to RFC registers while the RF PD is off (which is
   * something that should never happen because we do in fact check before
   * accessing)
   */
  if((HWREG(CPU_SCS_BASE + CPU_SCS_O_CFSR) == CPU_SCS_CFSR_IMPRECISERR) ||
     (HWREG(CPU_SCS_BASE + CPU_SCS_O_CFSR) & CPU_SCS_CFSR_PRECISERR)){
    /* ToDo: Check BFARVALID and then BFAR to filter down even further */
    return;
  }

  while(1);
}
/*---------------------------------------------------------------------------*/
