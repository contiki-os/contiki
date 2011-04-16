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
 * $Id: shell-time.c,v 1.9 2010/10/20 15:18:49 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell-time.h"

#include "sys/clock.h"
#include "net/rime/timesynch.h"

#include "lib/random.h"

#include <stdio.h>
#include <string.h>

#define MAX_COMMANDLENGTH 64
#define PERIOD_INTERVAL   60

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */


/*---------------------------------------------------------------------------*/
PROCESS(shell_time_process, "time");
SHELL_COMMAND(time_command,
	      "time",
	      "time [seconds]: output time in binary format, or set time in seconds since 1970",
	      &shell_time_process);
PROCESS(shell_timestamp_process, "timestamp");
SHELL_COMMAND(timestamp_command,
	      "timestamp",
	      "timestamp: prepend a timestamp to data",
	      &shell_timestamp_process);
PROCESS(shell_repeat_process, "repeat");
PROCESS(shell_repeat_server_process, "repeat server");
SHELL_COMMAND(repeat_command,
	      "repeat",
	      "repeat <num> <time> <command>: run a command every <time> seconds",
	      &shell_repeat_process);
PROCESS(shell_randwait_process, "randwait");
SHELL_COMMAND(randwait_command,
	      "randwait",
	      "randwait <maxtime> <command>: wait for a random time before running a command",
	      &shell_randwait_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_time_process, ev, data)
{
  struct {
    uint16_t len;
    uint16_t clock;
    uint16_t rtimer;
    uint16_t timesynch;
    uint16_t timesynch_authority;
    uint16_t time[2];
  } msg;
  unsigned long newtime;
  const char *nextptr;
  
  PROCESS_BEGIN();

  if(data != NULL) {
    newtime = shell_strtolong(data, &nextptr);
    if(data != nextptr) {
      shell_set_time(newtime);
    }
  }
  
  msg.clock = (uint16_t)clock_time();
  msg.rtimer = (uint16_t)RTIMER_NOW();
#if TIMESYNCH_CONF_ENABLED
  msg.timesynch = timesynch_time();
  msg.timesynch_authority = timesynch_authority_level();
#else
  msg.timesynch = 0;
  msg.timesynch_authority = -1;
#endif
  msg.time[0] = (uint16_t)(shell_time() >> 16);
  msg.time[1] = (uint16_t)(shell_time());
  msg.len = 6;

  shell_output(&time_command, &msg, sizeof(msg), "", 0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_timestamp_process, ev, data)
{
  struct shell_input *input;
  struct msg {
    uint16_t len;
    uint16_t time[2];
    uint16_t timesynch;
    uint8_t data[MAX_COMMANDLENGTH];
  } msg;
  
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;
    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }
    
    msg.len = 3 + *(uint16_t *)input->data1;
    msg.time[0] = (uint16_t)(shell_time() >> 16);
    msg.time[1] = (uint16_t)(shell_time());
#if TIMESYNCH_CONF_ENABLED
    msg.timesynch = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
    msg.timesynch = 0;
#endif /* TIMESYNCH_CONF_ENABLED */
    memcpy(msg.data, input->data1 + 2,
	   input->len1 - 2 > MAX_COMMANDLENGTH?
	   MAX_COMMANDLENGTH: input->len1 - 2);
    
    shell_output(&timestamp_command, &msg, 6 + input->len1,
		 input->data2, input->len2);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_repeat_server_process, ev, data)
{
  static char *command;
  static struct process *started_process;
  char command_copy[MAX_COMMANDLENGTH];
  int ret;

  if(ev == shell_event_input) {
    goto exit;
  }
  
  PROCESS_BEGIN();

  command = data;
  
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE &&
			   data == &shell_repeat_process);
  {
    strncpy(command_copy, command, MAX_COMMANDLENGTH);
    ret = shell_start_command(command_copy, (int)strlen(command_copy),
			      &repeat_command, &started_process);
    
    if(started_process != NULL &&
       process_is_running(started_process)) {
      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXITED &&
			       data == started_process);
    }
  }

  /*  PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
  
  printf("haha \n");
  if(repeat_command.child != NULL &&
     process_is_running(repeat_command.child->process)) {
    process_post_synch(repeat_command.child->process, ev, data);
    }*/
 exit:
  ;
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
repeat_print_usage(void)
{
  shell_output_str(&repeat_command, "usage: ", repeat_command.description);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_repeat_process, ev, data)
{
  static int reps, period, period_left;
  static char command[MAX_COMMANDLENGTH];
  static struct etimer etimer;
  static int i;
  static clock_time_t start_time;
  const char *args, *next;

  if(ev == shell_event_input) {
    struct shell_input *input;
    input = data;
    /*    printf("shell repeat input %d %d\n", input->len1, input->len2);*/
    if(input->len1 + input->len2 != 0) {
      shell_output(&repeat_command, input->data1, input->len1,
		   input->data2, input->len2);
    }
  }

  PROCESS_BEGIN();

  /*  printf("data '%s'\n", data);*/

  args = data;

  if(args == NULL) {
    repeat_print_usage();
    PROCESS_EXIT();
  }

  reps = shell_strtolong(args, &next);
  if(next == args) {
    repeat_print_usage();
    PROCESS_EXIT();
  }

  args = next;
  period = shell_strtolong(args, &next);
  if(next == args) {
    repeat_print_usage();
    PROCESS_EXIT();
  }

  args = next;

  while(*args == ' ') {
    args++;
  }

  strncpy(command, args, MAX_COMMANDLENGTH);
  if(strlen(command) == 0) {
    repeat_print_usage();
    PROCESS_EXIT();
  }

  /*  printf("repeats %d period %d command '%s'\n",
      reps, period, command);*/

  start_time = clock_time();
  etimer_set(&etimer, CLOCK_SECOND * period);
  for(i = 0; reps == 0 || i < reps; ++i) {

    process_start(&shell_repeat_server_process, command);
    process_post(&shell_repeat_server_process,
		 PROCESS_EVENT_CONTINUE,
		 &shell_repeat_process);
    PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXITED &&
		       data == &shell_repeat_server_process);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    etimer_reset(&etimer);
    /*    PROCESS_PAUSE();

    for(period_left = period;
	period_left > 0;
	period_left -= MIN(PERIOD_INTERVAL, period_left)) {
      etimer_set(&etimer, CLOCK_SECOND * MIN(PERIOD_INTERVAL, period_left));
      PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
      }*/
  }
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_randwait_process, ev, data)
{
  static int maxwait;
  static char command[MAX_COMMANDLENGTH];
  static struct etimer etimer;
  static struct process *started_process;
  const char *args, *next;
  int ret;

  /*  if(ev == shell_event_input) {
    struct shell_input *input;
    input = data;
    printf("shell randwait input %d %d\n", input->len1, input->len2);
    if(input->len1 + input->len2 != 0) {
      shell_output(&randwait_command, input->data1, input->len1,
		   input->data2, input->len2);
    }
    }*/

  
  PROCESS_BEGIN();

  args = data;

  if(args == NULL) {
    shell_output_str(&randwait_command, "usage 0", "");
    PROCESS_EXIT();
  }
  
  maxwait = shell_strtolong(args, &next);
  if(next == args) {
    shell_output_str(&randwait_command, "usage 1", "");
    PROCESS_EXIT();
  }
  args = next;

  while(*args == ' ') {
    args++;
  }
  
  strncpy(command, args, MAX_COMMANDLENGTH);
  if(strlen(command) == 0) {
    shell_output_str(&repeat_command, "usage 3", "");
    PROCESS_EXIT();
  }

  /*  printf("randwait %d command '%s'\n",
      maxwait, command);*/

  etimer_set(&etimer, random_rand() % (CLOCK_SECOND * maxwait));
  PROCESS_WAIT_UNTIL(etimer_expired(&etimer));

/*   printf("Starting '%s' child %p (%s)\n", command, randwait_command.child, */
/* 	 randwait_command.child == NULL? "null": randwait_command.child->command); */
  
  ret = shell_start_command(command, (int)strlen(command),
			    randwait_command.child, &started_process);
  
  if(started_process != NULL &&
     process_is_running(started_process)) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXITED &&
			     data == started_process);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_time_init(void)
{
  shell_register_command(&time_command);
  shell_register_command(&timestamp_command);
  shell_register_command(&repeat_command);
  shell_register_command(&randwait_command);
}
/*---------------------------------------------------------------------------*/
