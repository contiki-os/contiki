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
#ifndef COAP_COMMON_H
#define COAP_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#include "er-coap.h"
#include "coap-binding.h"

#include "coap-data-format.h"

// Global variable for SenML basetime factorization
extern unsigned long coap_batch_basetime;
extern int core_itf_linked_batch_resource;

/*---------------------------------------------------------------------------*/

#ifndef COAP_CONF_DATA_FORMAT
#define COAP_DATA_FORMAT coap_data_format_text
#else
#define COAP_DATA_FORMAT COAP_CONF_DATA_FORMAT
#endif

extern coap_data_format_t COAP_DATA_FORMAT;

/*---------------------------------------------------------------------------*/

#ifdef REST_CONF_DEFAULT_PERIOD
#define REST_DEFAULT_PERIOD REST_CONF_DEFAULT_PERIOD
#else
#define REST_DEFAULT_PERIOD 10
#endif

/*---------------------------------------------------------------------------*/

/* double expansion */
#define TO_STRING2(x)  # x
#define TO_STRING(x)  TO_STRING2(x)

#define CT_EVAL(x) TO_STRING(CT_CONCAT(x))
#define CT_CONCAT(x) x ## _REST_TYPE
/*---------------------------------------------------------------------------*/

/* Inclusion of rest types available */
#include "rest-type-text.h"
#include "rest-type-xml.h"
#include "rest-type-json.h"
#include "rest-type-senml.h"

/*---------------------------------------------------------------------------*/

#define COAP_RESOURCE_TYPE_BOOLEAN 0
#define COAP_RESOURCE_TYPE_SIGNED_INT 1
#define COAP_RESOURCE_TYPE_UNSIGNED_INT 2
#define COAP_RESOURCE_TYPE_DECIMAL_ONE 3
#define COAP_RESOURCE_TYPE_DECIMAL_TWO 4
#define COAP_RESOURCE_TYPE_DECIMAL_THREE 5
#define COAP_RESOURCE_TYPE_STRING 6
#define COAP_RESOURCE_TYPE_OPAQUE 7

#define COAP_RESOURCE_TYPE_0_TYPE uint32_t
#define COAP_RESOURCE_TYPE_1_TYPE int32_t
#define COAP_RESOURCE_TYPE_2_TYPE uint32_t
#define COAP_RESOURCE_TYPE_3_TYPE uint32_t
#define COAP_RESOURCE_TYPE_4_TYPE uint32_t
#define COAP_RESOURCE_TYPE_5_TYPE uint32_t
#define COAP_RESOURCE_TYPE_6_TYPE char *
#define COAP_RESOURCE_TYPE_7_TYPE void *

/*---------------------------------------------------------------------------*/

int
coap_strtoul(char const *data, char const *max, uint32_t *value);

int
coap_strtofix(char const *data, char const *max, uint32_t *value, int precision);

int
coap_format_binding_value(int resource_type, char *buffer, int size, void *data);

int
coap_parse_binding_value(int resource_type, char const *buffer, char const * max, void *data);

/*---------------------------------------------------------------------------*/

#define REST_PARSE_EMPTY(payload, len, actuator_set) {\
  if(len==0) {\
    success = actuator_set(); \
  } else {\
    printf("len: %d\n", len); \
  } \
}

#define REST_PARSE_ONE_INT(payload, len, actuator_set) { \
  char * endstr; \
  int value = strtol((char const *)payload, &endstr, 10); \
  if ( ! *endstr ) { \
    success = actuator_set(value); \
  } \
}

#define REST_PARSE_ONE_UINT(buffer, max, data) { \
  uint32_t value; \
  if (coap_strtoul((char const *)buffer, max, &value)) { \
    data = value; \
    success = 1; \
  } \
}

#define REST_PARSE_ONE_DECIMAL(buffer, max, data) { \
  uint32_t value; \
  if (coap_strtofix((char const *)buffer, max, &value, 1)) { \
    data = value; \
    success = 1; \
  } \
}

#define REST_PARSE_ONE_STR(payload, len, actuator_set) {\
  success = actuator_set(payload, len); \
}

/*---------------------------------------------------------------------------*/

int
coap_add_ipaddr(char * buf, int size, const uip_ipaddr_t *addr);

resource_t*
rest_find_resource_by_url(const char *url);

