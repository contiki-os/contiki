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
 *         Implementation of OMA LWM2M / IPSO Light Control
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "ipso-objects.h"
#include "lwm2m-object.h"
#include "lwm2m-engine.h"

#ifdef IPSO_LIGHT_CONTROL
extern const struct ipso_objects_actuator IPSO_LIGHT_CONTROL;
#endif /* IPSO_LIGHT_CONTROL */
/*---------------------------------------------------------------------------*/
static unsigned long last_on_time;
static uint32_t total_on_time;
static int dim_level = 0;
static uint8_t is_on = 0;
/*---------------------------------------------------------------------------*/
static int
read_state(lwm2m_context_t *ctx, uint8_t *outbuf, size_t outsize)
{
  return ctx->writer->write_boolean(ctx, outbuf, outsize, is_on ? 1 : 0);
}
/*---------------------------------------------------------------------------*/
static int
write_state(lwm2m_context_t *ctx, const uint8_t *inbuf, size_t insize,
            uint8_t *outbuf, size_t outsize)
{
  int value;
  size_t len;

  len = ctx->reader->read_boolean(ctx, inbuf, insize, &value);
  if(len > 0) {
    if(value) {
      if(!is_on) {
        is_on = 1;
        last_on_time = clock_seconds();
      }
    } else {
      if(is_on) {
        total_on_time += clock_seconds() - last_on_time;
        is_on = 0;
      }
    }
#ifdef IPSO_LIGHT_CONTROL
    if(IPSO_LIGHT_CONTROL.set_on) {
      IPSO_LIGHT_CONTROL.set_on(value);
    } else if(IPSO_LIGHT_CONTROL.set_dim_level) {
      dim_level = value ? 100 : 0;
      IPSO_LIGHT_CONTROL.set_dim_level(dim_level);
    }
#endif /* IPSO_LIGHT_CONTROL */
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static int
read_dim(lwm2m_context_t *ctx, uint8_t *outbuf, size_t outsize)
{
  return ctx->writer->write_int(ctx, outbuf, outsize, dim_level);
}
/*---------------------------------------------------------------------------*/
static int
write_dim(lwm2m_context_t *ctx, const uint8_t *inbuf, size_t insize,
          uint8_t *outbuf, size_t outsize)
{
  int32_t value;
  size_t len;

  len = ctx->reader->read_int(ctx, inbuf, insize, &value);
  if(len > 0) {
    if(value < 0) {
      value = 0;
    } else if(value > 100) {
      value = 100;
    }

    dim_level = value;
    if(value > 0) {
      if(!is_on) {
        is_on = 1;
        last_on_time = clock_seconds();
      }
    } else {
      if(is_on) {
        total_on_time += clock_seconds() - last_on_time;
        is_on = 0;
      }
    }
#ifdef IPSO_LIGHT_CONTROL
    if(IPSO_LIGHT_CONTROL.set_dim_level) {
      IPSO_LIGHT_CONTROL.set_dim_level(dim_level);
    } else if(IPSO_LIGHT_CONTROL.set_on) {
      IPSO_LIGHT_CONTROL.set_on(is_on);
    }
#endif /* IPSO_LIGHT_CONTROL */
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static int
read_on_time(lwm2m_context_t *ctx, uint8_t *outbuf, size_t outsize)
{
  unsigned long now;
  if(is_on) {
    /* Update the on time */
    now = clock_seconds();
    total_on_time += now - last_on_time;
    last_on_time = now;
  }
  return ctx->writer->write_int(ctx, outbuf, outsize, (int32_t)total_on_time);
}
/*---------------------------------------------------------------------------*/
static int
write_on_time(lwm2m_context_t *ctx,
              const uint8_t *inbuf, size_t insize,
              uint8_t *outbuf, size_t outsize)
{
  int32_t value;
  size_t len;

  len = ctx->reader->read_int(ctx, inbuf, insize, &value);
  if(len > 0 && value == 0) {
    total_on_time = 0;
    if(is_on) {
      last_on_time = clock_seconds();
    }
  }
  return len;
}
/*---------------------------------------------------------------------------*/
LWM2M_RESOURCES(light_control_resources,
                LWM2M_RESOURCE_CALLBACK(5850, { read_state, write_state, NULL }),
                LWM2M_RESOURCE_CALLBACK(5851, { read_dim, write_dim, NULL }),
                LWM2M_RESOURCE_CALLBACK(5852, { read_on_time, write_on_time, NULL }),
                );
LWM2M_INSTANCES(light_control_instances,
		LWM2M_INSTANCE(0, light_control_resources));
LWM2M_OBJECT(light_control, 3311, light_control_instances);
/*---------------------------------------------------------------------------*/
void
ipso_light_control_init(void)
{
#ifdef IPSO_LIGHT_CONTROL
  if(IPSO_LIGHT_CONTROL.init) {
    IPSO_LIGHT_CONTROL.init();
  }
  if(IPSO_LIGHT_CONTROL.is_on) {
    is_on = IPSO_LIGHT_CONTROL.is_on();
  }
  if(IPSO_LIGHT_CONTROL.get_dim_level) {
    dim_level = IPSO_LIGHT_CONTROL.get_dim_level();
    if(dim_level > 0 && IPSO_LIGHT_CONTROL.is_on == NULL) {
      is_on = 1;
    }
  }
#endif /* IPSO_LIGHT_CONTROL */
  last_on_time = clock_seconds();

  lwm2m_engine_register_object(&light_control);
}
/*---------------------------------------------------------------------------*/
/** @} */
