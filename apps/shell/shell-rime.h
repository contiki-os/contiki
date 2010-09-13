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
 * $Id: shell-rime.h,v 1.7 2010/09/13 13:29:30 adamdunkels Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SHELL_RIME_H__
#define __SHELL_RIME_H__

#include "shell.h"

void shell_rime_init(void);

/* Rime channel used by the 'unicast' debug command, which uses 1 channel */
#define SHELL_RIME_CHANNEL_UNICAST   20

/* Rime channel used by the 'runicast' debug command, which uses 1 channel */
#define SHELL_RIME_CHANNEL_RUNICAST  SHELL_RIME_CHANNEL_UNICAST + 1

/* Rime channel used by the 'broadcast' debug command, which uses 1 channel */
#define SHELL_RIME_CHANNEL_BROADCAST SHELL_RIME_CHANNEL_RUNICAST + 1

/* Rime channel used by the 'nodes' command, which uses 1 channel */
#define SHELL_RIME_CHANNEL_NODES     SHELL_RIME_CHANNEL_BROADCAST + 1

/* Rime channel used by the 'collect' command, which uses 2 channels */
#define SHELL_RIME_CHANNEL_COLLECT   SHELL_RIME_CHANNEL_NODES + 1

/* Rime channel used by the 'ping' command, which uses 3 channels */
#define SHELL_RIME_CHANNEL_PING      SHELL_RIME_CHANNEL_COLLECT + 2

/* Rime channel used by the 'netcmd' command, which uses 1 channels */
#define SHELL_RIME_CHANNEL_NETCMD    SHELL_RIME_CHANNEL_PING + 3

/* Rime channel used by the 'sendcmd' command, which uses 1 channels */
#define SHELL_RIME_CHANNEL_SENDCMD   SHELL_RIME_CHANNEL_NETCMD + 1

/* Rime channel used by the 'netfile' command, which uses 2 channels */
#define SHELL_RIME_CHANNEL_NETFILE   SHELL_RIME_CHANNEL_SENDCMD + 1

/* Rime channel used by the 'sendtest' command, which uses 1 channels */
#define SHELL_RIME_CHANNEL_SENDTEST  SHELL_RIME_CHANNEL_NETFILE + 2

/* Rime channel used by the 'download' command, which uses 2 channels */
#define SHELL_RIME_CHANNEL_DOWNLOAD  SHELL_RIME_CHANNEL_SENDTEST + 1

/* Rime channel used by the 'netperf' command, which uses 6 channels */
#define SHELL_RIME_CHANNEL_NETPERF   SHELL_RIME_CHANNEL_DOWNLOAD + 2


/* Announcement idenfied used by the 'neighbors' command, uses one idenfier */
#define SHELL_RIME_ANNOUNCEMENT_IDENTIFIER_NEIGHBORS SHELL_RIME_CHANNEL_DOWNLOAD + 2

extern struct collect_conn shell_collect_conn;

#endif /* __SHELL_RIME_H__ */
