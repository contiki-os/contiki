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
 *         Implementation of the Contiki OMA LWM2M security
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include <stdint.h>
#include "lwm2m-object.h"
#include "lwm2m-engine.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifdef LWM2M_CONF_SERVER_MAX_COUNT
#define MAX_COUNT LWM2M_CONF_SERVER_MAX_COUNT
#else
#define MAX_COUNT 2
#endif

/* hoping that we do not get more than 64 bytes... */
#define MAX_SIZE 64

static int32_t bs_arr[MAX_COUNT];
static int32_t secmode_arr[MAX_COUNT];
static int32_t sid_arr[MAX_COUNT];

static char server_uri[MAX_COUNT][MAX_SIZE];
static uint16_t su_len[MAX_COUNT];
static char client_id[MAX_COUNT][MAX_SIZE];
static uint16_t client_id_len[MAX_COUNT];
static char server_id[MAX_COUNT][MAX_SIZE];
static uint16_t server_id_len[MAX_COUNT];
static char psk_key[MAX_COUNT][MAX_SIZE];
static uint16_t psk_key_len[MAX_COUNT];
static lwm2m_instance_t security_instances[MAX_COUNT];

LWM2M_RESOURCES(security_resources,
                LWM2M_RESOURCE_STRING_VAR_ARR(0, MAX_COUNT, MAX_SIZE, su_len, server_uri),
                LWM2M_RESOURCE_INTEGER_VAR_ARR(1, MAX_COUNT, bs_arr),
                LWM2M_RESOURCE_INTEGER_VAR_ARR(2, MAX_COUNT, secmode_arr),
                LWM2M_RESOURCE_STRING_VAR_ARR(3, MAX_COUNT, MAX_SIZE, client_id_len, client_id),
                LWM2M_RESOURCE_STRING_VAR_ARR(4, MAX_COUNT, MAX_SIZE, server_id_len, server_id),
                /* TODO This should not be readable! */
                LWM2M_RESOURCE_STRING_VAR_ARR(5, MAX_COUNT, MAX_SIZE, psk_key_len, psk_key),
                LWM2M_RESOURCE_INTEGER_VAR_ARR(10, MAX_COUNT, sid_arr)
                );
LWM2M_OBJECT(security, 0, security_instances);
/*---------------------------------------------------------------------------*/
void
lwm2m_security_init(void)
{
  lwm2m_instance_t template = LWM2M_INSTANCE_UNUSED(0, security_resources);
  int i;

  /* Initialize the instances */
  for(i = 0; i < MAX_COUNT; i++) {
    security_instances[i] = template;
    security_instances[i].id = i;
  }

  /**
   * Register this device and its handlers - the handlers
   * automatically sends in the object to handle.
   */
  PRINTF("*** Init lwm2m-security\n");
  lwm2m_engine_register_object(&security);
}
/*---------------------------------------------------------------------------*/
/** @} */
