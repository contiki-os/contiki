/*
 * Copyright (c) 2015, Yanzi Networks AB.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup oma-lwm2m An implementation of OMA LWM2M
 * @{
 *
 * This application is an implementation of OMA Lightweight M2M.
 */

/**
 * \file
 *         Header file for the Contiki OMA LWM2M object API
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#ifndef LWM2M_OBJECT_H_
#define LWM2M_OBJECT_H_

#include "rest-engine.h"
#include "er-coap-observe.h"

#define LWM2M_OBJECT_SECURITY_ID                0
#define LWM2M_OBJECT_SERVER_ID                  1
#define LWM2M_OBJECT_ACCESS_CONTROL_ID          2
#define LWM2M_OBJECT_DEVICE_ID                  3
#define LWM2M_OBJECT_CONNECTIVITY_MONITORING_ID 4
#define LWM2M_OBJECT_FIRMWARE_ID                5
#define LWM2M_OBJECT_LOCATION_ID                6
#define LWM2M_OBJECT_CONNECTIVITY_STATISTICS_ID 7

#define LWM2M_SECURITY_SERVER_URI               0
#define LWM2M_SECURITY_BOOTSTRAP_SERVER         1
#define LWM2M_SECURITY_MODE                     2
#define LWM2M_SECURITY_CLIENT_PKI               3
#define LWM2M_SECURITY_SERVER_PKI               4
#define LWM2M_SECURITY_KEY                      5
#define LWM2M_SECURITY_SHORT_SERVER_ID         10

/* Pre-shared key mode */
#define LWM2M_SECURITY_MODE_PSK                 0
/* Raw Public Key mode */
#define LWM2M_SECURITY_MODE_RPK                 1
/* Certificate mode */
#define LWM2M_SECURITY_MODE_CERTIFICATE         2
/* NoSec mode */
#define LWM2M_SECURITY_MODE_NOSEC               3

#define LWM2M_OBJECT_STR_HELPER(x) (uint8_t *) #x
#define LWM2M_OBJECT_STR(x) LWM2M_OBJECT_STR_HELPER(x)

#define LWM2M_OBJECT_PATH_STR_HELPER(x) #x
#define LWM2M_OBJECT_PATH_STR(x) LWM2M_OBJECT_PATH_STR_HELPER(x)

struct lwm2m_reader;
struct lwm2m_writer;
/* Data model for OMA LWM2M objects */
typedef struct lwm2m_context {
  uint16_t object_id;
  uint16_t object_instance_id;
  uint16_t resource_id;
  uint8_t object_instance_index;
  uint8_t resource_index;
  /* TODO - add uint16_t resource_instance_id */

  const struct lwm2m_reader *reader;
  const struct lwm2m_writer *writer;
} lwm2m_context_t;

/* LWM2M format writer for the various formats supported */
typedef struct lwm2m_writer {
  size_t (* write_int)(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen, int32_t value);
  size_t (* write_string)(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen, const char *value, size_t strlen);
  size_t (* write_float32fix)(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen, int32_t value, int bits);
  size_t (* write_boolean)(const lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen, int value);
} lwm2m_writer_t;

typedef struct lwm2m_reader {
  size_t (* read_int)(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len, int32_t *value);
  size_t (* read_string)(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len, uint8_t *value, size_t strlen);
  size_t (* read_float32fix)(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len, int32_t *value, int bits);
  size_t (* read_boolean)(const lwm2m_context_t *ctx, const uint8_t *inbuf, size_t len, int *value);
} lwm2m_reader_t;

typedef struct lwm2m_value_callback {
  int (* read)(lwm2m_context_t *ctx, uint8_t *outbuf, size_t outlen);
  int (* write)(lwm2m_context_t *ctx,
                const uint8_t *buffer, size_t len,
                uint8_t *outbuf, size_t outlen);
  int (* exec)(lwm2m_context_t *ctx, const uint8_t *arg, size_t len,
               uint8_t *outbuf, size_t outlen);
} lwm2m_value_callback_t;

#define LWM2M_RESOURCE_TYPE_STR_VALUE                1
#define LWM2M_RESOURCE_TYPE_STR_VARIABLE             2
#define LWM2M_RESOURCE_TYPE_STR_VARIABLE_ARRAY       3
#define LWM2M_RESOURCE_TYPE_INT_VALUE                4
#define LWM2M_RESOURCE_TYPE_INT_VARIABLE             5
#define LWM2M_RESOURCE_TYPE_INT_VARIABLE_ARRAY       6
#define LWM2M_RESOURCE_TYPE_FLOATFIX_VALUE           7
#define LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE        8
#define LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE_ARRAY  9
#define LWM2M_RESOURCE_TYPE_BOOLEAN_VALUE           10
#define LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE        11
#define LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE_ARRAY  12
#define LWM2M_RESOURCE_TYPE_CALLBACK                16
#define LWM2M_RESOURCE_TYPE_INSTANCES               17

