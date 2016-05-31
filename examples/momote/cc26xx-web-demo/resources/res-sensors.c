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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *  CoAP resource handler for the Sensortag-CC26xx sensors
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "cc26xx-web-demo.h"
#include "coap-server.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/*
 * Generic resource handler for any sensor in this example. Ultimately gets
 * called by all handlers and populates the CoAP response
 */
static void
res_get_handler_all(int sens_type, void *request, void *response,
                    uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  const cc26xx_web_demo_sensor_reading_t *reading;

  reading = cc26xx_web_demo_sensor_lookup(sens_type);

  if(reading == NULL) {
    REST.set_response_status(response, REST.status.NOT_FOUND);
    REST.set_response_payload(response, coap_server_not_found_msg,
                              strlen(coap_server_not_found_msg));
    return;
  }

  REST.get_header_accept(request, &accept);

  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", reading->converted);

    REST.set_response_payload(response, (uint8_t *)buffer,
                              strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{\"%s\":%s}",
             reading->descr, reading->converted);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_XML) {
    REST.set_header_content_type(response, REST.type.APPLICATION_XML);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
             "<%s val=\"%s\" unit=\"%s\"/>", reading->xml_element,
             reading->converted, reading->units);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, coap_server_supported_msg,
                              strlen(coap_server_supported_msg));
  }
}
/*---------------------------------------------------------------------------*/
/* BatMon resources and handler: Temperature, Voltage */
static void
res_get_handler_batmon_temp(void *request, void *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_BATMON_TEMP, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_batmon_volt(void *request, void *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_BATMON_VOLT, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_batmon_temp, "title=\"Battery Temp\";rt=\"C\"",
         res_get_handler_batmon_temp, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
RESOURCE(res_batmon_volt, "title=\"Battery Voltage\";rt=\"mV\"",
         res_get_handler_batmon_volt, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
#if BOARD_SENSORTAG
/*---------------------------------------------------------------------------*/
/* MPU resources and handler: Accelerometer and Gyro */
static void
res_get_handler_mpu_acc_x(void *request, void *response, uint8_t *buffer,
                          uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_MPU_ACC_X, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_mpu_acc_y(void *request, void *response, uint8_t *buffer,
                          uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_MPU_ACC_Y, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_mpu_acc_z(void *request, void *response, uint8_t *buffer,
                          uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_MPU_ACC_Z, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_mpu_gyro_x(void *request, void *response, uint8_t *buffer,
                           uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_MPU_GYRO_X, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_mpu_gyro_y(void *request, void *response, uint8_t *buffer,
                           uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_MPU_GYRO_Y, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_mpu_gyro_z(void *request, void *response, uint8_t *buffer,
                           uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_MPU_GYRO_Z, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_mpu_acc_x, "title=\"Acc X\";rt=\"G\"", res_get_handler_mpu_acc_x,
         NULL, NULL, NULL);
RESOURCE(res_mpu_acc_y, "title=\"Acc Y\";rt=\"G\"", res_get_handler_mpu_acc_y,
         NULL, NULL, NULL);
RESOURCE(res_mpu_acc_z, "title=\"Acc Z\";rt=\"G\"", res_get_handler_mpu_acc_z,
         NULL, NULL, NULL);

RESOURCE(res_mpu_gyro_x, "title=\"Gyro X\";rt=\"deg/sec\"",
         res_get_handler_mpu_gyro_x, NULL, NULL, NULL);
RESOURCE(res_mpu_gyro_y, "title=\"Gyro Y\";rt=\"deg/sec\"",
         res_get_handler_mpu_gyro_y, NULL, NULL, NULL);
RESOURCE(res_mpu_gyro_z, "title=\"Gyro Z\";rt=\"deg/sec\"",
         res_get_handler_mpu_gyro_z, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
/* TMP sensor resources and handlers: Object, Ambient */
static void
res_get_handler_obj_temp(void *request, void *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_TMP_OBJECT, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_amb_temp(void *request, void *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_TMP_AMBIENT, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_tmp007_obj, "title=\"Temperature (Object)\";rt=\"C\"",
         res_get_handler_obj_temp, NULL, NULL, NULL);

RESOURCE(res_tmp007_amb, "title=\"Temperature (Ambient)\";rt=\"C\"",
         res_get_handler_amb_temp, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
/* BMP sensor resources: Temperature, Pressure */
static void
res_get_handler_bmp_temp(void *request, void *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_BMP_TEMP, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_bmp_press(void *request, void *response, uint8_t *buffer,
                          uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_BMP_PRES, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_bmp280_temp, "title=\"Barometer (Temperature)\";rt=\"C\"",
         res_get_handler_bmp_temp, NULL, NULL, NULL);

RESOURCE(res_bmp280_press,
         "title=\"Barometer (Pressure)\";rt=\"hPa (hectopascal / millibar)\"",
         res_get_handler_bmp_press, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
/* HDC1000 sensor resources and handler: Temperature, Pressure */
static void
res_get_handler_hdc_temp(void *request, void *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_HDC_TEMP, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_hdc_humidity(void *request, void *response, uint8_t *buffer,
                             uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_HDC_HUMIDITY, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_hdc1000_temp, "title=\"Temperature\";rt=\"C\"",
         res_get_handler_hdc_temp, NULL, NULL, NULL);

RESOURCE(res_hdc1000_hum, "title=\"Humidity\";rt=\"%RH\"",
         res_get_handler_hdc_humidity, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
/* Illuminance resources and handler */
static void
res_get_handler_opt(void *request, void *response, uint8_t *buffer,
                    uint16_t preferred_size, int32_t *offset)
{
  res_get_handler_all(CC26XX_WEB_DEMO_SENSOR_OPT_LIGHT, request, response,
                      buffer, preferred_size, offset);
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_opt3001_light, "title=\"Illuminance\";rt=\"Lux\"",
         res_get_handler_opt, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
#endif /* BOARD_SENSORTAG */
/*---------------------------------------------------------------------------*/
/** @} */
