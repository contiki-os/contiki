/** \addtogroup apps
 * @{ */

/**
 * \defgroup servreghack A service registration and diseemination hack
 * @{
 *
 * This application is a quick'n'dirty hack for registering,
 * disseminating, and looking up services. A service is identified by
 * an 8-bit integer between 1 and 255. Integers below 128 are reserved
 * for system services.
 *
 * A service is registered with the function
 * servreg_hack_register(). Registered services will be transmitted to
 * all neighbors that run the servreg-hack application. These will in
 * turn resend the registration to their neighbors.
 *
 * Services from neighbors are stored in a local table. Services
 * stored in the table can be looked up using a combination of the
 * servreg_hack_list() and servreg_hack_item_match() functions.
 *
 */

/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: servreg-hack.h,v 1.1 2010/06/15 19:00:28 adamdunkels Exp $
 */

/**
 * \file
 *         Header file for the servreg-hack application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef SERVREG_HACK_H
#define SERVREG_HACK_H

#include "contiki-conf.h"
#include "net/uip.h"

typedef uint8_t servreg_hack_id_t;
typedef void servreg_hack_item_t;

/**
 * \brief      Initialize and start the servreg-hack application
 *
 *             This function initializes and starts the servreg-hack
 *             application and daemon.
 */
void servreg_hack_init(void);

/**
 * \brief      Register that this node provides a service
 * \param service_id The 8-bit ID for the service
 * \param addr The address associated with the service
 *
 *             This function is used to register a
 *             service. Registering a service means that other nodes
 *             in the network will become aware that this node
 *             provides this service. The servreg-hack application
 *             does not specify what this service means, nor does it
 *             provide any mechanism by which the service can be
 *             reached: this is up to the application that uses the
 *             servreg-hack application.
 */
void servreg_hack_register(servreg_hack_id_t service_id, const uip_ipaddr_t *addr);


/**
 * \brief      Get the IP address of a node offering a service
 * \param service_id The service ID of the service
 * \return     A pointer to the IP address of the node, or NULL if the service was not known
 *
 *             This function returns the address of the node offering
 *             a specific service. If the service is not known, the
 *             function returns NULL. If there are more than one nodes
 *             offering the service, this function returns the address
 *             of the node that most recently announced its service.
 *
 *             To get a list of all nodes offering a specific service,
 *             use the servreg_hack_list_head() function to get the
 *             full list of offered services.
 */
uip_ipaddr_t * servreg_hack_lookup(servreg_hack_id_t service_id);


/**
 * \brief      Obtain the list of services provided by neighbors
 * \return     The list of services
 *
 *             This function returns a list of services registered by
 *             other nodes. To find a specific service, the caller
 *             needs to iterate through the list and check each list
 *             item with the servreg_hack_item_match() function.
 */
servreg_hack_item_t *servreg_hack_list_head(void);

/**
 * \brief      Get the service ID for a list item
 * \param item The list item
 * \return     The service ID for a list item
 *
 *             This function is used when iterating through the list
 *             of registered services.
 */
servreg_hack_id_t servreg_hack_item_id(servreg_hack_item_t *item);

/**
 * \brief      Get the IP address for a list item
 * \param item The list item
 * \return     The IP address
 *
 *             This function is used when iterating through the list
 *             of registered services.
 */
uip_ipaddr_t * servreg_hack_item_address(servreg_hack_item_t *item);

#endif /* SERVREG_HACK_H */


/** @} */
/** @} */
