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
 */

/**
 * \file
 *         Implementation of the Contiki OMA LWM2M device
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "lwm2m-object.h"
#include "lwm2m-device.h"
#include "lwm2m-engine.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static int32_t time_offset = 0;
/*---------------------------------------------------------------------------*/
static int
read_lwtime(lwm2m_context_t *ctx, uint8_t *outbuf, size_t outsize)
{
  return ctx->writer->write_int(ctx, outbuf, outsize,
                                time_offset + clock_seconds());
}
/*---------------------------------------------------------------------------*/
static int
set_lwtime(lwm2m_context_t *ctx, const uint8_t *inbuf, size_t insize,
           uint8_t *outbuf, size_t outsize)
{
  /* assume that this only read one TLV value */
  int32_t lw_time;
  size_t len = ctx->reader->read_int(ctx, inbuf, insize, &lw_time);
  if(len == 0) {
    PRINTF("FAIL: could not read time '%*.s'\n", (int)insize, inbuf);
  } else {
    PRINTF("Got: time: %*.s => %" PRId32 "\n", (int)insize, inbuf, lw_time);

    time_offset = lw_time - clock_seconds();
    PRINTF("Write time...%" PRId32 " => offset = %" PRId32 "\n",
           lw_time, time_offset);
  }
  /* return the number of bytes read */
  return len;
}
/*---------------------------------------------------------------------------*/
#ifdef PLATFORM_REBOOT
static struct ctimer reboot_timer;
static void
do_the_reboot(void *ptr)
{
  PLATFORM_REBOOT();
}
static int
reboot(lwm2m_context_t *ctx, const uint8_t *arg, size_t argsize,
       uint8_t *outbuf, size_t outsize)
{
  PRINTF("Device will reboot!\n");
  ctimer_set(&reboot_timer, CLOCK_SECOND / 2, do_the_reboot, NULL);
  return 0;
}
#endif /* PLATFORM_REBOOT */
/*---------------------------------------------------------------------------*/
#ifdef PLATFORM_FACTORY_DEFAULT
static int
factory_reset(lwm2m_context_t *ctx, const uint8_t *arg, size_t arg_size,
              uint8_t *outbuf, size_t outsize)
{
  PRINTF("Device will do factory default!\n");
  PLATFORM_FACTORY_DEFAULT();
  return 0;
}
#endif /* PLATFORM_FACTORY_DEFAULT */
/*---------------------------------------------------------------------------*/
LWM2M_RESOURCES(device_resources,
#ifdef LWM2M_DEVICE_MANUFACTURER
                LWM2M_RESOURCE_STRING(0, LWM2M_DEVICE_MANUFACTURER),
#endif /* LWM2M_DEVICE_MANUFACTURER */
#ifdef LWM2M_DEVICE_TYPE
                LWM2M_RESOURCE_STRING(17, LWM2M_DEVICE_TYPE),
#endif /* LWM2M_DEVICE_TYPE */
#ifdef LWM2M_DEVICE_MODEL_NUMBER
                LWM2M_RESOURCE_STRING(1, LWM2M_DEVICE_MODEL_NUMBER),
#endif /* LWM2M_DEVICE_MODEL_NUMBER */
#ifdef LWM2M_DEVICE_SERIAL_NO
                LWM2M_RESOURCE_STRING(2, LWM2M_DEVICE_SERIAL_NO),
#endif /* LWM2M_DEVICE_SERIAL_NO */
#ifdef LWM2M_DEVICE_FIRMWARE_VERSION
                LWM2M_RESOURCE_STRING(3, LWM2M_DEVICE_FIRMWARE_VERSION),
#endif /* LWM2M_DEVICE_FIRMWARE_VERSION */
#ifdef PLATFORM_REBOOT
                LWM2M_RESOURCE_CALLBACK(4, { NULL, NULL, reboot }),
#endif /* PLATFORM_REBOOT */
#ifdef PLATFORM_FACTORY_DEFAULT
                LWM2M_RESOURCE_CALLBACK(5, { NULL, NULL, factory_reset }),
#endif /* PLATFORM_FACTORY_DEFAULT */
                /* Current Time */
                LWM2M_RESOURCE_CALLBACK(13, { read_lwtime, set_lwtime, NULL }),
                );
LWM2M_INSTANCES(device_instances, LWM2M_INSTANCE(0, device_resources));
LWM2M_OBJECT(device, 3, device_instances);
/*---------------------------------------------------------------------------*/
void
lwm2m_device_init(void)
{
  /**
   * Register this device and its handlers - the handlers
   * automatically sends in the object to handle.
   */
  PRINTF("*** Init lwm2m-device\n");
  lwm2m_engine_register_object(&device);
}
/*---------------------------------------------------------------------------*/
/** @} */
