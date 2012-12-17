/*
 * Contiki PIC32 Port project
 * 
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup pic32-contiki-port PIC32 Contiki Port
 *
 * @{
 */

/** 
 * \file   mtarch.h
 * \brief  PIC32MX initialization routines
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \author Daniele Alessandrelli <d.alessandrelli@sssup.it>
 * \date   2012-03-23
 */

/*
 * PIC32MX795F512L - Specific Functions
 * 
 * All the functions in this part of the file are specific for the
 * pic32mx795f512l that is characterized by registers' name that differ from
 * the 3xx and 4xx families of the pic32mx.
 */

#include <pic32_irq.h>

#include <p32xxxx.h>
#include <peripheral/system.h> 
#include <stdint.h>

#include <dev/leds.h>

/* General Excepiton Handler: overrides the default handler */
static enum {
  EXCEP_IRQ = 0,                /* interrupt */
  EXCEP_AdEL = 4,               /* address error exception (load or ifetch) */
  EXCEP_AdES,                   /* address error exception (store) */
  EXCEP_IBE,                    /* bus error (ifetch) */
  EXCEP_DBE,                    /* bus error (load/store) */
  EXCEP_Sys,                    /* syscall */
  EXCEP_Bp,                     /* breakpoint */
  EXCEP_RI,                     /* reserved instruction */
  EXCEP_CpU,                    /* coprocessor unusable */
  EXCEP_Overflow,               /* arithmetic overflow */
  EXCEP_Trap,                   /* trap (possible divide by zero) */
  EXCEP_IS1 = 16,               /* implementation specfic 1 */
  EXCEP_CEU,                    /* CorExtend unuseable */
  EXCEP_C2E                     /* coprocessor 2 */
} cp0_exception_code;

static unsigned int cp0_exception_cause; /* CP0: CAUSE register */
static unsigned int cp0_exception_epc;   /* CP0: Exception Program Counter */

/*---------------------------------------------------------------------------*/
void
pic32_init(void)
{
  unsigned long int r;

  ASM_DIS_INT;

  /* Disable JTAG Port */
  DDPCONbits.JTAGEN = 0;

  // set the CP0 cause IV bit high
  asm volatile("mfc0 %0,$13" : "=r"(r));
  r |= 0x00800000;
  asm volatile("mtc0 %0,$13" : "+r"(r));

  INTCONSET = _INTCON_MVEC_MASK;

  SYSKEY = 0;
  SYSKEY = 0xaa996655;
  SYSKEY = 0x556699aa;
  
  /* Enable Sleep Mode */
  OSCCONCLR = 1 << _OSCCON_SLPEN_POSITION;
  
  SYSKEY = 0;

  ASM_EN_INT;
}


/*---------------------------------------------------------------------------*/
void
_general_exception_handler(void)
{
  asm volatile ("mfc0 %0,$14":"=r" (cp0_exception_epc));

  asm volatile ("mfc0 %0,$13":"=r" (cp0_exception_cause));

  cp0_exception_code = (cp0_exception_cause >> 2) & 0x0000001F;
  
  leds_on(LEDS_ALL);

  while(1){
    ;
  }
}
/*---------------------------------------------------------------------------*/

/** @} */
