/**
 * \addtogroup shell
 * @{
 */

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
 */

/**
 * \file
 *         The shell application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"

#include "net/rime.h"

#include "shell.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#ifdef SHELL_CONF_PROMPT
extern char shell_prompt_text[];
#else
static char shell_prompt_text[] = "Contiki> ";
#endif

#ifdef SHELL_CONF_BANNER
extern char shell_banner_text[];
#else
static char shell_banner_text[] = "Contiki command shell";
#endif

LIST(commands);

int shell_event_input;

static struct process *front_process;

static unsigned long time_offset;

PROCESS(shell_process, "Shell");
PROCESS(shell_server_process, "Shell server");
/*---------------------------------------------------------------------------*/
PROCESS(help_command_process, "help");
SHELL_COMMAND(help_command, "help", "help: shows this help",
	      &help_command_process);
SHELL_COMMAND(question_command, "?", "?: shows this help",
	      &help_command_process);
PROCESS(shell_killall_process, "killall");
SHELL_COMMAND(killall_command, "killall", "killall: stop all running commands",
	      &shell_killall_process);
PROCESS(shell_kill_process, "kill");
SHELL_COMMAND(kill_command, "kill", "kill <command>: stop a specific command",
	      &shell_kill_process);
PROCESS(shell_null_process, "null");
SHELL_COMMAND(null_command, "null", "null: discard input",
	      &shell_null_process);
PROCESS(shell_exit_process, "exit");
SHELL_COMMAND(exit_command, "exit", "exit: exit shell",
	      &shell_exit_process);
