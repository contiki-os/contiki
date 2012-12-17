/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *
 *         Example demonstrating the flash memory functionality on
 *         sensinode N740s
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "dev/leds.h"
#include "cc2430_sfr.h"
#include "8051def.h"
#include "dev/m25p16.h"
#include "dev/n740.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#include "debug.h"
#define PRINTF(...) printf(__VA_ARGS__)
#define PUTBIN(b) putbin(b)
#else
#define PRINTF(...)
#define PUTBIN(b)
#endif

static struct m25p16_rdid id;

#define USE_SECTOR 0x10
#define MAX_READ_CHUNK 10
static uint8_t r_addr[3]; /* Read address: {USE_SECTOR, 0, 0} */
static uint8_t d_buf[MAX_READ_CHUNK];
static uint8_t rv;
static uint8_t counter;
/*---------------------------------------------------------------------------*/
PROCESS(serial_flash_process, "Serial Flash example");
AUTOSTART_PROCESSES(&serial_flash_process);
/*---------------------------------------------------------------------------*/
static void
rdsr()
{
  rv = 0;

  n740_analog_deactivate();
  rv = m25p16_rdsr();
  n740_analog_activate();

  PRINTF("RDSR: ");
  putbin(rv);
  PRINTF("\n");
}
/*---------------------------------------------------------------------------*/
static void
rdid()
{
  uint8_t i;
  memset(&id, 0, sizeof(struct m25p16_rdid));

  n740_analog_deactivate();
  m25p16_rdid(&id);
  n740_analog_activate();

  PRINTF("RDID: 0x%02x\n", id.man_id);
  PRINTF("Type: 0x%02x\n", id.mem_type);
  PRINTF("Size: 0x%02x\n", id.mem_size);
  PRINTF("ULen: 0x%02x\n", id.uid_len);
  PRINTF(" UID:");
  for(i = 0; i < id.uid_len; i++) {
    PRINTF(" %02x", id.uid[i]);
  }
  PRINTF("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(serial_flash_process, ev, data)
{
  static struct etimer et;
  uint8_t i;

  PROCESS_BEGIN();

  PRINTF("Start\n");

  memset(r_addr, 0, 3);
  r_addr[0] = USE_SECTOR;
  counter = 1;

  while(1) {

    /* Delay */
    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    leds_on(LEDS_GREEN);

    if(counter == 0) {
      n740_analog_deactivate();
      rv = m25p16_rdsr();
      n740_analog_activate();
      /* If counter==0, we started Bulk Erasing earlier. Check if we still are */
      if(rv & M25P16_SR_WIP) {
        PRINTF("Yield [%02x]\n", rv);
      } else {
        counter = 1;
      }
    }
    if(counter) {
      /*
       * Take us out of Deep Power Down - On first power-on, the device will
       * go to stand by mode (which is not DP). However, we drop to DP at the
       * end of every loop. RES must be 0x14. This is the old style signature
       * and is only still there for backward compatibility.
       */
      n740_analog_deactivate();
      rv = m25p16_res_res();
      n740_analog_activate();

      PRINTF(" RES: 0x%02x\n", rv);

      n740_analog_deactivate();
      rv = M25P16_WIP();
      n740_analog_activate();

      PRINTF("========\n");
      memset(d_buf, 0, MAX_READ_CHUNK);


      /*
       * Read Device ID: Return values must be:
       *   man_id: 0x20 (Numonyx)
       * mem_type: 0x20
       * mem_size: 0x15 (2 ^ 0x15 bytes = 2MB)
       *  uid_len: number of bytes in UID
       *      uid: Either all zeroes or a customized factory data content
       * */
      rdid();

      /* Check the value of our Status Register (SR) */
      rdsr();

      /* Enable Write: Set Bit 1 in the SR to 1 (bit WEL) */
      PRINTF("WREN\n");
      n740_analog_deactivate();
      m25p16_wren();
      n740_analog_activate();

      /* Confirm: SR & 0x02 must be 1 */
      rdsr();

      /* Disable the WEL bit */
      PRINTF("WRDI\n");
      n740_analog_deactivate();
      m25p16_wrdi();
      n740_analog_activate();

      /* Confirm: SR & 0x02 must be 0 */
      rdsr();

      /* Write something to the SR. We don't need to explicitly set WEL, wrsr()
       * will do it for us. When the cycle ends, WEL will go low */
      PRINTF("WRSR\n");
      n740_analog_deactivate();

      /* For instance, let's protect sector 31 (that's the highest one) */
      m25p16_wrsr(M25P16_SR_BP0);

      /*
       * While this is running, WEL should remain high and WIP (bit 0) should
       * also be high. When this ends, WIP and WEL will go low.
       *
       * While the write is in ongoing, we can still read the SR to check the
       * cycle's progress
       */
      while(M25P16_WIP());

      n740_analog_activate();

      /* Confirm: SR & 0x02 must be 0 */
      rdsr();

      /* Read MAX_READ_CHUNK bytes from Page 0x000000 */
      memset(d_buf, 0, MAX_READ_CHUNK);
      n740_analog_deactivate();
      m25p16_read(r_addr, d_buf, MAX_READ_CHUNK);
      n740_analog_activate();

      PRINTF("READ:");
      for(i = 0; i < MAX_READ_CHUNK; i++) {
        PRINTF(" %02x", d_buf[i]);
      }
      PRINTF("\n");

      /* Write MAX_READ_CHUNK bytes to the same Page */
      PRINTF("WRITE\n");
      for(i = 0; i < MAX_READ_CHUNK; i++) {
        d_buf[i] = i;
      }
      n740_analog_deactivate();

      /* We don't need to wren() explicitly, pp() will do that for us */
      m25p16_pp(r_addr, d_buf, MAX_READ_CHUNK);

      /* Wait for the cycle */
      while(M25P16_WIP());

      /* Trash our data buffer */
      memset(d_buf, 0, MAX_READ_CHUNK);

      PRINTF("ERASE\n");
      n740_analog_deactivate();

      /* Bulk erase every 4 loops, sector erase otherwise */

      /* Bulk Erase: This takes a few seconds so we can't really block on it.
       * It'd be a bad thing to do and the watchdog would bark anyway.
       * Bulk Erase will only be accepted if all SR_BP[2:0] == 0 */
      if((counter % 4) == 0) {
        m25p16_wrsr(0);
        while(M25P16_WIP());
        m25p16_be();
        counter = 0;
      } else {
        m25p16_se(USE_SECTOR);
        while(M25P16_WIP());
        /* Drop to Deep Power Down */
        m25p16_dp();
        counter++;
      }
      n740_analog_activate();
    }
    leds_off(LEDS_GREEN);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
