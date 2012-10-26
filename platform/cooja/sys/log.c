/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "sys/log.h"
#include "lib/simEnvChange.h"

#define IMPLEMENT_PRINTF 1

#if WITH_UIP
/* uIP packets via SLIP */
#include "uip.h"
#define MAX_LOG_LENGTH (2*UIP_BUFSIZE)
#else /* WITH_UIP */
#define MAX_LOG_LENGTH 1024
#endif /* WITH_UIP */

#if MAX_LOG_LENGTH < 1024
#undef MAX_LOG_LENGTH
#define MAX_LOG_LENGTH 1024
#endif /* MAX_LOG_LENGTH < 1024 */


const struct simInterface simlog_interface;

/* Variables shared between COOJA and Contiki */
char simLoggedData[MAX_LOG_LENGTH];
int simLoggedLength;
char simLoggedFlag;

/*-----------------------------------------------------------------------------------*/
void
simlog_char(char c)
{
  if (simLoggedLength + 1 > MAX_LOG_LENGTH) {
    /* Dropping message due to buffer overflow */
    return;
  }

  simLoggedData[simLoggedLength] = c;
  simLoggedLength += 1;
  simLoggedFlag = 1;
}
/*-----------------------------------------------------------------------------------*/
void
simlog(const char *message)
{
  if (simLoggedLength + strlen(message) > MAX_LOG_LENGTH) {
    /* Dropping message due to buffer overflow */
    return;
  }

  memcpy(simLoggedData + simLoggedLength, message, strlen(message));
  simLoggedLength += strlen(message);
  simLoggedFlag = 1;
}
/*-----------------------------------------------------------------------------------*/
void
log_message(const char *part1, const char *part2)
{
  simlog(part1);
  simlog(part2);
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*-----------------------------------------------------------------------------------*/
#if IMPLEMENT_PRINTF
int
putchar(int c)
{
  simlog_char(c);
  return c;
}
/*-----------------------------------------------------------------------------------*/
int
puts(const char* s)
{
  simlog(s);
  simlog_char('\n');
  return 0;
}
/*-----------------------------------------------------------------------------------*/
int
printf(const char *fmt, ...)
{
  int res;
  static char buf[MAX_LOG_LENGTH];
  va_list ap;
  va_start(ap, fmt);
  res = vsnprintf(buf, MAX_LOG_LENGTH, fmt, ap);
  va_end(ap);

  simlog(buf);
  return res;
}
#endif /* IMPLEMENT_PRINTF */
/*-----------------------------------------------------------------------------------*/

SIM_INTERFACE(simlog_interface,
          doInterfaceActionsBeforeTick,
          doInterfaceActionsAfterTick);