typedef struct lwm2m_resource {
  uint16_t id;
  uint8_t type; /* indicate value type and multi-instance resource */
  union {
    struct {
      uint16_t len;
      const uint8_t *value;
    } string;
    struct {
      uint16_t size;
      uint16_t *len;
      uint8_t **var;
    } stringvar;
    struct {
      uint16_t count;
      uint16_t size;
      /* string var array with counting entries */
      uint16_t *len;
      uint8_t *var;
    } stringvararr;
    struct {
      int32_t value;
    } integer;
    struct {
      int32_t *var;
    } integervar;
    struct {
      /* used for multiple instances (dynamic) NOTE: this is an index into
         the instance so having two instances means that there is need for
         allocation of two ints here */
      uint16_t count;
      int32_t *var; /* used as an array? */
    } integervararr;
    struct {
      int32_t value;
    } floatfix;
    struct {
      int32_t *var;
    } floatfixvar;
    struct {
      uint16_t count;
      int32_t *var;
    } floatfixvararr;
    struct {
      int value;
    } boolean;
    struct {
      int *var;
    } booleanvar;
    struct {
      uint16_t count;
      int *var;
    } booleanvararr;
    lwm2m_value_callback_t callback;
    /* lwm2m_resource *resources[];  TO BE ADDED LATER*/
  } value;
} lwm2m_resource_t;

#define LWM2M_INSTANCE_FLAG_USED 1

typedef struct lwm2m_instance {
  uint16_t id;
  uint16_t count;
  uint16_t flag;
  const lwm2m_resource_t *resources;
} lwm2m_instance_t;

typedef struct lwm2m_object {
  uint16_t id;
  uint16_t count;
  const char *path;
  resource_t *coap_resource;
  lwm2m_instance_t *instances;
} lwm2m_object_t;

#define LWM2M_RESOURCES(name, ...)                              \
  static const lwm2m_resource_t name[] = { __VA_ARGS__ }

#define LWM2M_RESOURCE_STRING(id, s)                                    \
  { id, LWM2M_RESOURCE_TYPE_STR_VALUE, .value.string.len = sizeof(s) - 1, .value.string.value = (uint8_t *) s }

#define LWM2M_RESOURCE_STRING_VAR(id, s, l, v)                           \
  { id, LWM2M_RESOURCE_TYPE_STR_VARIABLE, .value.stringvar.size = (s), .value.stringvar.len = (l), .value.stringvar.var = (v) }

#define LWM2M_RESOURCE_STRING_VAR_ARR(id, c, s, l, v)                   \
  { id, LWM2M_RESOURCE_TYPE_STR_VARIABLE_ARRAY, .value.stringvararr.count = c, .value.stringvararr.size = s, .value.stringvararr.len = l, .value.stringvararr.var = (uint8_t *) v }

#define LWM2M_RESOURCE_INTEGER(id, v)                                   \
  { id, LWM2M_RESOURCE_TYPE_INT_VALUE, .value.integer.value = (v) }

#define LWM2M_RESOURCE_INTEGER_VAR(id, v)                               \
  { id, LWM2M_RESOURCE_TYPE_INT_VARIABLE, .value.integervar.var = (v) }

#define LWM2M_RESOURCE_INTEGER_VAR_ARR(id, c, v)                        \
  { id, LWM2M_RESOURCE_TYPE_INT_VARIABLE_ARRAY, .value.integervararr.count = (c), .value.integervararr.var = (v) }

#define LWM2M_RESOURCE_FLOATFIX(id, v)                                   \
  { id, LWM2M_RESOURCE_TYPE_FLOATFIX_VALUE, .value.floatfix.value = (v) }

#define LWM2M_RESOURCE_FLOATFIX_VAR(id, v)                              \
  { id, LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE, .value.floatfixvar.var = (v) }

#define LWM2M_RESOURCE_FLOATFIX_VAR_ARR(id, c, v)                       \
  { id, LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE_ARRAY, .value.floatfixvararr.count = (c), .value.floatfixvararr.var = (v) }

#define LWM2M_RESOURCE_BOOLEAN(id, v)                                   \
  { id, LWM2M_RESOURCE_TYPE_BOOLEAN_VALUE, .value.boolean.value = (v) }

#define LWM2M_RESOURCE_BOOLEAN_VAR(id, v)                                   \
  { id, LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE, .value.booleanvar.var = (v) }

