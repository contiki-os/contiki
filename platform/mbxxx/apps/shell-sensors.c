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
 *         Shell function for temp and acc sensors.
 * \author
 *         Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 */

#include <string.h>
#include <stdio.h>

#include "contiki.h"
#include "shell.h"
#include "contiki-net.h"
#include "dev/temperature-sensor.h"
#include "dev/acc-sensor.h"

/*---------------------------------------------------------------------------*/
PROCESS(shell_sensors_process, "sensors");
SHELL_COMMAND(sensors_command,
	      "sensors",
	      "sensors {temp|acc}: get sensor value",
	      &shell_sensors_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_sensors_process, ev, data)
{
  
  char str_buf[22];
  
  PROCESS_BEGIN();

  if(data == NULL) {
    shell_output_str(&sensors_command,
		     "sensors {temp|acc}: a sensor must be specified", "");
    PROCESS_EXIT();
  }
  
  if(strcmp(data,"temp")==0) {
    
    unsigned int temp = temperature_sensor.value(0);
    
    snprintf(str_buf,sizeof(str_buf),"%d.%d degC",temp/10,temp-(temp/10)*10);

    shell_output_str(&sensors_command, "Temp: ", str_buf);
    
  }
  else if (strcmp(data,"acc")==0) {
    
    snprintf(str_buf,sizeof(str_buf),"%d,%d,%d) mg",acc_sensor.value(ACC_X_AXIS),acc_sensor.value(ACC_Y_AXIS),acc_sensor.value(ACC_Z_AXIS));
    
    shell_output_str(&sensors_command, "(X,Y,Z): (", str_buf);
    
  }  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_sensors_init(void)
{
  SENSORS_ACTIVATE(acc_sensor);
  
  shell_register_command(&sensors_command);
}
/*---------------------------------------------------------------------------*/