void
simple_resource_get_handler(int resource_type, char const * resource_name, void *resource_value, void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

void
simple_resource_set_handler(int resource_type, char const * resource_name, int(*resource_set)(uint32_t value, uint32_t len), void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

void
full_resource_get_handler(coap_full_resource_t *resource_info, void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

void
full_resource_config_handler(coap_full_resource_t *resource_info, void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

int
full_resource_config_attr_handler(coap_full_resource_t *resource_info, char *buffer, int size);

/*---------------------------------------------------------------------------*/

#define IF_MACRO(x) REST_CONF_IF(x)
#define RT_MACRO(x) REST_CONF_RT(x)
#define CT_MACRO(x) REST_CONF_CT(x)
#define OBS_MACRO REST_CONF_OBS

#ifndef REST_CONF_IF
#define REST_CONF_IF(resource_if) ";if=\""resource_if"\""
#endif
#ifndef REST_CONF_RT
#define REST_CONF_RT(resource_rt) ";rt=\""resource_rt"\""
#endif
#ifndef REST_CONF_CT
#define REST_CONF_CT(resource_ct) ";ct=" resource_ct
#endif
#ifndef REST_CONF_OBS
#define REST_CONF_OBS ";obs"
#endif

#define TYPE_EVAL(x) TYPE_CONCAT(x)
#define TYPE_CONCAT(x) COAP_RESOURCE_TYPE_ ## x ## _TYPE
/*---------------------------------------------------------------------------*/

#define RESOURCE_DECL(resource_name) extern resource_t resource_##resource_name

#define REST_RESOURCE_GET_HANDLER(resource_name, resource_format_type, resource_id, resource_value) \
    void \
    resource_##resource_name##_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) \
    { \
      TYPE_EVAL(resource_format_type) value = resource_value; \
      simple_resource_get_handler(resource_format_type, resource_id, (void *)&value, request, response, buffer, preferred_size, offset); \
    }

#define REST_FULL_RESOURCE_GET_HANDLER(resource_name) \
  void \
  resource_##resource_name##_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) \
  { \
    full_resource_get_handler(&resource_##resource_name##_info, request, response, buffer, preferred_size, offset); \
  }

#define REST_RESOURCE_CONFIG_HANDLER(resource_name) \
  void \
  resource_##resource_name##_put_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) \
  { \
    full_resource_config_handler(&resource_##resource_name##_info, request, response, buffer, preferred_size, offset); \
  }

#define REST_RESOURCE_CONFIG_ATTR_HANDLER(resource_name) \
  int \
  resource_##resource_name##_attr_handler(char *buffer, int size) \
  { \
    return full_resource_config_attr_handler(&resource_##resource_name##_info, buffer, size); \
  }

#define REST_RESOURCE_PUT_HANDLER(resource_name, resource_format_type, resource_id, resource_actuator) \
  void \
  resource_##resource_name##_put_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) \
  { \
  simple_resource_set_handler(resource_format_type, resource_id, resource_actuator, request, response, buffer, preferred_size, offset); \
  }

#define REST_RESOURCE_POST_HANDLER(resource_name, parser, actuator_set) \
  void \
  resource_##resource_name##_post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) \
  { \
    const uint8_t * payload; \
    int success = 0; \
    size_t len = REST.get_request_payload(request, &payload); \
    parser(payload, len, actuator_set); \
    REST.set_response_status(response, success ? REST.status.CHANGED : REST.status.BAD_REQUEST); \
  }

#define REST_RESOURCE_PERIODIC_HANDLER(resource_name) \
  void \
  resource_##resource_name##_periodic_handler(void) \
  { \
    REST.notify_subscribers(&resource_##resource_name);\
  }

#define REST_RESOURCE_EVENT_HANDLER(resource_name) \
  void \
  resource_##resource_name##_event_handler(void) \
  { \
    REST.notify_subscribers(&resource_##resource_name);\
  }

#if REST_HAS_ATTR_METHOD
#define EVENT_RESOURCE_ATTR(name, attributes, get_handler, post_handler, put_handler, delete_handler, event_handler, attr_handler) \
resource_t name = { NULL, NULL, IS_OBSERVABLE, attributes, get_handler, post_handler, put_handler, delete_handler, { .trigger = event_handler }, attr_handler }
#else
#define EVENT_RESOURCE_ATTR(name, attributes, get_handler, post_handler, put_handler, delete_handler, event_handler, attr_handler) \
resource_t name = { NULL, NULL, IS_OBSERVABLE, attributes, get_handler, post_handler, put_handler, delete_handler, { .trigger = event_handler } }
#endif

/*---------------------------------------------------------------------------*/

#define REST_RESOURCE(resource_name, ignore, resource_if, resource_type, resource_format_type, resource_id, resource_value) \
  RESOURCE_DECL(resource_name); \
  REST_RESOURCE_GET_HANDLER(resource_name, resource_format_type, resource_id, resource_value) \
  RESOURCE(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(CT_EVAL(COAP_DATA_FORMAT)), resource_##resource_name##_get_handler, NULL, NULL, NULL);

#define REST_ACTUATOR(resource_name, ignore, resource_if, resource_type, resource_format_type, resource_id, resource_value, resource_actuator) \
  RESOURCE_DECL(resource_name); \
  REST_RESOURCE_GET_HANDLER(resource_name, resource_format_type, resource_id, resource_value) \
  REST_RESOURCE_PUT_HANDLER(resource_name, resource_format_type, resource_id, resource_actuator) \
  RESOURCE(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(CT_EVAL(COAP_DATA_FORMAT)), resource_##resource_name##_get_handler, NULL, resource_##resource_name##_put_handler, NULL);

#define REST_EXEC(resource_name, ignore, resource_if, resource_type, parser, actuator) \
  RESOURCE_DECL(resource_name); \
  REST_RESOURCE_POST_HANDLER(resource_name, parser, actuator) \
  RESOURCE(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(CT_EVAL(COAP_DATA_FORMAT)), NULL, resource_##resource_name##_post_handler, NULL, NULL);

#define REST_PERIODIC_RESOURCE(resource_name, resource_period, resource_if, resource_type, resource_format_type, resource_id, resource_value) \
  RESOURCE_DECL(resource_name); \
  REST_RESOURCE_GET_HANDLER(resource_name, resource_format_type, resource_id, resource_value) \
  REST_RESOURCE_PERIODIC_HANDLER(resource_name) \
  PERIODIC_RESOURCE(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(CT_EVAL(COAP_DATA_FORMAT)) OBS_MACRO, resource_##resource_name##_get_handler, NULL, NULL, NULL, (resource_period * CLOCK_SECOND), resource_##resource_name##_periodic_handler);

#define REST_EVENT_RESOURCE(resource_name, ignore, resource_if, resource_type, resource_format_type, resource_id, resource_value) \
  RESOURCE_DECL(resource_name); \
  REST_RESOURCE_GET_HANDLER(resource_name, resource_format_type, resource_id, resource_value) \
  REST_RESOURCE_EVENT_HANDLER(resource_name) \
  EVENT_RESOURCE(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(CT_EVAL(COAP_DATA_FORMAT)) OBS_MACRO, resource_##resource_name##_get_handler, NULL, NULL, NULL, resource_##resource_name##_event_handler);

#define REST_FULL_RESOURCE(resource_name, resource_period, resource_if, resource_type, resource_format_type, resource_id, resource_value) \
  RESOURCE_DECL(resource_name); \
  extern void update_resource_##resource_name##_value(coap_resource_data_t *data); \
  coap_full_resource_t resource_##resource_name##_info = { \
      .coap_resource = &resource_##resource_name, \
      .flags = resource_format_type, \
      .trigger = { .flags = resource_period != 0 ? COAP_BINDING_FLAGS_PMIN_VALID : 0, .pmin = resource_period }, \
      .update_value = update_resource_##resource_name##_value, \
      .name = resource_id, \
  }; \
  REST_FULL_RESOURCE_GET_HANDLER(resource_name) \
  REST_RESOURCE_CONFIG_HANDLER(resource_name) \
  REST_RESOURCE_CONFIG_ATTR_HANDLER(resource_name) \
  REST_RESOURCE_EVENT_HANDLER(resource_name) \
  void update_resource_##resource_name##_value(coap_resource_data_t *data) { \
    data->last_value = (resource_value); \
  } \
  EVENT_RESOURCE_ATTR(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(CT_EVAL(COAP_DATA_FORMAT)) OBS_MACRO, resource_##resource_name##_get_handler, NULL, resource_##resource_name##_put_handler, NULL, resource_##resource_name##_event_handler, resource_##resource_name##_attr_handler);

#define INIT_RESOURCE(resource_name, path) \
    extern resource_t resource_##resource_name; \
    rest_activate_resource(&resource_##resource_name, path);

#define INIT_FULL_RESOURCE(resource_name, path) \
    extern resource_t resource_##resource_name; \
    rest_activate_resource(&resource_##resource_name, path); \
    extern coap_full_resource_t resource_##resource_name##_info; \
    coap_binding_add_resource(&resource_##resource_name##_info);

#define REST_RES_REF(resource_name) &resource_##resource_name,

#endif /* COAP_COMMON_H */
