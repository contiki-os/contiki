/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *         Shell command that posts sensor data to Twitter
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "cfs/cfs.h"
#include "dev/sht11.h"
#include "shell.h"
#include "twitter.h"

#include <stdio.h>
#include <string.h>

#define MAX_USERNAME_PASSWORD  32

/*---------------------------------------------------------------------------*/
PROCESS(sensortweet_process, "sensortweet");
SHELL_COMMAND(sensortweet_command,
	      "sensortweet",
	      "sensortweet <username:password>: post sensor data to Twitter",
	      &sensortweet_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensortweet_process, ev, data)
{
  char message[140];
  char username_password[MAX_USERNAME_PASSWORD];
  int temp;
  uint16_t humidity;
  uint16_t battery_indicator;
  int humidity_converted;
  PROCESS_BEGIN();

  /* Open the username/password file. */
  if(data == NULL) {
    PROCESS_EXIT();
  }
  strncpy(username_password, data, MAX_USERNAME_PASSWORD);
  /*  username_password[len] = 0;*/

  temp = sht11_temp();
  humidity = sht11_humidity();
  battery_indicator = sht11_sreg() & 0x40? 1: 0;

  humidity_converted = (int)(-4L + 405L * humidity / 10000L);
  if(humidity_converted > 100) {
    humidity_converted = 100;
  }
  if(humidity_converted < 0) {
    humidity_converted = 0;
  }
  
  if(!battery_indicator) {
    snprintf(message, sizeof(message), "Contiki #sensortweet %d.%d: Temperature %d.%d C, humidity %d%%",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     (temp / 10 - 396) / 10,
	     (temp / 10 - 396) % 10,
	     humidity_converted);
  } else {
    snprintf(message, sizeof(message), "Contiki #sensortweet %d.%d: Battery low",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
  }

  twitter_post((uint8_t *)username_password, message);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_sensortweet_init(void)
{
  shell_register_command(&sensortweet_command);
}
/*---------------------------------------------------------------------------*/
