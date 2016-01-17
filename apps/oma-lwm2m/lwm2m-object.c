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
 * \addtogroup oma-lwm2m
 * @{
 *
 */

/**
 * \file
 *         Implementation of the Contiki OMA LWM2M object API
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "lwm2m-object.h"
#include <string.h>
/*---------------------------------------------------------------------------*/
int
lwm2m_object_is_resource_string(const lwm2m_resource_t *resource)
{
  if(resource == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VALUE ||
     resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE ||
     resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE_ARRAY) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
const uint8_t *
lwm2m_object_get_resource_string(const lwm2m_resource_t *resource,
                                 const lwm2m_context_t *context)
{
  if(resource == NULL || context == NULL) {
    return NULL;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VALUE) {
    return resource->value.string.value;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE) {
    return *(resource->value.stringvar.var);
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.stringvararr.count) {
      return resource->value.stringvararr.var +
        resource->value.stringvararr.size * context->object_instance_index;
    }
    return NULL;
  }
  /* Not a string */
  return NULL;
}
/*---------------------------------------------------------------------------*/
uint16_t
lwm2m_object_get_resource_strlen(const lwm2m_resource_t *resource,
                                 const lwm2m_context_t *context)
{
  if(resource == NULL || context == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VALUE) {
    return resource->value.string.len;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE) {
    return *(resource->value.stringvar.len);
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.stringvararr.count) {
      return resource->value.stringvararr.len[context->object_instance_index];
    }
    return 0;
  }
  /* Not a string */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_set_resource_string(const lwm2m_resource_t *resource,
                                 const lwm2m_context_t *context,
                                 uint16_t len, const uint8_t *string)
{
  if(resource == NULL || context == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE) {
    if(len > resource->value.stringvar.size) {
      /* Too large */
      return 0;
    }
    memcpy(resource->value.stringvar.var, string, len);
    *(resource->value.stringvar.len) = len;
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_STR_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.stringvararr.count &&
       len <= resource->value.stringvararr.size) {
      memcpy(resource->value.stringvararr.var +
             resource->value.stringvararr.size * context->object_instance_index,
             string, len);
      resource->value.stringvararr.len[context->object_instance_index] = len;
      return 1;
    }
    return 0;
  }
  /* Not a string variable */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_is_resource_int(const lwm2m_resource_t *resource)
{
  if(resource == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_INT_VALUE ||
     resource->type == LWM2M_RESOURCE_TYPE_INT_VARIABLE ||
     resource->type == LWM2M_RESOURCE_TYPE_INT_VARIABLE_ARRAY) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_get_resource_int(const lwm2m_resource_t *resource,
                              const lwm2m_context_t *context,
                              int32_t *value)
{
  if(resource == NULL || context == NULL || value == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_INT_VALUE) {
    *value = resource->value.integer.value;
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_INT_VARIABLE) {
    *value = *(resource->value.integervar.var);
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_INT_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.integervararr.count) {
      *value = resource->value.integervararr.var[context->object_instance_index];
      return 1;
    }
    return 0;
  }
  /* Not an integer */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_set_resource_int(const lwm2m_resource_t *resource,
                              const lwm2m_context_t *context,
                              int32_t value)
{
  if(resource == NULL || context == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_INT_VARIABLE) {
    *(resource->value.integervar.var) = value;
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_INT_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.integervararr.count) {
      resource->value.integervararr.var[context->object_instance_index] =
        value;
      return 1;
    }
    return 0;
  }
  /* Not an integer variable */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_is_resource_floatfix(const lwm2m_resource_t *resource)
{
  if(resource == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VALUE ||
     resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE ||
     resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE_ARRAY) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_get_resource_floatfix(const lwm2m_resource_t *resource,
                                   const lwm2m_context_t *context,
                                   int32_t *value)
{
  if(resource == NULL || context == NULL || value == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VALUE) {
    *value = resource->value.floatfix.value;
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE) {
    *value = *(resource->value.floatfixvar.var);
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.floatfixvararr.count) {
      *value = resource->value.floatfixvararr.var[context->object_instance_index];
      return 1;
    }
    return 0;
  }
  /* Not an float */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_set_resource_floatfix(const lwm2m_resource_t *resource,
                                   const lwm2m_context_t *context,
                                   int32_t value)
{
  if(resource == NULL || context == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE) {
    *(resource->value.floatfixvar.var) = value;
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_FLOATFIX_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.floatfixvararr.count) {
      resource->value.floatfixvararr.var[context->object_instance_index] =
        value;
      return 1;
    }
    return 0;
  }
  /* Not an float variable */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_is_resource_boolean(const lwm2m_resource_t *resource)
{
  if(resource == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VALUE ||
     resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE ||
     resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE_ARRAY) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_get_resource_boolean(const lwm2m_resource_t *resource,
                                  const lwm2m_context_t *context,
                                  int *value)
{
  if(resource == NULL || context == NULL || value == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VALUE) {
    *value = resource->value.boolean.value;
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE) {
    *value = *(resource->value.booleanvar.var);
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.booleanvararr.count) {
      *value = resource->value.booleanvararr.var[context->object_instance_index];
      return 1;
    }
    return 0;
  }
  /* Not a boolean */
  return 0;
}
/*---------------------------------------------------------------------------*/
int
lwm2m_object_set_resource_boolean(const lwm2m_resource_t *resource,
                                  const lwm2m_context_t *context,
                                  int value)
{
  if(resource == NULL || context == NULL) {
    return 0;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE) {
    *(resource->value.booleanvar.var) = value;
    return 1;
  }
  if(resource->type == LWM2M_RESOURCE_TYPE_BOOLEAN_VARIABLE_ARRAY) {
    if(context->object_instance_index < resource->value.booleanvararr.count) {
      resource->value.booleanvararr.var[context->object_instance_index] =
        value;
      return 1;
    }
    return 0;
  }
  /* Not a boolean variable */
  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
