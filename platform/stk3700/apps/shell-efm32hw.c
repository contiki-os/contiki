/*
 * Copyright (c) 2013, Kerlink
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
 *         Few commands to manage Hardware on EFM32 (Gpio, adc, ...)
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */


#include "contiki.h"
#include "contiki-net.h"
#include "process.h"
#include "shell-efm32hw.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "arm_utils.h"
#include "em_emu.h"
#include "em_int.h"

#include "gpio.h"
#include "burtc.h"
#include "adc.h"

#define MAX_COMMANDLENGTH 64
#define PERIOD_INTERVAL   60

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */


/*---------------------------------------------------------------------------*/
PROCESS(shell_gpio_process, "gpio");
SHELL_COMMAND(gpio_command,
	      "gpio",
	      "gpio P<X><p> <up|down|rd|ana|pp|ds|wait|wake>: handle gpio action",
	      &shell_gpio_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_gpio_process, ev, data)
{
  const char *next, *arg;
  unsigned char action = 0;
  uint8_t portno, pin, gpio = 0;
  char output_buffer[20];
  
  PROCESS_BEGIN();

  arg = data;
  next = strchr(data, ' ');
  if(next == NULL) {
	//dump_gpio_state();
    //shell_output_str(&gpio_command, gpio_command.description, "");
    PROCESS_EXIT();
  }

  // Get GPIO
  // ex: E3, B11
  if( (arg[0] == 'P') ||  (arg[0] == 'p'))
  {
    gpio = ((arg[1] - 'A') + 0xA) << 4;
	gpio += shell_strtoul(&arg[2], NULL, 10);
  }
  else
  {
	 shell_output_str(&gpio_command, gpio_command.description, "");
	 PROCESS_EXIT();
  }

  // Now Parse command
  arg = next+1;
  next = strchr(arg, ' ');
  if(!strcmp(arg, "down")) action = 1;
  else if(!strcmp(arg, "up")) action = 2;
  else if(!strcmp(arg, "rd")) action = 3;
  else if(!strcmp(arg, "ana")) action = 4;
  else if(!strcmp(arg, "pp")) action = 5;
  else if(!strcmp(arg, "ds")) action = 6;
  else if(!strcmp(arg, "wait")) action = 7;
  else if(!strcmp(arg, "wake")) action = 8;
  arg = next+1;
  
  // Execute action
  switch(action)
  {
  	  case 1:
  		gpio_set_value(gpio, 0);
  		break;
  	  case 2:
        gpio_set_value(gpio, 1);
        break;
  	  case 3:
        gpio_set_input(gpio);
        snprintf(output_buffer,sizeof(output_buffer),"%u",gpio_get_value(gpio));
        shell_output_str(&gpio_command,output_buffer,"");
        break;
  	  case 4:
        //gpio_set_analog(gpio);
        break;
  	  case 5:
  	    gpio_set_output_pushpull(gpio, 0);
  		break;
  	  case 6:
  	    //GPIO_PinModeSet(gpioModePushPullDrive);
        //SIM3_GPIO_SET_DRIVE_STRENGTH( gpio_to_bp(gpio), BV(GPIO_PINNO(gpio)) );
        break;
  	  case 7:
/*		  // TODO : wait on irq
  		  gpio_set_input(gpio);
  		  gpio_wait(gpio);
  		  snprintf(output_buffer,sizeof(output_buffer),"Trig, IO value = %d", gpio_get_value(gpio));
  		  shell_output_str(&gpio_command,output_buffer,"");
  		  */
		  break;
  	  case 8:
  	    /*
  		  if(!strcmp(arg, "down"))
  		  {
  			 gpio_set_wakeup(gpio, 1, 0);
  		  }
  		  else
  		  {
  			 gpio_set_wakeup(gpio, 1, 1);
  		  }
*/
		  break;
  	  default:
  		 shell_output_str(&gpio_command, gpio_command.description, "");
  		 PROCESS_EXIT();
  		 break;
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS(shell_jump_process, "jump");
SHELL_COMMAND(jump_command,
          "jump",
          "jump [addr]: Jump to application pointed by [addr]",
          &shell_jump_process);


#ifndef USERAPP_START_ADDR
#define USERAPP_START_ADDR 0
#endif

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_jump_process, ev, data)
{
    struct shell_input *input;
    uint32_t startaddr = USERAPP_START_ADDR;

    PROCESS_BEGIN();

    input = data;
    if(input->len1 + input->len2 != 0)
    {

        startaddr = shell_strtolong((char *)data, NULL);
    }

    arm_jump_to_application(startaddr);

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS(shell_adc_process, "adc");
SHELL_COMMAND(adc_command,
	      "adc",
	      "adc <no>: Read adc<no> value",
	      &shell_adc_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_adc_process, ev, data)
{
  uint16_t u16_value, adcno = 0;
  char output_buffer[40];
  int32_t temp = 0;

  PROCESS_BEGIN();

  adcno = shell_strtolong((char *)data, NULL);
  if(adcno != 0)
  {
	u16_value = adc_get_value_mv(adcno);

	snprintf(output_buffer,sizeof(output_buffer),"%u mV",u16_value);
	shell_output_str(&adc_command,output_buffer,"");
  }
  else
  {
	  // Print status
	  temp = adc_get_inttemp();
	  snprintf(output_buffer,sizeof(output_buffer),"CPU: %li.%01u C",(temp/10), (uint16_t)(temp%10));
	  shell_output_str(&adc_command,output_buffer,"");
/*
	  snprintf(output_buffer,sizeof(output_buffer),"VBat: %u mV",adc_get_vbatvoltage());
	  shell_output_str(&adc_command,output_buffer,"");
	  snprintf(output_buffer,sizeof(output_buffer),"VDC: %u mV",adc_get_vdcvoltage());
	  shell_output_str(&adc_command,output_buffer,"");
	  */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS(shell_temp_process, "temp");
SHELL_COMMAND(temp_command,
	      "temp",
	      "temp: Get temperature",
	      &shell_temp_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_temp_process, ev, data)
{
  int32_t temp = 0;
  char output_buffer[40];

  PROCESS_BEGIN();

  temp = adc_get_inttemp();
  snprintf(output_buffer,sizeof(output_buffer),"CPU: %li.%01u%%C",(temp/10), (uint16_t)(temp%10));
  shell_output_str(&temp_command,output_buffer,"");

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS(shell_standby_process, "standby");
SHELL_COMMAND(standby_command,
	      "standby",
	      "standby [N]: Enter PM2 until wakeup event or for N seconds",
	      &shell_standby_process);
/*---------------------------------------------------------------------------*/

void rtc_alarm_callback(void)
{
  // Wake up
}

PROCESS_THREAD(shell_standby_process, ev, data)
{
  unsigned long sec = 0;

  PROCESS_BEGIN();

  if(data != NULL)
  {
	  sec = shell_strtolong((char *)data, NULL);
	  burtc_set_useralarm(sec, rtc_alarm_callback);
  }

  leds_off(LEDS_USER);

  printf("Entering standby mode\r\n");
  clock_delay_msec(10);

  enter_standby();

  clock_delay_msec(10);
  leds_on(LEDS_USER);
  printf("Wake up \r\n");

  PROCESS_END();
}


/*---------------------------------------------------------------------------*/

PROCESS(shell_rtc_process, "rtc");
SHELL_COMMAND(rtc_command,
	      "rtc",
	      "rtc <timeoffset>: Print/Set RTC time",
	      &shell_rtc_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rtc_process, ev, data)
{
	int32_t timeoffset = 0;
	char output_buffer[20];

	PROCESS_BEGIN();

	if(data != NULL)
	{
		timeoffset = shell_strtol((char *)data, NULL);
		if(timeoffset != 0)
		{
			burtc_set_offset(timeoffset);
		}
	}

	snprintf(output_buffer,sizeof(output_buffer),"%lu",burtc_gettime());
	shell_output_str(&rtc_command,output_buffer,"");

	PROCESS_END();
}


/*---------------------------------------------------------------------------*/

void
shell_efm32hw_init(void)
{
  shell_register_command(&gpio_command);
  shell_register_command(&jump_command);

  shell_register_command(&adc_command);
  shell_register_command(&temp_command);

  shell_register_command(&standby_command);

  shell_register_command(&rtc_command);
}
/*---------------------------------------------------------------------------*/
