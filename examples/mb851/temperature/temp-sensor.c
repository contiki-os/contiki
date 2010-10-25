/*
 * Copyright (c) 2010, STMicroelectronics.
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
 * $Id: temp-sensor.c,v 1.1 2010/10/25 13:34:29 salvopitru Exp $
 */

/**
 * \file
 *         A very simple Contiki application showing how to use the temperature
 *			sensor.
 * \author
 *         Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 *
 */

#include "contiki.h"

#include "dev/temperature-sensor.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(temp_process, "Temperature process");
AUTOSTART_PROCESSES(&temp_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(temp_process, ev, data)
{
  static struct etimer etimer;
  
  PROCESS_BEGIN();
  
  printf("Starting measuring temperature\r\n");  
  
  while(1) {
    etimer_set(&etimer, CLOCK_SECOND);
    
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    
    unsigned int temp = temperature_sensor.value(0);
    printf("Temp: %d.%d °C  \r",temp/10,temp-(temp/10)*10);
    
  }
  
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

