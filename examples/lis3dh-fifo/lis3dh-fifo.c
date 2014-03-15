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
#define NUM_ACC_SAMPLES 100
#define EXAMPLE_POLL_FREQUENCY 4

#include "contiki.h"

/* Accelerometer driver */
#include "lis3dh.h"

#include <stdio.h>              /* For printf() */
#include <stdint.h>             /* int16_t et al. */

/* Statistics */
#include "statistics.h"

/*---------------------------------------------------------------------------*/
PROCESS(lis3dh_poll_process, "LIS3DH poller process");
AUTOSTART_PROCESSES(&lis3dh_poll_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(lis3dh_poll_process, ev, data)
{
  static struct etimer et;
  static int16_t acc_buffer[NUM_ACC_AXES * NUM_ACC_SAMPLES];
  static size_t acc_buffer_count = 0;
  stats_3d_t stats;
  uint8_t fifo_src;
  uint8_t fifo_count;
  uint8_t status_reg;

  PROCESS_BEGIN();

  printf("\nLIS3DH test\n");

  /* initialize the LIS3DH hardware */
  printf("Initialize LIS3DH\n");
  lis3dh_init();
  lis3dh_set_fifo(0);
  /* Turn on sensor and set the sampling frequency of the accelerometer */
  printf("Set ODR\n");
  lis3dh_set_odr(ODR_100Hz);
  /* Set scaling option to +/- 2g */
  printf("Set scale\n");
  lis3dh_set_scale(SCALE_2G);
  printf("Set axes XYZ\n");
  lis3dh_set_axes(LIS3DH_AXES_XYZ);
  printf("Disable auxiliary ADC\n");
  lis3dh_set_aux_adc(0, 0);
  printf("Enable FIFO\n");
  lis3dh_set_fifo(1);
  printf("Set FIFO mode\n");
  /* do a cycle of the FIFO mode in order to reset the FIFO in case we are
   * coming from a warm reset where the FIFO has already been filled. */
  lis3dh_set_fifo_mode(FIFO_MODE_BYPASS);
  lis3dh_set_fifo_mode(FIFO_MODE_FIFO);
  printf("LIS3DH init done\n");

  /* Poll at 8Hz */
  etimer_set(&et, CLOCK_SECOND / EXAMPLE_POLL_FREQUENCY);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("+");

    /* Reset the etimer to trig again */
    etimer_reset(&et);

    fifo_count = 0;
    fifo_src = lis3dh_read_byte(FIFO_SRC_REG);
    status_reg = lis3dh_read_byte(STATUS_REG);

    if(fifo_src & LIS3DH_FIFO_SRC_REG_EMPTY_MASK) {
      continue;
    }

    fifo_count =
      ((fifo_src & LIS3DH_FIFO_SRC_REG_FSS_MASK) >>
       LIS3DH_FIFO_SRC_REG_FSS_SHIFT);
    /* Debug */
    /*
       printf("c: %d\n", fifo_count);
       printf("FS: %x\n", fifo_src);
       printf("SR: %x\n", status_reg);
     */
    while(fifo_count > 0) {
      /* Read accelerometer values */
      lis3dh_read_xyz(&acc_buffer[acc_buffer_count * NUM_ACC_AXES]);
      /* Debug */
      /*
         printf("X: %d\tY: %d\tZ: %d\n",
         (unsigned int)(acc_buffer[acc_buffer_count * NUM_ACC_AXES + 0]),
         (unsigned int)(acc_buffer[acc_buffer_count * NUM_ACC_AXES + 1]),
         (unsigned int)(acc_buffer[acc_buffer_count * NUM_ACC_AXES + 2]));
       */

      ++acc_buffer_count;
      if(acc_buffer_count >= NUM_ACC_SAMPLES) {
        acc_buffer_count = 0;
        compute_stats_3d_int16(&acc_buffer[0], NUM_ACC_SAMPLES, &stats);
        print_stats_3d(&stats);
      }
      --fifo_count;
    }
    /* Reset FIFO */
    /*
     * This is only necessary if the FIFO was filled before we read it but there
     * is AFAICT no good way of checking if the FIFO has been stopped because
     * of an overrun. I guess we are supposed to use FIFO mode streaming instead
     * if we want to stream data.
     */
    lis3dh_set_fifo_mode(FIFO_MODE_BYPASS);
    /*
     * There is some problem with the first sample after resetting the FIFO.
     * It seems that it will keep returning old data when recently reset.
     * Avoid this bug by delaying in bypass mode for a while.
     * This is only an ugly hack for a work around, you should probably use
     * interrupts instead if you need the FIFO.
     */
    /*
       udelay(0xFFFF);
       lis3dh_read_xyz(&acc_buffer[acc_buffer_count * NUM_ACC_AXES]);
       udelay(0xFFFF);
     */
    lis3dh_set_fifo_mode(FIFO_MODE_FIFO);
    /*
       udelay(0xFFFF);
       lis3dh_read_xyz(&acc_buffer[acc_buffer_count * NUM_ACC_AXES]);
     */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
