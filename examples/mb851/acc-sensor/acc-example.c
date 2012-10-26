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
 */

/**
 * \file
 *         Example showing use of the accelerometer.
 * \author
 *         Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 *
 */

#include "contiki.h"

#include "dev/acc-sensor.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(acc_process, "Accelerometer process");
AUTOSTART_PROCESSES(&acc_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(acc_process, ev, data)
{
  static struct etimer etimer;
  
  PROCESS_BEGIN();
  
  printf("Starting measuring acceleration\r\n");
  
  SENSORS_ACTIVATE(acc_sensor);
  
  // Enable High Range.
  //acc_sensor.configure(ACC_RANGE, ACC_HIGH_RANGE);
  
  // Enable High Pass Filter.
  //acc_sensor.configure(ACC_HPF, ACC_1HZ);

  
  while(1) {
    etimer_set(&etimer, CLOCK_SECOND/2);
    
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    
    printf("(X,Y,Z): (%d,%d,%d) mg      \r",acc_sensor.value(ACC_X_AXIS),acc_sensor.value(ACC_Y_AXIS),acc_sensor.value(ACC_Z_AXIS));
    
  }
  
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

