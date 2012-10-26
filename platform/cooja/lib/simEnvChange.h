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

#ifndef __SIMENVCHANGE_H__
#define __SIMENVCHANGE_H__

#include "contiki.h"

/* Simulation interface structure */
struct simInterface {
  void         (* doActionsBeforeTick) (void);
  void         (* doActionsAfterTick)  (void);
};

// Variable for keeping the last process_run() return value
extern int simProcessRunValue;
extern int simEtimerPending;
extern clock_time_t simNextExpirationTime;
extern clock_time_t simCurrentTime;

// Variable that when set to != 0, stops the mote from falling asleep next tick
extern char simDontFallAsleep;

// Definition for registering an interface
#define SIM_INTERFACE(name, doActionsBeforeTick, doActionsAfterTick) \
const struct simInterface name = { doActionsBeforeTick, doActionsAfterTick }

// Definition for getting access to simulation interface
#define SIM_INTERFACE_NAME(name) \
extern const struct simInterface name

// Definition for creating all interface (from main file)
#define SIM_INTERFACES(...) \
const struct simInterface *simInterfaces[] = {__VA_ARGS__, NULL};

// Functions which polls all interfaces
void doActionsBeforeTick();
void doActionsAfterTick();

#endif /* __SIMENVCHANGE_H__ */
