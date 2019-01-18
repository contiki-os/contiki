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
 * @(#)$Id: service.c,v 1.1 2007/05/26 23:24:24 oliverschmidt Exp $
 */

#include <string.h>

#include "contiki.h"

/**
 * \addtogroup service
 * @{
 */

/**
 * \file
 *         Implementation of the Contiki service mechanism.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

static struct service *services_list = NULL;

/*---------------------------------------------------------------------------*/
void
service_register(struct service *s)
{
  struct service *existing;

  
  s->p = PROCESS_CURRENT();

  existing = service_find(s->name);
  if(existing != NULL) {
    service_remove(existing);
  }

  s->next = services_list;
  services_list = s;
}
/*---------------------------------------------------------------------------*/
void
service_remove(struct service *s)
{
  struct service *t;


  /* Check if service is first on the list. */
  if(s == services_list) {
    services_list = s->next;
    
    /* Post a notification to the owner process. */
    process_post(s->p, PROCESS_EVENT_SERVICE_REMOVED, s);
    
  } else {
    for(t = services_list; t != NULL && t->next != s; t = t->next);
    if(t != NULL) {
      t->next = s->next;
      
      /* Post a notification to the owner process. */
      process_post(s->p, PROCESS_EVENT_SERVICE_REMOVED, s);
    }
  }

  s->next = NULL;
}
/*---------------------------------------------------------------------------*/
struct service *
service_find(const char *name)
{
  struct service *s;  

  
  for(s = services_list; s != NULL; s = s->next) {
    if(strcmp(s->name, name) == 0) {
      return s;
    }    
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/

/** @} */
