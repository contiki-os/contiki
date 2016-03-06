/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
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
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *     A CC26XX-specific CoAP server
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "board-peripherals.h"
#include "rf-core/rf-ble.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/* Common resources */
extern resource_t res_leds;

extern resource_t res_batmon_temp;
extern resource_t res_batmon_volt;

extern resource_t res_device_sw;
extern resource_t res_device_hw;
extern resource_t res_device_uptime;
extern resource_t res_device_cfg_reset;

extern resource_t res_parent_rssi;
extern resource_t res_parent_ip;

#if RF_BLE_ENABLED
extern resource_t res_ble_advd;
#endif

/* Board-specific resources */
#if BOARD_SENSORTAG
extern resource_t res_bmp280_temp;
extern resource_t res_bmp280_press;
extern resource_t res_tmp007_amb;
extern resource_t res_tmp007_obj;
extern resource_t res_hdc1000_temp;
extern resource_t res_hdc1000_hum;
extern resource_t res_opt3001_light;
extern resource_t res_mpu_acc_x;
extern resource_t res_mpu_acc_y;
extern resource_t res_mpu_acc_z;
extern resource_t res_mpu_gyro_x;
extern resource_t res_mpu_gyro_y;
extern resource_t res_mpu_gyro_z;
extern resource_t res_toggle_red;
extern resource_t res_toggle_green;
#else
extern resource_t res_toggle_red;
extern resource_t res_toggle_green;
extern resource_t res_toggle_orange;
extern resource_t res_toggle_yellow;
#endif
/*---------------------------------------------------------------------------*/
const char *coap_server_not_found_msg = "Resource not found";
const char *coap_server_supported_msg = "Supported:"
                                        "text/plain,"
                                        "application/json,"
                                        "application/xml";
/*---------------------------------------------------------------------------*/
static void
start_board_resources(void)
{
#if BOARD_SENSORTAG
  rest_activate_resource(&res_bmp280_temp, "sen/bar/temp");
  rest_activate_resource(&res_bmp280_press, "sen/bar/pres");
  rest_activate_resource(&res_tmp007_amb, "sen/tmp/amb");
  rest_activate_resource(&res_tmp007_obj, "sen/tmp/obj");
  rest_activate_resource(&res_hdc1000_temp, "sen/hdc/t");
  rest_activate_resource(&res_hdc1000_hum, "sen/hdc/h");
  rest_activate_resource(&res_opt3001_light, "sen/opt/light");
  rest_activate_resource(&res_mpu_acc_x, "sen/mpu/acc/x");
  rest_activate_resource(&res_mpu_acc_y, "sen/mpu/acc/y");
  rest_activate_resource(&res_mpu_acc_z, "sen/mpu/acc/z");
  rest_activate_resource(&res_mpu_gyro_x, "sen/mpu/gyro/x");
  rest_activate_resource(&res_mpu_gyro_y, "sen/mpu/gyro/y");
  rest_activate_resource(&res_mpu_gyro_z, "sen/mpu/gyro/z");
  rest_activate_resource(&res_leds, "lt");
  rest_activate_resource(&res_toggle_green, "lt/g");
  rest_activate_resource(&res_toggle_red, "lt/r");
#elif BOARD_SMARTRF06EB
  rest_activate_resource(&res_leds, "lt");
  rest_activate_resource(&res_toggle_red, "lt/r");
  rest_activate_resource(&res_toggle_yellow, "lt/y");
  rest_activate_resource(&res_toggle_green, "lt/g");
  rest_activate_resource(&res_toggle_orange, "lt/o");
#endif
}
/*---------------------------------------------------------------------------*/
PROCESS(coap_server_process, "CC26XX CoAP Server");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_server_process, ev, data)
{
  PROCESS_BEGIN();

  printf("CC26XX CoAP Server\n");

  /* Initialize the REST engine. */
  rest_init_engine();

  rest_activate_resource(&res_batmon_temp, "sen/batmon/temp");
  rest_activate_resource(&res_batmon_volt, "sen/batmon/voltage");

  rest_activate_resource(&res_device_hw, "dev/mdl/hw");
  rest_activate_resource(&res_device_sw, "dev/mdl/sw");
  rest_activate_resource(&res_device_uptime, "dev/uptime");
  rest_activate_resource(&res_device_cfg_reset, "dev/cfg_reset");

  rest_activate_resource(&res_parent_rssi, "net/parent/RSSI");
  rest_activate_resource(&res_parent_ip, "net/parent/IPv6");

#if RF_BLE_ENABLED
  rest_activate_resource(&res_ble_advd, "dev/ble_advd");
#endif

  start_board_resources();

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
