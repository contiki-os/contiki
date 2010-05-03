/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: jcreate-shell.c,v 1.6 2010/05/03 22:02:59 nifi Exp $
 */

/**
 * \file
 *         Jcreate-specific Contiki shell
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"

#include "dev/leds.h"
#include "dev/acc-sensor.h"

#include <stdio.h>
#include <string.h>

#include <io.h>
#include <signal.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_poke_process, "poke");
SHELL_COMMAND(poke_command,
	      "poke",
	      "poke <address> <byte>: write byte <byte> to address <address>",
	      &shell_poke_process);
PROCESS(shell_peek_process, "peek");
SHELL_COMMAND(peek_command,
	      "peek",
	      "peek <address>: read a byte from address <address>",
	      &shell_peek_process);
PROCESS(shell_acc_process, "acc");
SHELL_COMMAND(acc_command,
	      "acc",
	      "acc: print out accelerometer data",
	      &shell_acc_process);
PROCESS(shell_leds_process, "leds");
SHELL_COMMAND(leds_command,
	      "leds",
	      "leds: set LEDs",
	      &shell_leds_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_poke_process, ev, data)
{
  uint8_t *address;
  uint8_t byte;
  const char *args, *next;

  PROCESS_BEGIN();

  args = data;

  if(args == NULL) {
    shell_output_str(&poke_command, "usage 0", "");
    PROCESS_EXIT();
  }
  
  address = (uint8_t *)(int)shell_strtolong(args, &next);
  if(next == args) {
    shell_output_str(&poke_command, "usage 1", "");
    PROCESS_EXIT();
  }

  args = next;
  byte = shell_strtolong(args, &next);
  if(next == args) {
    shell_output_str(&poke_command, "usage 2", "");
    PROCESS_EXIT();
  }

  printf("address %p byte 0x%02x\n", address, byte);
  
  *address = byte;
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_peek_process, ev, data)
{
  uint8_t *address;
  const char *args, *next;
  char buf[32];

  PROCESS_BEGIN();

  args = data;

  if(args == NULL) {
    shell_output_str(&peek_command, "usage 0", "");
    PROCESS_EXIT();
  }
  
  address = (uint8_t *)(int)shell_strtolong(args, &next);
  if(next == args) {
    shell_output_str(&peek_command, "usage 1", "");
    PROCESS_EXIT();
  }

  snprintf(buf, sizeof(buf), "0x%02x", *address);

  shell_output_str(&peek_command, buf, "");
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
struct acc_msg {
  uint16_t len;
  uint16_t acc[4];
};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_acc_process, ev, data)
{
  struct acc_msg msg;
  int i;
  
  PROCESS_BEGIN();

  msg.len = 1;
  for(i = 0; i < 4; ++i) {
    msg.acc[i] = acc_sensor.value(i);
  }

  shell_output(&acc_command, &msg, sizeof(msg), "", 0);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_leds_process, ev, data)
{
  struct acc_msg *msg;
  struct shell_input *input;
  int val, i;
  static int num;
  const char *args, *next;
  
  PROCESS_BEGIN();
  
  args = data;
  if(args == NULL) {
    shell_output_str(&acc_command, "usage 0", "");
    PROCESS_EXIT();
  }
  
  num = shell_strtolong(args, &next);
  if(next == args) {
    shell_output_str(&acc_command, "usage 1", "");
    PROCESS_EXIT();
  }

  PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
  input = data;
  msg = (struct acc_msg *)input->data1;
  val = 0;
  for(i = 0; i < msg->acc[num] >> 9; ++i) {
    val = (val << 1) | 1;
  }
  leds_on(val & 0xff);
  leds_off(~(val & 0xff));
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS(sky_shell_process, "Sky Contiki shell");
AUTOSTART_PROCESSES(&sky_shell_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sky_shell_process, ev, data)
{
  PROCESS_BEGIN();

  serial_shell_init();
  shell_blink_init();
  shell_file_init();
  shell_coffee_init();
  shell_ps_init();
  shell_reboot_init();
  shell_rime_init();
  /*  shell_rime_debug_init();*/
  shell_rime_netcmd_init();
/*   shell_rime_ping_init(); */
  shell_rime_neighbors_init();
  shell_rime_sendcmd_init();
  /*  shell_rime_sniff_init();*/
  shell_rime_unicast_init();
  /*shell_sky_init();*/
  shell_text_init();
  shell_time_init();

  SENSORS_ACTIVATE(acc_sensor);
  shell_register_command(&acc_command);
  shell_register_command(&poke_command);
  shell_register_command(&peek_command);
  shell_register_command(&leds_command);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
