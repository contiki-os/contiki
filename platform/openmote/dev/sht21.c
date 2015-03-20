/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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
 * \addtogroup platform
 * @{
 *
 * \defgroup openmote The OpenMote Platform
 *
 * \file
 * Driver for the SHT21 temperature and humidity sensor in OpenMote-CC2538.
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */

/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "dev/i2c.h"
#include "dev/sht21.h"
/*---------------------------------------------------------------------------*/
#define SHT21_ADDRESS                   (0x40)

#define SHT21_USER_REG_READ             (0xE7)
#define SHT21_USER_REG_WRITE            (0xE6)
#define SHT21_USER_REG_RESERVED_BITS    (0x38)

#define SHT21_RESOLUTION_12b_14b        ((0 << 7) | (0 << 0))
#define SHT21_RESOLUTION_8b_12b         ((0 << 7) | (1 << 0))
#define SHT21_RESOLUTION_10b_13b        ((1 << 7) | (0 << 0))
#define SHT21_RESOLUTION_11b_11b        ((1 << 7) | (1 << 0))
#define SHT21_BATTERY_ABOVE_2V25        (0 << 6)
#define SHT21_BATTERY_BELOW_2V25        (1 << 6)
#define SHT21_ONCHIP_HEATER_ENABLE      (1 << 2)
#define SHT21_ONCHIP_HEATER_DISABLE     (0 << 2)
#define SHT21_OTP_RELOAD_ENABLE         (0 << 1)
#define SHT21_OTP_RELOAD_DISABLE        (1 << 1)

#define SHT21_TEMPERATURE_HM_CMD        (0xE3)
#define SHT21_HUMIDITY_HM_CMD           (0xE5)
#define SHT21_TEMPERATURE_NHM_CMD       (0xF3)
#define SHT21_HUMIDITY_NHM_CMD          (0xF5)
#define SHT21_RESET_CMD                 (0xFE)

#define SHT21_STATUS_MASK               ( 0xFC )

#define SHT21_DEFAULT_CONFIG            (SHT21_RESOLUTION_12b_14b | \
                                         SHT21_ONCHIP_HEATER_DISABLE | \
                                         SHT21_BATTERY_ABOVE_2V25 | \
                                         SHT21_OTP_RELOAD_DISABLE)

#define SHT21_USER_CONFIG               (SHT21_RESOLUTION_8b_12b | \
                                         SHT21_ONCHIP_HEATER_DISABLE | \
                                         SHT21_BATTERY_ABOVE_2V25 | \
                                         SHT21_OTP_RELOAD_DISABLE)
/*---------------------------------------------------------------------------*/
//const struct sensors_sensor temp_sensor, humidity_sensor;
SENSORS_SENSOR(temp_sensor, "Temp Sensor", sht21_value, sht21_config, sht21_status);
SENSORS_SENSOR(humidity_sensor, "Humidity Sensor", sht21_value, sht21_config, sht21_status);
/*---------------------------------------------------------------------------*/
/**
 *
 */
int sht21_value(int type)
{
  switch(type) {
    case SHT21_TEMP_VAL :
      return (int)sht21_read_temperature();
    case SHT21_HUMIDITY_VAL :
      return (int)sht21_read_humidity();
  }
  return 0;
}
/**
 *
 */
int sht21_config(int type, int value)
{
  switch(type) {
    case SENSORS_HW_INIT :
      sht21_init();
      return 0;
    case SENSORS_ACTIVE :
      return 0;
    case SENSORS_CONFIG :
      switch(value) {
        case 0 :
          sht21_set_config(SHT21_DEFAULT_CONFIG);
          break;
        case 1 :
          sht21_set_config(SHT21_USER_CONFIG);
          break;
        default :
          sht21_set_config(value);
          break;
      }
      break;
  }
  return 0;
}
/**
 *
 */
int sht21_status(int type){
  return (int)sht21_is_present();
}
/**
 *
 */
void sht21_set_config(uint8_t config)
{
  uint8_t data[2];

  /* Setup the configuration vector, the first position holds address */
  /* and the second position holds the actual configuration */
  data[0] = SHT21_USER_REG_WRITE;
  data[1] = 0;

  /* Read the current configuration according to the datasheet (pag. 9, fig. 18) */
  i2c_single_send(SHT21_ADDRESS, SHT21_USER_REG_READ);
  i2c_single_receive(SHT21_ADDRESS, &data[1]);

  /* Clean all the configuration bits except those reserved */
  data[1] &= SHT21_USER_REG_RESERVED_BITS;

  /* Set the configuration bits without changing those reserved */
  data[1] |= config;

  i2c_burst_send(SHT21_ADDRESS, data, sizeof(data));
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
void
sht21_init(void)
{
  sht21_set_config(SHT21_USER_CONFIG);
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
void
sht21_reset(void)
{
  /* Send a soft-reset command according to the datasheet (pag. 9, fig. 17) */
  i2c_single_send(SHT21_ADDRESS, SHT21_RESET_CMD);
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint8_t
sht21_is_present(void)
{
  uint8_t is_present;

  /* Read the current configuration according to the datasheet (pag. 9, fig. 18) */
  i2c_single_send(SHT21_ADDRESS, SHT21_USER_REG_READ);
  i2c_single_receive(SHT21_ADDRESS, &is_present);

  /* Clear the reserved bits according to the datasheet (pag. 9, tab. 8) */
  is_present &= ~SHT21_USER_REG_RESERVED_BITS;

  return is_present == SHT21_DEFAULT_CONFIG;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint16_t
sht21_read_temperature(void)
{
  uint8_t sht21_temperature[2];
  uint16_t temperature;

  /* Read the current temperature according to the datasheet (pag. 8, fig. 15) */
  i2c_single_send(SHT21_ADDRESS, SHT21_TEMPERATURE_HM_CMD);
  i2c_burst_receive(SHT21_ADDRESS, sht21_temperature, sizeof(sht21_temperature));

  temperature = (sht21_temperature[0] << 8) | (sht21_temperature[1] & SHT21_STATUS_MASK);

  return temperature;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
float
sht21_convert_temperature(uint16_t temperature)
{
  float result;

  result = -46.85;
  result += 175.72*(float)temperature/65536.0;

  return result;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
uint16_t
sht21_read_humidity(void)
{
  uint8_t sht21_humidity[2];
  uint16_t humidity;

  /* Read the current humidity according to the datasheet (pag. 8, fig. 15) */
  i2c_single_send(SHT21_ADDRESS, SHT21_HUMIDITY_HM_CMD);
  i2c_burst_receive(SHT21_ADDRESS, sht21_humidity, sizeof(sht21_humidity));

  humidity = (sht21_humidity[0] << 8) | (sht21_humidity[1] & SHT21_STATUS_MASK);

  return humidity;
}
/*---------------------------------------------------------------------------*/
/**
 *
 */
float
sht21_convert_humidity(uint16_t humidity)
{
  float result;

  result = -6.0;
  result += 125.0*(float)humidity/65536.0;

  return result;
}
/*---------------------------------------------------------------------------*/
/** @} */
