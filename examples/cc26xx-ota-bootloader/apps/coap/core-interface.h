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
#ifndef CORE_INTERFACE_H
#define CORE_INTERFACE_H

#include "coap-common.h"
#include "coap-push.h"

// Global variable for CoRE linked batch
extern int core_itf_linked_batch_resource;
// -------------------------------------

#ifdef CORE_ITF_CONF_MAX_BATCH_BUFFER_SIZE
#define CORE_ITF_MAX_BATCH_BUFFER_SIZE CORE_ITF_CONF_MAX_BATCH_BUFFER_SIZE
#else
#define CORE_ITF_MAX_BATCH_BUFFER_SIZE 256
#endif

// Resources path

#define BINDING_TABLE_RES "bnd"
#define LINKED_BATCH_TABLE_RES "l"

// Interface Description

#define IF_LINK_LIST "core.ll"
#define IF_BATCH "core.b"
#define IF_LINKED_BATCH "core.lb"
#define IF_SENSOR "core.s"
#define IF_PARAMETER "core.p"
#define IF_RO_PARAMETER "core.rp"
#define IF_ACTUATOR "core.a"
#define IF_BINDING "core.bnd"

#define LIST_INCLUDE_SELF 0x0001
#define LIST_INCLUDE_ATTR 0x0002

// Resource handlers

extern void
resource_batch_get_handler(uint8_t *batch_buffer, int *batch_buffer_size, resource_t const * batch_resource, resource_t * batch_resource_list[], int batch_resource_list_size, uint16_t flags, void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

extern void
resource_linked_list_get_handler(resource_t const * linked_list_resource, resource_t * linked_resource_list[], int linked_resource_list_size, uint16_t flags,
    void *request, void *response, uint8_t *buffer,
    uint16_t preferred_size, int32_t *offset);


// Handler definition macros

#define REST_RESOURCES_LIST(resource_name, ...) \
  resource_t *resource_##resource_name##_batch_list[] = {__VA_ARGS__};

#define REST_RESOURCES_LIST_SIZE(resource_name) (sizeof(resource_##resource_name##_batch_list) / sizeof(resource_t *))

#define REST_RESOURCE_BATCH_HANDLER(resource_name, flags, ...) \
  REST_RESOURCES_LIST(resource_name, __VA_ARGS__); \
  void \
  resource_##resource_name##_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) \
  { \
    static uint8_t batch_buffer[CORE_ITF_MAX_BATCH_BUFFER_SIZE+1]; \
    static int batch_buffer_size = 0; \
    resource_batch_get_handler(batch_buffer, &batch_buffer_size, &resource_##resource_name, resource_##resource_name##_batch_list, REST_RESOURCES_LIST_SIZE(resource_name), flags, request, response, buffer, preferred_size, offset); \
  }

#define REST_RESOURCE_LINKED_LIST_HANDLER(resource_name, flags, ...) \
  REST_RESOURCES_LIST(resource_name, __VA_ARGS__); \
  void \
  resource_##resource_name##_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) \
  { \
    resource_linked_list_get_handler(&resource_##resource_name, resource_##resource_name##_batch_list, REST_RESOURCES_LIST_SIZE(resource_name), flags, request, response, buffer, preferred_size, offset); \
  }

// Handler macros

#define BATCH_RESOURCE(resource_name, flags, resource_if, resource_type, ...) \
  RESOURCE_DECL(resource_name); \
  REST_RESOURCE_BATCH_HANDLER(resource_name, flags, __VA_ARGS__) \
  RESOURCE(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(TO_STRING(40) " " CT_EVAL(COAP_DATA_FORMAT)), resource_##resource_name##_get_handler, NULL, NULL, NULL);

#define LINKED_LIST_RESOURCE(resource_name, flags, resource_if, resource_type, ...) \
  RESOURCE_DECL(resource_name); \
  REST_RESOURCE_LINKED_LIST_HANDLER(resource_name, flags, __VA_ARGS__) \
  RESOURCE(resource_##resource_name, "" IF_MACRO(resource_if) RT_MACRO(resource_type) CT_MACRO(40) OBS_MACRO, resource_##resource_name##_get_handler, NULL, NULL, NULL);

#endif /* CORE_INTERFACE_H */
