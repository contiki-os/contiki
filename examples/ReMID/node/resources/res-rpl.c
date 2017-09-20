/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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



/////////////////////////////////////////////////////////IMPORTS//////////////////////////////////////////////////////////////////////////
#include "er-coap-engine.h"
#include "net/rpl/rpl.h"
#include "res.h"
#include "net/ip/uip-debug.h"

//////////////////////////////////////////////////////////CONSTANTS/////////////////////////////////////////////////////////////////////////

#define DAG_RESOURCE "rpl/dag"
#define DAG_PARENT_LABEL "parent"
#define DAG_CHILD_LABEL "child"

//////////////////////////////////////////////////////////////////FUNCTION PROTOTYPES//////////////////////////////////////////////////////
static void res_get_dag_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_dag_event_handler(void);

///////////////////////////////////////////////////////////////////RESOURCE DEFINITION//////////////////////////////////////////////////////
EVENT_RESOURCE(resource_rpl_dag, "obs;title=\"RPL DAG Parent and Children\"",
               res_get_dag_handler,       /* GET handler */
               NULL,                /* POST handler */
               NULL,                /* PUT handler */
               NULL,                /* DELETE handler */
               res_dag_event_handler);      /* event handler */


static void res_get_dag_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t strpos = 0;            /* position in overall string (which is larger than the buffer) */
  size_t bufpos = 0;            /* position within buffer (bytes written) */
  
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  /* make sure the request accepts JSON reply or does not specify the reply type */
  if(accept == -1 || accept == REST.type.APPLICATION_JSON) {
    res_reply_char_if_possible('{', buffer, &bufpos, preferred_size, &strpos, offset);
    uip_ds6_defrt_t *default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
    if(default_route)
    {
      res_reply_char_if_possible('"', buffer, &bufpos, preferred_size, &strpos, offset);
      res_reply_string_if_possible(DAG_PARENT_LABEL, buffer, &bufpos, preferred_size, &strpos, offset);
      res_reply_string_if_possible("\":[\"", buffer, &bufpos, preferred_size, &strpos, offset);
      uip_ipaddr_t p;
      uip_ipaddr_copy(&p,&default_route->ipaddr);
      p.u16[0] = rpl_get_any_dag()->prefix_info.prefix.u16[0];
      res_reply_ip_if_possible(&p, buffer, &bufpos, preferred_size, &strpos, offset);
      res_reply_string_if_possible("\"]", buffer, &bufpos, preferred_size, &strpos, offset);


    } else {
      res_reply_char_if_possible('"', buffer, &bufpos, preferred_size, &strpos, offset);
      res_reply_string_if_possible(DAG_PARENT_LABEL, buffer, &bufpos, preferred_size, &strpos, offset);
      res_reply_string_if_possible("\":[]", buffer, &bufpos, preferred_size, &strpos, offset);
    }

    res_reply_string_if_possible(",\"\"\"", buffer, &bufpos, preferred_size, &strpos, offset);
    res_reply_string_if_possible(DAG_CHILD_LABEL, buffer, &bufpos, preferred_size, &strpos, offset);
    res_reply_string_if_possible("\":[", buffer, &bufpos, preferred_size, &strpos, offset);
    
    uip_ds6_route_t *route;
    for(route = uip_ds6_route_head(); route; route = uip_ds6_route_next(route))
    {
      if(route != uip_ds6_route_head())
      {
        res_reply_char_if_possible(',', buffer, &bufpos, preferred_size, &strpos, offset);
      }
      res_reply_char_if_possible('"', buffer, &bufpos, preferred_size, &strpos, offset);
      res_reply_ip_if_possible(&route->ipaddr, buffer, &bufpos, preferred_size, &strpos, offset);
      res_reply_char_if_possible('"', buffer, &bufpos, preferred_size, &strpos, offset);
      if(bufpos > preferred_size && strpos - bufpos > *offset) {
        break;
      }
    }


    res_reply_string_if_possible("]}", buffer, &bufpos, preferred_size, &strpos, offset);

    if(bufpos > 0) {
      /* Build the header of the reply */
      REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
      /* Build the payload of the reply */
      REST.set_response_payload(response, buffer, bufpos);
    } else if(strpos > 0) {
      coap_set_status_code(response, BAD_OPTION_4_02);
      coap_set_payload(response, "BlockOutOfScope", 15);
    }
    if(strpos <= *offset + preferred_size) {
      *offset = -1;
    } else {
      *offset += preferred_size;
    }
  } else { /* if the client accepts a response payload format other than json, return 406 */
    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
    return;
  }
}


/* Notify all clients who observe changes to rpl/dag resource i.e. to the RPL dodag connections */
static void
res_dag_event_handler()
{
  /* Registered observers are notified and will trigger the GET handler to create the response. */
  REST.notify_subscribers(&resource_rpl_dag);
}



