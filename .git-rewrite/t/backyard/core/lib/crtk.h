/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: crtk.h,v 1.1 2007/05/23 23:23:23 oliverschmidt Exp $
 */

/*
 * RUNES CRTK header, version 0.3
 *
 * Stefanos Zachariadis, UCL, s.zachariadis@cs.ucl.ac.uk
 */

#ifndef CRTK_H
#define CRTK_H

/* For now this file is intimately dependent on sys/service.h. Really
 * mysterious error messages are generated if one forgets to include
 * sys/service.h, for this reason it is included here!
 */
#include "sys/service.h"

#define COMPONENT 0

#define INTERFACE 1

#define RECEPTACLE 2

typedef const char *Receptacle;

typedef const void *Interface;

#if 0
typedef struct INTERFACES {
  Interface *i;
  struct INTERFACES *next;
} Interfaces;
#endif

#if 0
typedef struct RECEPTACLES {
  Receptacle *r;
  struct RECEPTACLES *next;
} Receptacles;
#endif

typedef struct ATTRIBUTE {
  char *key;
  void *value;
  struct ATTRIBUTE *next;
} Attribute;

typedef struct ENTITY { 
  int id; 
  int type; 
  char *name;
  Attribute *attributes;
  struct ENTITY *next;
} Entity; 

#define INVOKE(receptacle, iface, function)			\
  {								\
    struct service *service_s; \
    service_s = service_find(receptacle);	\
    if(service_s != NULL) {					\
      ((struct iface *)service_s->interface)->function;		\
    }								\
 }

#define DECLARE_COMPONENT(compname,interface) \
extern struct process compname##_process; \
struct compname interface;

#define DECLARE_INTERFACE(intname,interface)\
  struct intname interface;

#define IMPLEMENT_COMPONENT(compname, ...)\
  static const struct compname compname##_implementation_interface = __VA_ARGS__;\
  static struct service compname##_implementation = { NULL, NULL, #compname, & compname##_implementation_interface }; \
  static Entity compname##_entity;\
  static void construct(void);\
  static void destroy(void);\
  PROCESS(compname##_process, #compname"_process");	\
  PROCESS_THREAD(compname##_process, ev, data) {\
   static Attribute name = { "template", #compname, NULL };\
  PROCESS_EXITHANDLER(goto exit);\
  PROCESS_BEGIN();\
  compname##_entity.type = COMPONENT;\
  compname##_entity.name = #compname;\
  construct();\
  SERVICE_REGISTER(compname##_implementation);\
  INVOKE("crtk", crtk, instantiate(&compname##_entity,&name));\
  PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_SERVICE_REMOVED);\
  exit:\
  destroy();\
  SERVICE_REMOVE(compname##_implementation);\
  INVOKE("crtk", crtk, delent(&compname##_entity));\
  PROCESS_END();  \
  }

/* Registry crtk is built upon. */
DECLARE_COMPONENT(crtk,
{
  /* prints the contents of the registry. */
  void (* printregistry)(void);
  
  /* delete the given entity. */
  void (* delent)(Entity *e);

  /* adds a new attribute for the given entity. */
  void (* putattr)(Entity *e, Attribute *a);

  /* searches for an attribute with the given key, for the given entity. */
  void (* getattr)(Entity e, char *key, Attribute **a);

  /* connects an interface and a receptacle. */
  void (* connect) (Interface i, Receptacle *r);
 
  /* instantiates a new component. */
  void (* instantiate) (Entity *e, Attribute *init);

  /* returns on e all the entities (and their attributes) available. */
  void (* getentities) (Entity **e);
});

#endif /* CRTK_H */
