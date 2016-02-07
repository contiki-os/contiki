/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

/* Include the default configuration file early here so that this file can
 * unconfigure the IP buffer size.  That will allow uipopt.h to define a
 * default IP buffer size that is larger and more useful.
 */
#include "contiki-default-conf.h"

#undef UIP_CONF_BUFFER_SIZE

#include <inttypes.h>

#define CLOCK_CONF_SECOND 128
typedef unsigned long clock_time_t;

typedef uint64_t rtimer_clock_t;
#define RTIMER_ARCH_SECOND 1024
#define RTIMER_CLOCK_LT(a, b)     ((int64_t)((a) - (b)) < 0)

/* We define the following macros and types otherwise Contiki does not
 * compile.
 */
#define CCIF
#define CLIF

#define UIP_CONF_LLH_LEN 14

#define LINKADDR_CONF_SIZE 6

typedef unsigned short uip_stats_t;

#endif /* CONTIKI_CONF_H */
