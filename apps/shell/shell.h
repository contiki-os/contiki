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
 * $Id: shell.h,v 1.10 2008/11/09 12:38:02 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include "sys/process.h"

struct shell_command {
  struct shell_command *next;
  char *command;
  char *description;
  struct process *process;
  struct shell_command *child;
};

void shell_init(void);

void shell_start(void);

void shell_input(char *commandline, int commandline_len);
int shell_start_command(char *commandline, int commandline_len,
			struct shell_command *child,
			struct process **started_process);
void shell_output(struct shell_command *c,
		  void *data1, int size1,
		  const void *data2, int size2);
void shell_output_str(struct shell_command *c,
		      char *str1, const char *str2);

void shell_default_output(const char *text1, int len1,
			  const char *text2, int len2);

void shell_prompt(char *prompt);

void shell_register_command(struct shell_command *c);
void shell_unregister_command(struct shell_command *c);

unsigned long shell_strtolong(const char *str, const char **retstr);

unsigned long shell_time(void);
void shell_set_time(unsigned long seconds);

void shell_quit(void);


#define SHELL_COMMAND(name, command, description, process) \
static struct shell_command name = { NULL, command, \
                                     description, process }

enum {
  SHELL_FOREGROUND,
  SHELL_BACKGROUND,
  SHELL_NOTHING,
};

extern int shell_event_input;

struct shell_input {
  char *data1;
  const char *data2;
  int len1, len2;
};

#include "shell-blink.h"
#include "shell-coffee.h"
#include "shell-exec.h"
#include "shell-file.h"
#include "shell-httpd.h"
#include "shell-netfile.h"
#include "shell-ps.h"
#include "shell-reboot.h"
#include "shell-rime-debug.h"
#include "shell-rime-netcmd.h"
#include "shell-rime-ping.h"
#include "shell-rime-sniff.h"
#include "shell-rime.h"
#include "shell-rsh.h"
#include "shell-run.h"
#include "shell-sendtest.h"
#include "shell-sky.h"
#include "shell-text.h"
#include "shell-time.h"
#include "shell-vars.h"
#include "shell-wget.h"

#endif /* __SHELL_H__ */