#define LWM2M_RESOURCE_BOOLEAN_VAR_ARR(id, c, v)                        \
  { id, LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE_ARRAY, .value.booleanvararr.count = (c), .value.booleanvararr.var = (v) }

#define LWM2M_RESOURCE_CALLBACK(id, ...)                                \
  { id, LWM2M_RESOURCE_TYPE_CALLBACK, .value.callback = __VA_ARGS__ }

#define LWM2M_INSTANCE(id, resources)                           \
  { id, sizeof(resources)/sizeof(lwm2m_resource_t), LWM2M_INSTANCE_FLAG_USED, resources }

#define LWM2M_INSTANCE_UNUSED(id, resources)                      \
  { id, sizeof(resources)/sizeof(lwm2m_resource_t), 0, resources }

#define LWM2M_INSTANCES(name, ...)                              \
  static lwm2m_instance_t name[] = { __VA_ARGS__ }

#define LWM2M_OBJECT(name, id, instances)                       \
    static void lwm2m_get_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset); \
    static void lwm2m_put_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset); \
    static void lwm2m_post_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset); \
    static void lwm2m_delete_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset); \
    static resource_t rest_rsc_##name = { NULL, NULL, HAS_SUB_RESOURCES | IS_OBSERVABLE, NULL, lwm2m_get_h_##name, lwm2m_post_h_##name, lwm2m_put_h_##name, lwm2m_delete_h_##name, { NULL } }; \
    static const lwm2m_object_t name = { id, sizeof(instances)/sizeof(lwm2m_instance_t), LWM2M_OBJECT_PATH_STR(id), &rest_rsc_##name, instances}; \
    static void lwm2m_get_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) { \
      lwm2m_engine_handler(&name, request, response, buffer, preferred_size, offset); } \
    static void lwm2m_put_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) { \
      lwm2m_engine_handler(&name, request, response, buffer, preferred_size, offset); } \
    static void lwm2m_post_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) { \
      lwm2m_engine_handler(&name, request, response, buffer, preferred_size, offset); } \
    static void lwm2m_delete_h_##name(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) { \
      lwm2m_engine_delete_handler(&name, request, response, buffer, preferred_size, offset); }

/* how do we register attributes in the above resource here ??? */

int lwm2m_object_is_resource_string(const lwm2m_resource_t *resource);
int lwm2m_object_is_resource_int(const lwm2m_resource_t *resource);
int lwm2m_object_is_resource_floatfix(const lwm2m_resource_t *resource);
int lwm2m_object_is_resource_boolean(const lwm2m_resource_t *resource);

static inline int
lwm2m_object_is_resource_callback(const lwm2m_resource_t *resource)
{
  return resource != NULL && resource->type == LWM2M_RESOURCE_TYPE_CALLBACK;
}

const uint8_t *
lwm2m_object_get_resource_string(const lwm2m_resource_t *resource,
                                 const lwm2m_context_t *context);

uint16_t
lwm2m_object_get_resource_strlen(const lwm2m_resource_t *resource,
                                 const lwm2m_context_t *context);

int
lwm2m_object_set_resource_string(const lwm2m_resource_t *resource,
                                 const lwm2m_context_t *context,
                                 uint16_t len, const uint8_t *string);

int
lwm2m_object_get_resource_int(const lwm2m_resource_t *resource,
                              const lwm2m_context_t *context,
                              int32_t *value);

int
lwm2m_object_set_resource_int(const lwm2m_resource_t *resource,
                              const lwm2m_context_t *context,
                              int32_t value);

int
lwm2m_object_get_resource_floatfix(const lwm2m_resource_t *resource,
                                   const lwm2m_context_t *context,
                                   int32_t *value);

int
lwm2m_object_set_resource_floatfix(const lwm2m_resource_t *resource,
                                   const lwm2m_context_t *context,
                                   int32_t value);

int
lwm2m_object_get_resource_boolean(const lwm2m_resource_t *resource,
                                  const lwm2m_context_t *context,
                                  int *value);

int
lwm2m_object_set_resource_boolean(const lwm2m_resource_t *resource,
                                  const lwm2m_context_t *context,
                                  int value);

static inline resource_t *
lwm2m_object_get_coap_resource(const lwm2m_object_t *object)
{
  return (resource_t *)object->coap_resource;
}

static inline void
lwm2m_object_notify_observers(const lwm2m_object_t *object, char *path)
{
  coap_notify_observers_sub(lwm2m_object_get_coap_resource(object), path);
}

#include "lwm2m-engine.h"

#endif /* LWM2M_OBJECT_H_ */
/**
 * @}
 * @}
 */