SHELL_COMMAND(quit_command, "quit", "quit: exit shell",
	      &shell_exit_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_null_process, ev, data)
{
  struct shell_input *input;
  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
command_kill(struct shell_command *c)
{
  if(c != NULL) {
    shell_output_str(&killall_command, "Stopping command ", c->command);
    process_exit(c->process);
  }
}
/*---------------------------------------------------------------------------*/
static void
killall(void)
{
  struct shell_command *c;
  for(c = list_head(commands);
      c != NULL;
      c = c->next) {
    if(c != &killall_command && process_is_running(c->process)) {
      command_kill(c);
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_killall_process, ev, data)
{

  PROCESS_BEGIN();

  killall();
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_kill_process, ev, data)
{
  struct shell_command *c;
  char *name;
  PROCESS_BEGIN();

  name = data;
  if(name == NULL || strlen(name) == 0) {
    shell_output_str(&kill_command,
		     "kill <command>: command name must be given", "");
  }

  for(c = list_head(commands);
      c != NULL;
      c = c->next) {
    if(strcmp(name, c->command) == 0 &&
       c != &kill_command &&
       process_is_running(c->process)) {
      command_kill(c);
      PROCESS_EXIT();
    }
  }

  shell_output_str(&kill_command, "Command not found: ", name);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(help_command_process, ev, data)
{
  struct shell_command *c;
  PROCESS_BEGIN();

  shell_output_str(&help_command, "Available commands:", "");
  for(c = list_head(commands);
      c != NULL;
      c = c->next) {
    shell_output_str(&help_command, c->description, "");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_exit_process, ev, data)
{
  PROCESS_BEGIN();

  shell_exit();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
replace_braces(char *commandline)
{
  char *ptr;
  int level = 0;
  
  for(ptr = commandline; *ptr != 0; ++ptr) {
    if(*ptr == '{') {
      if(level == 0) {
	*ptr = ' ';
      }
      ++level;
    } else if(*ptr == '}') {
      --level;
      if(level == 0) {
	*ptr = ' ';
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static char *
find_pipe(char *commandline)
{
  char *ptr;
  int level = 0;
  
  for(ptr = commandline; *ptr != 0; ++ptr) {
    if(*ptr == '{') {
      ++level;
    } else if(*ptr == '}') {
      --level;
    } else if(*ptr == '|') {
      if(level == 0) {
	return ptr;
      }
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static struct shell_command *
start_command(char *commandline, struct shell_command *child)
{
  char *next, *args;
  int command_len;
  struct shell_command *c;

  /* Shave off any leading spaces. */
  while(*commandline == ' ') {
    commandline++;
  }

  /* Find the next command in a pipeline and start it. */
  next = find_pipe(commandline);
  if(next != NULL) {
    *next = 0;
    child = start_command(next + 1, child);
  }

  /* Separate the command arguments, and remove braces. */
  replace_braces(commandline);
  args = strchr(commandline, ' ');
  if(args != NULL) {
    args++;
  }

  /* Shave off any trailing spaces. */
  command_len = (int)strlen(commandline);
  while(command_len > 0 && commandline[command_len - 1] == ' ') {
    commandline[command_len - 1] = 0;
    command_len--;
  }
  
  if(args == NULL) {
    command_len = (int)strlen(commandline);
    args = &commandline[command_len];
  } else {
    command_len = (int)(args - commandline - 1);
  }
  

  
  /* Go through list of commands to find a match for the first word in
     the command line. */
  for(c = list_head(commands);
      c != NULL &&
	!(strncmp(c->command, commandline, command_len) == 0 &&
	  c->command[command_len] == 0);
      c = c->next);
  
  if(c == NULL) {
    shell_output_str(NULL, commandline, ": command not found (try 'help')");
    command_kill(child);
    c = NULL;
  } else if(process_is_running(c->process) || child == c) {
    shell_output_str(NULL, commandline, ": command already running");
    c->child = NULL;
    c = NULL;
  } else {
    c->child = child;
    /*    printf("shell: start_command starting '%s'\n", c->process->name);*/
    /* Start a new process for the command. */
    process_start(c->process, args);
  }
  
  return c;
}
/*---------------------------------------------------------------------------*/
int
shell_start_command(char *commandline, int commandline_len,
		    struct shell_command *child,
		    struct process **started_process)
{
  struct shell_command *c;
  int background = 0;

  if(commandline_len == 0) {
    if(started_process != NULL) {
      *started_process = NULL;
    }
    return SHELL_NOTHING;
  }

  if(commandline[commandline_len - 1] == '&') {
    commandline[commandline_len - 1] = 0;
    background = 1;
    commandline_len--;
  }

  c = start_command(commandline, child);

  /* Return a pointer to the started process, so that the caller can
     wait for the process to complete. */
  if(c != NULL && started_process != NULL) {
    *started_process = c->process;
    if(background) {
      return SHELL_BACKGROUND;
    } else {
      return SHELL_FOREGROUND;
    }
  }
  return SHELL_NOTHING;
}
/*---------------------------------------------------------------------------*/
static void
input_to_child_command(struct shell_command *c,
		       char *data1, int len1,
		       const char *data2, int len2)
{
  struct shell_input input;
  if(process_is_running(c->process)) {
    input.data1 = data1;
    input.len1 = len1;
    input.data2 = data2;
    input.len2 = len2;
    process_post_synch(c->process, shell_event_input, &input);
  }
}
/*---------------------------------------------------------------------------*/
void
shell_input(char *commandline, int commandline_len)
{
  struct shell_input input;

  /*  printf("shell_input front_process '%s'\n", front_process->name);*/

  if(commandline[0] == '~' &&
     commandline[1] == 'K') {
    /*    process_start(&shell_killall_process, commandline);*/
    if(front_process != &shell_process) {
      process_exit(front_process);
    }
  } else {
    if(process_is_running(front_process)) {
      input.data1 = commandline;
      input.len1 = commandline_len;
      input.data2 = "";
      input.len2 = 0;
      process_post_synch(front_process, shell_event_input, &input);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
shell_output_str(struct shell_command *c, char *text1, const char *text2)
{
  if(c != NULL && c->child != NULL) {
    input_to_child_command(c->child, text1, (int)strlen(text1),
			   text2, (int)strlen(text2));
  } else {
    shell_default_output(text1, (int)strlen(text1),
			 text2, (int)strlen(text2));
  }
}
/*---------------------------------------------------------------------------*/
void
shell_output(struct shell_command *c,
	     void *data1, int len1,
	     const void *data2, int len2)
{
  if(c != NULL && c->child != NULL) {
    input_to_child_command(c->child, data1, len1, data2, len2);
  } else {
    shell_default_output(data1, len1, data2, len2);
  }
}
/*---------------------------------------------------------------------------*/
void
shell_unregister_command(struct shell_command *c)
{
  list_remove(commands, c);
}
/*---------------------------------------------------------------------------*/
void
shell_register_command(struct shell_command *c)
{
  struct shell_command *i, *p;

  p = NULL;
  for(i = list_head(commands);
      i != NULL &&
	strcmp(i->command, c->command) < 0;
      i = i->next) {
    p = i;
  }
  if(p == NULL) {
    list_push(commands, c);
  } else if(i == NULL) {
    list_add(commands, c);
  } else {
    list_insert(commands, p, c);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_process, ev, data)
{
  static struct process *started_process;
  struct shell_input *input;
  int ret;
  PROCESS_BEGIN();

  /* Let the system start up before showing the prompt. */
  PROCESS_PAUSE();
  
  while(1) {
    shell_prompt(shell_prompt_text);
    
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    {
      input = data;
      ret = shell_start_command(input->data1, input->len1, NULL,
				&started_process);

      if(started_process != NULL &&
	 ret == SHELL_FOREGROUND &&
	 process_is_running(started_process)) {
	front_process = started_process;
	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXITED &&
				 data == started_process);
      }
      front_process = &shell_process;
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_server_process, ev, data)
{
  struct process *p;
  struct shell_command *c;
  static struct etimer etimer;
  PROCESS_BEGIN();

  etimer_set(&etimer, CLOCK_SECOND * 10);
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_EXITED) {
      p = data;
      /*      printf("process exited '%s' (front '%s')\n", p->name,
	      front_process->name);*/
      for(c = list_head(commands);
	  c != NULL && c->process != p;
	  c = c->next);
      while(c != NULL) {
	if(c->child != NULL && c->child->process != NULL) {
	  /*	  printf("Killing '%s'\n", c->process->name);*/
	  input_to_child_command(c->child, "", 0, "", 0);
	  /*	  process_exit(c->process);*/
	}
	c = c->child;
      }
    } else if(ev == PROCESS_EVENT_TIMER) {
      etimer_reset(&etimer);
      shell_set_time(shell_time());
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_init(void)
{
  list_init(commands);
  shell_register_command(&help_command);
  shell_register_command(&question_command);
  shell_register_command(&killall_command);
  shell_register_command(&kill_command);
  shell_register_command(&null_command);
  shell_register_command(&exit_command);
  shell_register_command(&quit_command);
  
  shell_event_input = process_alloc_event();
  
  process_start(&shell_process, NULL);
  process_start(&shell_server_process, NULL);

  front_process = &shell_process;
}
/*---------------------------------------------------------------------------*/
unsigned long
shell_strtolong(const char *str, const char **retstr)
{
  int i;
  unsigned long num = 0;
  const char *strptr = str;

  if(str == NULL) {
    return 0;
  }
  
  while(*strptr == ' ') {
    ++strptr;
  }
  
  for(i = 0; i < 10 && isdigit(strptr[i]); ++i) {
    num = num * 10 + strptr[i] - '0';
  }
  if(retstr != NULL) {
    if(i == 0) {
      *retstr = str;
    } else {
      *retstr = strptr + i;
    }
  }
  
  return num;
}
/*---------------------------------------------------------------------------*/
unsigned long
shell_time(void)
{
  return clock_seconds() + time_offset;
}
/*---------------------------------------------------------------------------*/
void
shell_set_time(unsigned long seconds)
{
  time_offset = seconds - clock_seconds();
}
/*---------------------------------------------------------------------------*/
void
shell_start(void)
{
  shell_output_str(NULL, shell_banner_text, "");
  shell_output_str(NULL, "Type '?' and return for help", "");
  shell_prompt(shell_prompt_text);
}
/*---------------------------------------------------------------------------*/
void
shell_stop(void)
{
  killall();
}
/*---------------------------------------------------------------------------*/
void
shell_quit(void)
{
  shell_stop();
  process_exit(&shell_process);
  process_exit(&shell_server_process);
}
/*---------------------------------------------------------------------------*/

/** @} */
