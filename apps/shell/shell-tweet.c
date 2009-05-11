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
 * $Id: shell-tweet.c,v 1.1 2009/05/11 17:10:55 adamdunkels Exp $
 */

/**
 * \file
 *         Post Twitter message through the Contiki shell
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "cfs/cfs.h"
#include "shell.h"
#include "twitter.h"

#include <stdio.h>
#include <string.h>

#define MAX_USERNAME_PASSWORD  32

/*---------------------------------------------------------------------------*/
PROCESS(tweet_process, "tweet");
SHELL_COMMAND(tweet_command,
	      "tweet",
	      "tweet <username:password> <message>: post message to Twitter",
	      &tweet_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tweet_process, ev, data)
{
  const char *message;
  char username_password[MAX_USERNAME_PASSWORD];
  const char *next;
  
  PROCESS_BEGIN();

  if(data == NULL) {
    PROCESS_EXIT();
  }
  next = strchr(data, ' ');
  /* Make sure there is a message to post. */
  if(next == data) {
    PROCESS_EXIT();
  }
  message = next + 1;
  
  strncpy(username_password, data, next - (char *)data);
  username_password[next - (char *)data] = 0;

  twitter_post((uint8_t *)username_password, message);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_tweet_init(void)
{
  shell_register_command(&tweet_command);
}
/*---------------------------------------------------------------------------*/
