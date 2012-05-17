/*
 * Copyright (c) 2013, Robert Quattlebaum.
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
 *
 */

/**
 * @file   settings-example.c
 * @brief  Contiki example showing how to use the settings manager.
 * @author Robert Quattlebaum <darco@deepdarc.com>
 */

#include "contiki.h"
#include "lib/settings.h"

#include <stdio.h>              /* For printf() */

/*---------------------------------------------------------------------------*/
PROCESS(settings_example_process, "Settings Example Process");
AUTOSTART_PROCESSES(&settings_example_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(settings_example_process, ev, data)
{
  int i;
  settings_status_t status;
  settings_iter_t iter;
  char hostname[30];
  uint16_t panid;
  uint16_t channel;

  PROCESS_BEGIN();

  /*************************************************************************/
  /* Basic setting of parameters */

  status = settings_set_uint16(SETTINGS_KEY_PAN_ID, 0xABCD);
  if(SETTINGS_STATUS_OK != status) {
    printf("settings-example: `set` failed: %d\n", status);
  }

  status = settings_set_uint8(SETTINGS_KEY_CHANNEL, 26);
  if(SETTINGS_STATUS_OK != status) {
    printf("settings-example: `set` failed: %d\n", status);
  }

  status = settings_set_cstr(SETTINGS_KEY_HOSTNAME, "contiki.local");
  if(SETTINGS_STATUS_OK != status) {
    printf("settings-example: `set` failed: %d\n", status);
  }

  /*************************************************************************/
  /* Basic getting of parameters */

  panid = settings_get_uint16(SETTINGS_KEY_PAN_ID, 0);
  if(0xABCD != panid) {
    printf("settings-example: `get` failed: value mismatch.\n");
  }

  channel = settings_get_uint16(SETTINGS_KEY_CHANNEL, 0);
  if(26 != channel) {
    printf("settings-example: `get` failed: value mismatch.\n");
  }

  if(!settings_get_cstr(SETTINGS_KEY_HOSTNAME, 0, hostname, sizeof(hostname))) {
    printf("settings-example: `get` failed: settings_get_cstr returned NULL\n");
  } else if(strcmp(hostname, "contiki.local") != 0) {
    printf("settings-example: `get` failed: value mismatch.\n");
  }

  /*************************************************************************/
  /* Adding multiple values with the same key */

  for(i = 0; i < 10; i++) {
    settings_add_uint8(TCC('e','x'), i + 20);
  }

  /*************************************************************************/
  /* Reading multiple values with the same key */

  for(i = 0; i < 10; i++) {
    if(settings_get_uint8(TCC('e', 'x'), i) != i + 20) {
      printf("settings-example: `get` failed: value mismatch.\n");
    }
  }

  /*************************************************************************/
  /* Iterating thru all settings */

  for(iter = settings_iter_begin(); iter; iter = settings_iter_next(iter)) {
    settings_length_t len = settings_iter_get_value_length(iter);
    eeprom_addr_t addr = settings_iter_get_value_addr(iter);
    uint8_t byte;

    union {
      settings_key_t key;
      char bytes[0];
    } u;

    u.key = settings_iter_get_key(iter);

    if(u.bytes[0] >= 32 && u.bytes[0] < 127
       && u.bytes[1] >= 32 && u.bytes[1] < 127
    ) {
      printf("settings-example: [%c%c] = <",u.bytes[0],u.bytes[1]);
    } else {
      printf("settings-example: <0x%04X> = <",u.key);
    }

    for(; len; len--, addr++) {
      eeprom_read(addr, &byte, 1);
      printf("%02X", byte);
      if(len != 1) {
        printf(" ");
      }
    }

    printf(">\n");
  }

  printf("settings-example: Done.\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
