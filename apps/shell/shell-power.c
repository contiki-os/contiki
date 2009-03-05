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
 * $Id: shell-power.c,v 1.1 2009/03/05 21:12:03 adamdunkels Exp $
 */

/**
 * \file
 *         Power reporting functions of the Contiki shell
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "shell.h"
#include "sys/compower.h"
#include "sys/energest.h"

#include <stdio.h>

struct power_msg {
  uint16_t len;
  uint32_t cpu;
  uint32_t lpm;
  uint32_t transmit;
  uint32_t listen;
  uint32_t idle_transmit;
  uint32_t idle_listen;
};

#define WITH_POWERGRAPH 0

/*---------------------------------------------------------------------------*/
PROCESS(shell_power_process, "power");
SHELL_COMMAND(power_command,
	      "power",
	      "power: print power profile",
	      &shell_power_process);
PROCESS(shell_energy_process, "energy");
SHELL_COMMAND(energy_command,
	      "energy",
	      "energy: print energy profile",
	      &shell_energy_process);
PROCESS(shell_powerconv_process, "powerconv");
SHELL_COMMAND(powerconv_command,
	      "powerconv",
	      "powerconv: convert power profile to human readable output",
	      &shell_powerconv_process);
#if WITH_POWERGRAPH
PROCESS(shell_powergraph_process, "powergraph");
SHELL_COMMAND(powergraph_command,
	      "powergraph",
	      "powergraph: convert power profile to a 'graphical' repressentation",
	      &shell_powergraph_process);
#endif /* WITH_POWERGRAPH */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_power_process, ev, data)
{
  static uint32_t last_cpu, last_lpm, last_transmit, last_listen;
  static uint32_t last_idle_transmit, last_idle_listen;
  struct power_msg msg;

  PROCESS_BEGIN();

  energest_flush();
  
  msg.len = 12;
  msg.cpu = energest_type_time(ENERGEST_TYPE_CPU) - last_cpu;
  msg.lpm = energest_type_time(ENERGEST_TYPE_LPM) - last_lpm;
  msg.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - last_transmit;
  msg.listen = energest_type_time(ENERGEST_TYPE_LISTEN) - last_listen;
  msg.idle_transmit = compower_idle_activity.transmit - last_idle_transmit;
  msg.idle_listen = compower_idle_activity.listen - last_idle_listen;


  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  last_idle_listen = compower_idle_activity.listen;
  last_idle_transmit = compower_idle_activity.transmit;

  shell_output(&power_command, &msg, sizeof(msg), "", 0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_energy_process, ev, data)
{
  struct power_msg msg;

  PROCESS_BEGIN();

  energest_flush();
  
  msg.len = 12;
  msg.cpu = energest_type_time(ENERGEST_TYPE_CPU);
  msg.lpm = energest_type_time(ENERGEST_TYPE_LPM);
  msg.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  msg.listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  msg.idle_transmit = compower_idle_activity.transmit;
  msg.idle_listen = compower_idle_activity.listen;

  shell_output(&energy_command, &msg, sizeof(msg), "", 0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#define DEC2FIX(h,d) ((h * 64L) + (unsigned long)((d * 64L) / 1000L))
static void
printpower(struct power_msg *msg)
{
  char buf[100];
  unsigned long avg_power;
  unsigned long time;

  time = msg->cpu + msg->lpm;
  
  avg_power = (3L *
	       (msg->cpu       * DEC2FIX(1L,800L) +
		msg->lpm       * DEC2FIX(0L,545L) +
		msg->transmit  * DEC2FIX(17L,700L) +
		msg->listen    * DEC2FIX(20L,0))) / ((64L * time) / 1000);
  snprintf(buf, sizeof(buf), "CPU %d%% LPM %d%% tx %d%% rx %d%% idle tx %d%% idle rx %d%% tot %lu uW",
	   (int)((100L * (unsigned long)msg->cpu) / time),
	   (int)((100L * (unsigned long)msg->lpm) / time),
	   (int)((100L * (unsigned long)msg->transmit) / time),
	   (int)((100L * (unsigned long)msg->listen) / time),
	   (int)((100L * (unsigned long)msg->idle_transmit) / time),
	   (int)((100L * (unsigned long)msg->idle_listen) / time),
	   avg_power);
  shell_output_str(&powerconv_command, buf, "");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_powerconv_process, ev, data)
{
  struct power_msg *msg;
  struct shell_input *input;
  int len;

  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;
    
    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }
    len = input->len1;
    for(msg = (struct power_msg *)input->data1;
	len > 0;
	msg++, len -= sizeof(struct power_msg)) {
      printpower(msg);
    }
    len = input->len2;
    for(msg = (struct power_msg *)input->data2;
	len > 0;
	msg++, len -= sizeof(struct power_msg)) {
      printpower(msg);
    }

  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#if WITH_POWERGRAPH
#define MAX_POWERGRAPH 34
static void
printpowergraph(struct power_msg *msg)
{
  int i;
  unsigned long avg_power;
  unsigned long time;
  char buf[MAX_POWERGRAPH];

  time = msg->cpu + msg->lpm;
  
  avg_power = (3L *
	       (msg->cpu       * DEC2FIX(1L,800L) +
		msg->lpm       * DEC2FIX(0L,545L) +
		msg->transmit  * DEC2FIX(17L,700L) +
		msg->listen    * DEC2FIX(20L,0))) / ((64L * time) / 1000);
  memset(buf, 0, MAX_POWERGRAPH);
  for(i = 0; avg_power > 0 && i < MAX_POWERGRAPH; ++i) {
    buf[i] = '*';
    avg_power -= MIN(2000, avg_power);
  }
  shell_output_str(&powergraph_command, buf, "");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_powergraph_process, ev, data)
{
  struct power_msg *msg;
  struct shell_input *input;
  int len;

  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;
    
    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }
    len = input->len1;
    for(msg = (struct power_msg *)input->data1;
	len > 0;
	msg++, len -= sizeof(struct power_msg)) {
      printpowergraph(msg);
    }
    len = input->len2;
    for(msg = (struct power_msg *)input->data2;
	len > 0;
	msg++, len -= sizeof(struct power_msg)) {
      printpowergraph(msg);
    }

  }
  
  PROCESS_END();
}
#endif /* WITH_POWERGRAPH */
/*---------------------------------------------------------------------------*/
void
shell_power_init(void)
{
  shell_register_command(&power_command);
  shell_register_command(&powerconv_command);
  shell_register_command(&energy_command);

#if WITH_POWERGRAPH
  shell_register_command(&powergraph_command);
#endif /* WITH_POWERGRAPH */
}
/*---------------------------------------------------------------------------*/
