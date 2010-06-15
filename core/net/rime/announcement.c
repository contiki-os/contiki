/**
 * \addtogroup rimeannouncement
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
 * $Id: announcement.c,v 1.6 2010/06/15 19:22:25 adamdunkels Exp $
 */

/**
 * \file
 *         Implementation of the announcement primitive
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/announcement.h"
#include "lib/list.h"
#include "sys/cc.h"

LIST(announcements);

static void (* listen_callback)(int time);
static announcement_observer observer_callback;

/*---------------------------------------------------------------------------*/
void
announcement_init(void)
{
  list_init(announcements);
}
/*---------------------------------------------------------------------------*/
void
announcement_register(struct announcement *a, uint16_t id,
		      announcement_callback_t callback)
{
  a->id = id;
  a->has_value = 0;
  a->callback = callback;
  list_add(announcements, a);
  if(observer_callback) {
    observer_callback(a->id, a->has_value,
                      a->value, 0, ANNOUNCEMENT_BUMP);
  }
}
/*---------------------------------------------------------------------------*/
void
announcement_remove(struct announcement *a)
{
  list_remove(announcements, a);
}
/*---------------------------------------------------------------------------*/
void
announcement_remove_value(struct announcement *a)
{
  a->has_value = 0;
  if(observer_callback) {
    observer_callback(a->id, 0, 0, 0, ANNOUNCEMENT_NOBUMP);
  }

}
/*---------------------------------------------------------------------------*/
void
announcement_set_value(struct announcement *a, uint16_t value)
{
  uint16_t oldvalue = a->value;

  a->has_value = 1;
  a->value = value;
  if(observer_callback) {
    observer_callback(a->id, a->has_value,
                      value, oldvalue, ANNOUNCEMENT_NOBUMP);
  }
}
/*---------------------------------------------------------------------------*/
void
announcement_bump(struct announcement *a)
{
  if(observer_callback) {
    observer_callback(a->id, a->has_value,
                      a->value, a->value, ANNOUNCEMENT_BUMP);
  }
}
/*---------------------------------------------------------------------------*/
void
announcement_listen(int time)
{
  if(listen_callback) {
    listen_callback(time);
  }
}
/*---------------------------------------------------------------------------*/
void
announcement_register_listen_callback(void (*callback)(int time))
{
  listen_callback = callback;
}
/*---------------------------------------------------------------------------*/
void
announcement_register_observer_callback(announcement_observer callback)
{
  observer_callback = callback;
}
/*---------------------------------------------------------------------------*/
struct announcement *
announcement_list(void)
{
  return list_head(announcements);
}
/*---------------------------------------------------------------------------*/
void
announcement_heard(const rimeaddr_t *from, uint16_t id, uint16_t value)
{
  struct announcement *a;
  for(a = list_head(announcements); a != NULL; a = list_item_next(a)) {
    if(a->id == id) {
      if(a->callback != NULL) {
	a->callback(a, from, id, value);
      }
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
