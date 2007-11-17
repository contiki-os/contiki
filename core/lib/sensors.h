/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: sensors.h,v 1.2 2007/11/17 18:05:56 adamdunkels Exp $
 */

#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "contiki.h"

#define SENSORS_SENSOR(name, type, init, irq, activate, deactivate, \
                       active, value, configure, status)            \
const struct sensors_sensor name = { type  ,                        \
                               init, irq, activate, deactivate,     \
                               active, value, configure, status }

#define SENSORS_NUM (sizeof(sensors) / sizeof(struct sensors_sensor *))

#define SENSORS(...) \
const struct sensors_sensor *sensors[] = {__VA_ARGS__, NULL};       \
unsigned char sensors_flags[SENSORS_NUM]; \
struct process *sensors_selecting_proc[SENSORS_NUM]

struct sensors_sensor {
  char *       type;
  void         (* init)      (void);
  int          (* irq)       (void);
  void         (* activate)  (void);
  void         (* deactivate)(void);
  int          (* active)    (void);
  unsigned int (* value)     (int type);
  int          (* configure) (int type, void *parameters);
  void *       (* status)    (int type);
};

struct sensors_sensor *sensors_find(char *type);
struct sensors_sensor *sensors_next(const struct sensors_sensor *s);
struct sensors_sensor *sensors_first(void);

void sensors_changed(const struct sensors_sensor *s);


void sensors_add_irq(const struct sensors_sensor *s, unsigned char irq);
void sensors_remove_irq(const struct sensors_sensor *s, unsigned char irq);

int sensors_handle_irq(unsigned char irq_flag);

void sensors_select(const struct sensors_sensor *s, struct process *p);
void sensors_unselect(const struct sensors_sensor *s, const struct process *p);

extern process_event_t sensors_event;

PROCESS_NAME(sensors_process);

void irq_init(void);

#endif /* __SENSORS_H__ */
