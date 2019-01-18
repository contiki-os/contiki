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
 * @(#)$Id: service.h,v 1.1 2007/05/26 23:24:24 oliverschmidt Exp $
 */

/** \addtogroup sys
 * @{
 */

/**
 * \defgroup service The Contiki service mechanism
 *
 * The Contiki service mechanism enables cross-process functions. A
 * service that is registered by one process can be accessed by other
 * processes in the system. Services can be transparently replaced at
 * run-time.
 *
 * A service has an interface that callers use to access the service's
 * functions. This interface typically is defined in a header file
 * that is included by all users of the service. A service interface
 * is defined with the SERVICE_INTERFACE() macro.
 *
 * A service implementation is declared with the SERVICE() macro. The
 * SERVICE() statement specifies the actual functions that are used to
 * implement the service.
 *
 * Every service has a controlling process. The controlling process
 * registers the service with the system when it starts, and is also
 * notified if the service is removed or replaced. A process may
 * register any number of services.
 *
 * Service registration is done with a SERVICE_REGISTER()
 * statement. If a service with the same name is already registered,
 * this is removed before the new service is registered.
 *
 * The SERVICE_CALL() macro is used to call a service. If the service
 * to be called is not registered, the SERVICE_CALL() statement does
 * nothing. The SERVICE_FIND() function can be used to check if a
 * particular service exists before calling SERVICE_CALL().
 *
 * @{
 */

/**
 * \file
 *         Header file for the Contiki service mechanism.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "contiki.h"

struct service {
  struct service *next;
  struct process *p;
  const char *name;
  const void *interface;
};

/**
 * \name Service declaration and defition
 * @{
 */

/**
 * Define the name and interface of a service.
 *
 * This statement defines the name and interface of a service.
 *
 * \param name The name of the service.
 *
 * \param interface A list of function declarations that comprises the
 * service interface. This list must be enclosed by curly brackets and
 * consist of declarations of function pointers separated by
 * semicolons.
 *
 * \hideinitializer
 */
#define SERVICE_INTERFACE(name, interface) struct name interface;

#if ! CC_NO_VA_ARGS
/**
 * \brief      Define an implementation of a service interface.
 * \param name The name of this particular instance of the service, for use with SERVICE_REGISTER().
 * \param service_name The name of the service, from the SERVICE_INTERFACE().
 * \param ...  A structure containing the functions that implements the service.
 *
 *             This statement defines the name of this implementation
 *             of the service and defines the functions that actually
 *             implement the functions offered by the service.
 *
 * \hideinitializer
 */
#define SERVICE(name, service_name, ...)				\
  static struct service_name name##_interface = __VA_ARGS__ ;		\
  struct service name = { NULL, NULL, service_name##_name, & name##_interface }
#endif

/** @} */

/**
 * \name Calling a service
 * @{
 */

/**
 * Call a function from a specified service, if it is registered.
 *
 *
 * \param service_name The name of the service that is to be called.
 *
 * \param function The function that is to be called. This is a full
 * function call, including parameters.
 *
 * \hideinitializer
 */
#define SERVICE_CALL(service_name, function)			\
 {								\
   struct service *service_s;					\
   service_s = service_find(service_name##_name);		\
   if(service_s != NULL) {					\
     ((const struct service_name *)service_s->interface)->function; \
   }								\
 }

/* @} */

#define SERVICE_EXISTS(service_name) (service_find(service_name##_name) != NULL)

/**
 * \name Service registration and removal
 * @{
 */

/**
 * Register a service.
 *
 * \hideinitializer
 */
#define SERVICE_REGISTER(name) service_register(&name)

/**
 * Remove a service.
 *
 * \hideinitializer
 */
#define SERVICE_REMOVE(service_name) service_remove(&service_name)

/** @} */

/**
 * Find service.
 *
 * \hideinitializer
 */
#define SERVICE_FIND(service_name) service_find(service_name##_name)

CCIF void service_register(struct service *s);
CCIF void service_remove(struct service *s);
struct service *service_find(const char *name);

#endif /* __SERVICE_H__ */
/** @} */
/** @} */
