/*
 * Copyright (c) 2011, Institute for Pervasive Computing, ETH Zurich
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
 */

/**
 * \file
 *      An abstraction layer for RESTful Web services
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "contiki.h"
#include <string.h> /*for string operations in match_addresses*/
#include <stdio.h> /*for sprintf in rest_set_header_**/

#include "erbium.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

PROCESS_NAME(rest_manager_process);

LIST(restful_services);
LIST(restful_periodic_services);


void
rest_init_engine(void)
{
  list_init(restful_services);

  REST.set_service_callback(rest_invoke_restful_service);

  /* Start the RESTful server implementation. */
  REST.init();

  /*Start rest manager process*/
  process_start(&rest_manager_process, NULL);
}

void
rest_activate_resource(resource_t* resource)
{
  PRINTF("Activating: %s", resource->url);

  if (!resource->pre_handler)
  {
    rest_set_pre_handler(resource, REST.default_pre_handler);
  }
  if (!resource->post_handler)
  {
    rest_set_post_handler(resource, REST.default_post_handler);
  }

  list_add(restful_services, resource);
}

void
rest_activate_periodic_resource(periodic_resource_t* periodic_resource)
{
  list_add(restful_periodic_services, periodic_resource);
  rest_activate_resource(periodic_resource->resource);

  rest_set_post_handler(periodic_resource->resource, REST.subscription_handler);
}

void
rest_activate_event_resource(resource_t* resource)
{
  rest_activate_resource(resource);
  rest_set_post_handler(resource, REST.subscription_handler);
}

list_t
rest_get_resources(void)
{
  return restful_services;
}


void*
rest_get_user_data(resource_t* resource)
{
  return resource->user_data;
}

void
rest_set_user_data(resource_t* resource, void* user_data)
{
  resource->user_data = user_data;
}

void
rest_set_pre_handler(resource_t* resource, restful_pre_handler pre_handler)
{
  resource->pre_handler = pre_handler;
}

void
rest_set_post_handler(resource_t* resource, restful_post_handler post_handler)
{
  resource->post_handler = post_handler;
}

void
rest_set_special_flags(resource_t* resource, rest_resource_flags_t flags)
{
  resource->flags |= flags;
}

int
rest_invoke_restful_service(void* request, void* response, uint8_t *buffer, uint16_t buffer_size, int32_t *offset)
{
  uint8_t found = 0;
  uint8_t allowed = 0;

  PRINTF("rest_invoke_restful_service url /%.*s -->\n", url_len, url);

  resource_t* resource = NULL;
  const char *url = NULL;

  for (resource = (resource_t*)list_head(restful_services); resource; resource = resource->next)
  {
    /*if the web service handles that kind of requests and urls matches*/
    if ((REST.get_url(request, &url)==strlen(resource->url) || (REST.get_url(request, &url)>strlen(resource->url) && (resource->flags & HAS_SUB_RESOURCES)))
        && strncmp(resource->url, url, strlen(resource->url)) == 0)
    {
      found = 1;
      rest_resource_flags_t method = REST.get_method_type(request);

      PRINTF("method %u, resource->flags %u\n", (uint16_t)method, resource->flags);

      if (resource->flags & method)
      {
        allowed = 1;

        /*call pre handler if it exists*/
        if (!resource->pre_handler || resource->pre_handler(resource, request, response))
        {
          /* call handler function*/
          resource->handler(request, response, buffer, buffer_size, offset);

          /*call post handler if it exists*/
          if (resource->post_handler)
          {
            resource->post_handler(resource, request, response);
          }
        }
      } else {
        REST.set_response_status(response, REST.status.METHOD_NOT_ALLOWED);
      }
      break;
    }
  }

  if (!found) {
    REST.set_response_status(response, REST.status.NOT_FOUND);
  }

  return found & allowed;
}
/*-----------------------------------------------------------------------------------*/

PROCESS(rest_manager_process, "Rest Process");

PROCESS_THREAD(rest_manager_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  /* Initialize the PERIODIC_RESOURCE timers, which will be handled by this process. */
  periodic_resource_t* periodic_resource = NULL;
  for (periodic_resource = (periodic_resource_t*) list_head(restful_periodic_services); periodic_resource; periodic_resource = periodic_resource->next) {
    if (periodic_resource->period) {
      PRINTF("Periodic: Set timer for %s to %lu\n", periodic_resource->resource->url, periodic_resource->period);
      etimer_set(&periodic_resource->periodic_timer, periodic_resource->period);
    }
  }

  while (1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      for (periodic_resource = (periodic_resource_t*)list_head(restful_periodic_services);periodic_resource;periodic_resource = periodic_resource->next) {
        if (periodic_resource->period && etimer_expired(&periodic_resource->periodic_timer)) {

          PRINTF("Periodic: etimer expired for /%s (period: %lu)\n", periodic_resource->resource->url, periodic_resource->period);

          /* Call the periodic_handler function if it exists. */
          if (periodic_resource->periodic_handler) {
            (periodic_resource->periodic_handler)(periodic_resource->resource);
          }
          etimer_reset(&periodic_resource->periodic_timer);
        }
      }
    }
  }

  PROCESS_END();
}

