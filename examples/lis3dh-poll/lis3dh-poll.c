/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#define NUM_ACC_AXES 3
#define EXAMPLE_POLL_FREQUENCY 8

#include "contiki.h"

/* Accelerometer driver */
#include "lis3dh.h"

#include <stdio.h>              /* For printf() */
#include <stdint.h>             /* int16_t et al. */
/*---------------------------------------------------------------------------*/
PROCESS(lis3dh_poll_process, "LIS3DH poller process");
AUTOSTART_PROCESSES(&lis3dh_poll_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(lis3dh_poll_process, ev, data)
{
  static struct etimer et;
  int16_t acc_buffer[NUM_ACC_AXES];
  int16_t temperature;

  PROCESS_BEGIN();

  printf("\nLIS3DH test\n");

  /* initialize the LIS3DH hardware */
  printf("Initialize LIS3DH\n");
  lis3dh_init();
  /* Turn on sensor and set the sampling frequency of the accelerometer */
  printf("Set ODR\n");
  lis3dh_set_odr(ODR_100Hz);
  /* Set scaling option to +/- 2g */
  printf("Set scale\n");
  lis3dh_set_scale(SCALE_2G);
  printf("Set axes XYZ\n");
  lis3dh_set_axes(LIS3DH_AXES_XYZ);
  printf("Disable FIFO\n");
  lis3dh_set_fifo(0);
  printf("Enable temperature reading\n");
  lis3dh_set_aux_adc(1, 1);
  printf("LIS3DH init done\n");

  /* Poll at 8Hz */
  etimer_set(&et, CLOCK_SECOND / EXAMPLE_POLL_FREQUENCY);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Reset the etimer to trig again */
    etimer_reset(&et);

    /* Read accelerometer values */
    lis3dh_read_xyz(&acc_buffer[0]);
    temperature = lis3dh_read_aux_adc3();

    printf("X: %d\tY: %d\tZ: %d\tTemperature: %d\n", (unsigned int)acc_buffer[0],
           (unsigned int)acc_buffer[1], (unsigned int)acc_buffer[2],
           (unsigned int)temperature);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
