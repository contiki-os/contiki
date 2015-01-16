/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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
 *         Headers of multithreading module for TI CC32xx.
 * \author
 *         Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 */

#include "sys/mt.h"
#include "osi.h"

#include "mtarch.h"
#include "contiki-conf.h"

// Local defines
#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
#define MTARCH_TASK_PRIORITY 	1
#endif

#ifdef MTARCH_CONF_STACKSIZE
#define MTARCH_STACKSIZE 		MTARCH_CONF_STACKSIZE
#else
#define MTARCH_STACKSIZE 		4096
#endif

/*--------------------------------------------------------------------------*/
void
mtarch_init(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_remove(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *thread,
	     void (* function)(void *data),
	     void *data)
{
#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
	// Create task
	osi_TaskCreate(function, (const signed char * const)"ContikiMT", MTARCH_STACKSIZE, data, MTARCH_TASK_PRIORITY, &thread->task);
#else

#endif
}
/*--------------------------------------------------------------------------*/
void
mtarch_yield(void)
{
#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
	// Sleep to force context switch
	osi_Sleep(1);
#else

#endif
}
/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *thread)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_stop(struct mtarch_thread *thread)
{
#if defined(USE_FREERTOS) || defined(USE_TIRTOS)
	// Delete task
	osi_TaskDelete(&thread->task);
#else

#endif
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstart(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstop(void)
{
}
/*--------------------------------------------------------------------------*/
