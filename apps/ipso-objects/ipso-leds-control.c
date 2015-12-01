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
 * \addtogroup ipso-objects
 * @{
 *
 */

/**
 * \file
 *         Implementation of OMA LWM2M / IPSO Light Control for LEDs
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "lwm2m-object.h"
#include "lwm2m-engine.h"
#include "er-coap-engine.h"
#include "dev/leds.h"
#include <stdint.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if LEDS_ALL > 3
#define LEDS_CONTROL_NUMBER 3
#elif LEDS_ALL > 1
#define LEDS_CONTROL_NUMBER 2
#else
#define LEDS_CONTROL_NUMBER 1
#endif

struct led_state {
  unsigned long last_on_time;
  uint32_t total_on_time;
  uint8_t is_on;
};

static struct led_state states[LEDS_CONTROL_NUMBER];
static lwm2m_instance_t leds_control_instances[LEDS_CONTROL_NUMBER];
/*---------------------------------------------------------------------------*/
static int
read_state(lwm2m_context_t *ctx, uint8_t *outbuf, size_t outsize)
{
  uint8_t idx = ctx->object_instance_index;
  if(idx >= LEDS_CONTROL_NUMBER) {
    return 0;
  }
  return ctx->writer->write_boolean(ctx, outbuf, outsize,
                                    states[idx].is_on ? 1 : 0);
}
/*---------------------------------------------------------------------------*/
static int
write_state(lwm2m_context_t *ctx, const uint8_t *inbuf, size_t insize,
            uint8_t *outbuf, size_t outsize)
{
  int value;
  size_t len;

  uint8_t idx = ctx->object_instance_index;
  if(idx >= LEDS_CONTROL_NUMBER) {
    return 0;
  }

  len = ctx->reader->read_boolean(ctx, inbuf, insize, &value);
  if(len > 0) {
    if(value) {
      if(!states[idx].is_on) {
        states[idx].is_on = 1;
        states[idx].last_on_time = clock_seconds();
#if PLATFORM_HAS_LEDS
        leds_on(1 << idx);
#endif /* PLATFORM_HAS_LEDS */
      }
    } else if(states[idx].is_on) {
      states[idx].total_on_time += clock_seconds() - states[idx].last_on_time;
      states[idx].is_on = 0;
#if PLATFORM_HAS_LEDS
      leds_off(1 << idx);
#endif /* PLATFORM_HAS_LEDS */
    }
  } else {
    PRINTF("IPSO leds control - ignored illegal write to on/off\n");
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static int
read_on_time(lwm2m_context_t *ctx, uint8_t *outbuf, size_t outsize)
{
  unsigned long now;
  uint8_t idx = ctx->object_instance_index;
  if(idx >= LEDS_CONTROL_NUMBER) {
    return 0;
  }

  if(states[idx].is_on) {
    /* Update the on time */
    now = clock_seconds();
    states[idx].total_on_time += now - states[idx].last_on_time;
    states[idx].last_on_time = now;
  }
  return ctx->writer->write_int(ctx, outbuf, outsize,
                                (int32_t)states[idx].total_on_time);
}
/*---------------------------------------------------------------------------*/
static int
write_on_time(lwm2m_context_t *ctx,
              const uint8_t *inbuf, size_t insize,
              uint8_t *outbuf, size_t outsize)
{
  int32_t value;
  size_t len;
  uint8_t idx = ctx->object_instance_index;
  if(idx >= LEDS_CONTROL_NUMBER) {
    return 0;
  }

  len = ctx->reader->read_int(ctx, inbuf, insize, &value);
  if(len > 0 && value == 0) {
    PRINTF("IPSO leds control - reset On Time\n");
    states[idx].total_on_time = 0;
    if(states[idx].is_on) {
      states[idx].last_on_time = clock_seconds();
    }
  } else {
    PRINTF("IPSO leds control - ignored illegal write to On Time\n");
  }
  return len;
}
/*---------------------------------------------------------------------------*/
LWM2M_RESOURCES(leds_control_resources,
                LWM2M_RESOURCE_CALLBACK(5850, { read_state, write_state, NULL }),
                LWM2M_RESOURCE_CALLBACK(5852, { read_on_time, write_on_time, NULL })
                );
LWM2M_OBJECT(leds_control, 3311, leds_control_instances);
/*---------------------------------------------------------------------------*/
void
ipso_leds_control_init(void)
{
  lwm2m_instance_t template = LWM2M_INSTANCE(0, leds_control_resources);
  int i;

  /* Initialize the instances */
  for(i = 0; i < LEDS_CONTROL_NUMBER; i++) {
    leds_control_instances[i] = template;
    leds_control_instances[i].id = i;
  }

  /* register this device and its handlers - the handlers automatically
     sends in the object to handle */
  lwm2m_engine_register_object(&leds_control);
  PRINTF("IPSO leds control initialized with %u instances\n",
         LEDS_CONTROL_NUMBER);
}
/*---------------------------------------------------------------------------*/
/** @} */
