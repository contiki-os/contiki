/*
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *      CoAP resource to start/stop/configure BLE advertisements
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "rf-core/rf-ble.h"

#include <string.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define BLE_NAME_BUF_LEN 32
/*---------------------------------------------------------------------------*/
const char *forbidden_payload = "Name to advertise unspecified.\n"
                                "Use name=<name> in the request";
/*---------------------------------------------------------------------------*/
static void
res_ble_post_put_handler(void *request, void *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *text = NULL;
  char name[BLE_NAME_BUF_LEN];
  int success = 0;
  int rv;

  memset(name, 0, BLE_NAME_BUF_LEN);

  len = REST.get_post_variable(request, "name", &text);

  if(len > 0 && len < BLE_NAME_BUF_LEN) {
    memcpy(name, text, len);
    rf_ble_beacond_config(0, name);
    success = 1;
  }

  len = REST.get_post_variable(request, "interval", &text);

  rv = atoi(text);

  if(rv > 0) {
    rf_ble_beacond_config((clock_time_t)(rv * CLOCK_SECOND), NULL);
    success = 1;
  }

  len = REST.get_post_variable(request, "mode", &text);

  if(len) {
    if(strncmp(text, "on", len) == 0) {
      if(rf_ble_beacond_start()) {
        success = 1;
      } else {
        REST.set_response_status(response, REST.status.FORBIDDEN);
        REST.set_response_payload(response, forbidden_payload,
                                  strlen(forbidden_payload));
        return;
      }
    } else if(strncmp(text, "off", len) == 0) {
      rf_ble_beacond_stop();
      success = 1;
    } else {
      success = 0;
    }
  }

  if(!success) {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_ble_advd,
         "title=\"BLE advd config: POST/PUT name=<name>&mode=on|off"
         "&interval=<secs>\";rt=\"Control\"",
         NULL,
         res_ble_post_put_handler,
         res_ble_post_put_handler,
         NULL);
/*---------------------------------------------------------------------------*/
/** @} */
