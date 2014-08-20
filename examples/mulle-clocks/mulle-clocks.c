/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "K60.h"

#include <stdio.h> /* For printf() */

void
myidle(void)
{
  while(1);
}
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Hello, world\n");

  printf("Enable RTC CLKOUT on PTE26\n");

  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
  PORTE_PCR26 = PORT_PCR_MUX(6);
#if K60_CPU_REV == 2
  SIM_SOPT2 |= SIM_SOPT2_RTCCLKOUTSEL_MASK; /* 32kHz on RTCCLKOUT */
#endif

  /* Enable CLKOUT, FlexBus Clock */
  printf("Enable CLKOUT on PTC3\n");
#if K60_CPU_REV == 2
  SIM_SOPT2 &= ~(SIM_SOPT2_CLKOUTSEL(0x07)); /* Select clock on CLKOUT on Rev2.x silicon */
  SIM_SOPT2 |= SIM_SOPT2_CLKOUTSEL(0b010); /* Flash clock output on CLKOUT */
#endif
  SIM_SCGC7 |= SIM_SCGC7_FLEXBUS_MASK; /* Enable FlexBus clock gate */
  SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;  /* Enable PortC clock gate */
  PORTC_PCR3 = PORT_PCR_MUX(5);       /* Set pin for CLKOUT */

  myidle();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
