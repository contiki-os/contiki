/*
* Copyright (c) 2014, CETIC.
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
*/

/**
* \file
*         Simple CoAP Library
* \author
*         6LBR Team <6lbr@cetic.be>
*/

#include "contiki.h"
#include <string.h>
#include "coap-binding.h"
#include "coap-common.h"
#define DEBUG 0
#include "net/ip/uip-debug.h"

LIST(full_resources);

PROCESS(coap_binding_process, "Coap Binding");

/*---------------------------------------------------------------------------*/
void
coap_binding_serialize(coap_binding_t const *binding, nvm_binding_data_t *store)
{
	memcpy(&store->dest_addr, &binding->dest_addr.u8, 16);
	store->dest_port = binding->dest_port;
	strcpy(store->uri, binding->uri);
	strcpy(store->resource, binding->resource->url);
	store->flags = binding->cond.flags | COAP_BINDING_FLAGS_NVM_BINDING_VALID;
	store->pmin = binding->cond.pmin;
	store->pmax = binding->cond.pmax;
	store->step = binding->cond.step;
	store->less_than = binding->cond.less_than;
	store->greater_than = binding->cond.greater_than;
}
/*---------------------------------------------------------------------------*/
int
coap_binding_deserialize(nvm_binding_data_t const *store, coap_binding_t *binding)
{
	if ((store->flags & COAP_BINDING_FLAGS_NVM_BINDING_VALID) == 0) {
		return 0;
	}
	memcpy(&binding->dest_addr.u8, &store->dest_addr, 16);
	binding->dest_port = store->dest_port;
	strcpy(binding->uri, store->uri);
	binding->resource = rest_find_resource_by_url(store->resource);
	if (binding->resource == NULL) {
		PRINTF("Resource %s not found\n", store->resource);
		return 0;
	}
	binding->cond.flags = store->flags;
	binding->cond.pmin = store->pmin;
	binding->cond.pmax = store->pmax;
	binding->cond.step = store->step;
	binding->cond.less_than = store->less_than;
	binding->cond.greater_than = store->greater_than;
	return 1;
}
/*---------------------------------------------------------------------------*/
int
coap_binding_parse_filter_tag(char *p, coap_binding_cond_t *binding_cond, char *data, char *max, int resource_type)
{
  int flag_status = 1;
  uint32_t flag_value;

  if (strcmp(p, "pmin") == 0) {
    flag_status = coap_strtoul(data, max, &flag_value);
    if (flag_status != 0 && flag_value > 0) {
      binding_cond->pmin = flag_value;
      binding_cond->flags |= COAP_BINDING_FLAGS_PMIN_VALID;
      PRINTF("Pmin set to %lu\n", flag_value);
    } else {
      PRINTF("Pmin is invalid");
      coap_error_message = "Pmin is invalid";
      return 0;
    }
  } else if (strcmp(p, "pmax") == 0) {
    flag_status = coap_strtoul(data, max, &flag_value);
    if (flag_status != 0 && flag_value > 0) {
      binding_cond->pmax = flag_value;
      binding_cond->flags |= COAP_BINDING_FLAGS_PMAX_VALID;
      PRINTF("Pmax set to %lu\n", flag_value);
    } else {
      PRINTF("Pmax is invalid");
      coap_error_message = "Pmax is invalid";
      return 0;
    }
  } else if (strcmp(p, "st") == 0) {
    flag_status = coap_parse_binding_value(resource_type, data, max, &flag_value);
    if (flag_status != 0) {
      binding_cond->step = flag_value;
      binding_cond->flags |= COAP_BINDING_FLAGS_ST_VALID;
      PRINTF("Change Step set to %.*s\n", max - data, data);
    } else {
      PRINTF("Change Step is invalid");
      coap_error_message = "Change Step is invalid";
      return 0;
    }
  } else if (strcmp(p, "lt") == 0) {
    flag_status = coap_parse_binding_value(resource_type, data, max, &flag_value);
    if (flag_status != 0) {
      binding_cond->less_than = flag_value;
      binding_cond->flags |= COAP_BINDING_FLAGS_LT_VALID;
      PRINTF("Less Than set to %.*s\n", max - data, data);
    } else {
      PRINTF("Less Than is invalid");
      coap_error_message = "Less Than is invalid";
      return 0;
    }
  } else if (strcmp(p, "gt") == 0) {
    flag_status = coap_parse_binding_value(resource_type, data, max, &flag_value);
    if (flag_status != 0) {
      binding_cond->greater_than = flag_value;
      binding_cond->flags |= COAP_BINDING_FLAGS_GT_VALID;
      PRINTF("Greater Than set to %.*s\n", max - data, data);
    } else {
      PRINTF("Greater Than is invalid");
      coap_error_message = "Greater Than is invalid";
      return 0;
    }
  } else {
    PRINTF("Invalid filter detected");
    coap_error_message = "Invalid filter detected";
    return 0;
  }

  return flag_status;
}
/*---------------------------------------------------------------------------*/
int
coap_binding_parse_filters(char *buffer, size_t len, coap_binding_cond_t *target_binding_cond, int resource_type)
{
  int status = 0;
  char *p = buffer;
  char *max = buffer + len;
  char *sep;
  char *data;
  char *data_max;
  int filters = 1;
  coap_binding_cond_t binding_cond;

  PRINTF("Parsing %.*s\n", len, buffer);
  memset((void*)&binding_cond, 0, sizeof(coap_binding_cond_t));
  while (p != max && filters) {
    if(*p == '&') ++p;
    if(p == max) break;
    sep = p;
    while (*sep != '=' && sep != max) ++sep;
    if (*sep != '=') break;
    *sep++ = '\0';
    if(sep == max) break;
    if (*sep == '"') {
      sep++;
      if(sep == max) break;
      data = sep;
      while (*sep != '"' && sep != max) ++sep;
      if (*sep != '"') break;
      data_max = sep;
      *sep++ = '\0';
    } else {
      data = sep;
      while (*sep != '&' && sep != max) ++sep;
      data_max = sep;
    }
    filters = coap_binding_parse_filter_tag(p, &binding_cond, data, data_max, resource_type);
    p = sep;
  }
  status = p == max && filters;

  if (p != max) {
    coap_error_message = "Parsing failed";
    PRINTF("Parsing failed at %d ('%.*s')\n", p - buffer, max - p, p);
  } else if (!filters) {
    PRINTF("Filter invalid\n");
  } else {
    *target_binding_cond = binding_cond;
  }

  return status;
}
/*---------------------------------------------------------------------------*/
int
coap_binding_trigger_cond(coap_binding_cond_t * binding_cond, coap_resource_data_t *resource_data)
{
  unsigned long current_time = clock_seconds();
  if(binding_cond->flags == 0) {
    return 0;
  }
  PRINTF("Trigger : last + pmax = %lu, clock = %lu, delta = %ld\n", resource_data->last_sent_time + binding_cond->pmax, current_time, resource_data->last_sent_time + binding_cond->pmax - current_time);
  if ((binding_cond->flags & COAP_BINDING_FLAGS_PMAX_VALID) != 0 &&
      resource_data->last_sent_time + binding_cond->pmax <= current_time) {
    resource_data->last_sent_value = resource_data->last_value;
    PRINTF("Cond triggered, pmax reached");
    return 1;
  }
  if((binding_cond->flags & COAP_BINDING_FLAGS_PMIN_VALID) != 0 &&
      resource_data->last_sent_time + binding_cond->pmin > current_time) {
    PRINTF("Cond not triggered, pmin unreached\n");
    return 0;
  }
  if((binding_cond->flags & COAP_BINDING_FLAGS_ST_VALID) != 0 &&
      (resource_data->last_sent_value + binding_cond->step > resource_data->last_value &&
          resource_data->last_sent_value - binding_cond->step < resource_data->last_value)) {
    PRINTF("Cond not triggered, step unreached\n");
    return 0;
  }
  if((binding_cond->flags & COAP_BINDING_FLAGS_LT_VALID) != 0) {
    if(resource_data->last_value >= binding_cond->less_than) {
      PRINTF("Cond not triggered, value not less than threshold\n");
      return 0;
    } else if (resource_data->last_sent_value < binding_cond->less_than) {
      PRINTF("Cond not triggered, value already crossed threshold\n");
      return 0;
    }
  }
  if((binding_cond->flags & COAP_BINDING_FLAGS_GT_VALID) != 0) {
    if(resource_data->last_value <= binding_cond->greater_than) {
      PRINTF("Cond not triggered, value not greater than threshold\n");
      return 0;
    } else if (resource_data->last_sent_value > binding_cond->greater_than) {
      PRINTF("Cond not triggered, value already crossed threshold\n");
      return 0;
    }
  }
  PRINTF("Push triggered, value : %lu\n", resource_data->last_value);
  resource_data->last_sent_value = resource_data->last_value;
  return 1;
}
/*---------------------------------------------------------------------------*/
void
coap_binding_add_resource(coap_full_resource_t *resource)
{
  list_add(full_resources, resource);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_binding_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  while(1) {
    coap_full_resource_t *resource = NULL;
    for(resource = (coap_full_resource_t *)list_head(full_resources);
              resource; resource = resource->next) {
      PRINTF("Scanning resource %s\n", resource->coap_resource->url);
      resource->update_value(&resource->data);
      if(coap_binding_trigger_cond(&resource->trigger, &resource->data)) {
        resource->data.last_sent_time = clock_seconds();
        resource->coap_resource->trigger();
      }
    }
    etimer_set(&et, CLOCK_SECOND);
    while(!etimer_expired(&et)) {
      PROCESS_YIELD();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
coap_binding_init(void)
{
  list_init(full_resources);

  process_start(&coap_binding_process, NULL);
}
/*---------------------------------------------------------------------------*/
