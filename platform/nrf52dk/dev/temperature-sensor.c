/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 */

/**
 * \addtogroup nrf52dk-devices Device drivers
 * @{
 *
 * \addtogroup nrf52dk-devices-temp Temperature sensor driver
 * This is a driver for nRF52832 hardware sensor.
 *
 * @{
 *
 * \file
 *         Temperature sensor implementation.
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 *
 */
#ifndef SOFTDEVICE_PRESENT
#include "nrf_temp.h"
#else
#include "nrf_soc.h"
#endif
#include "contiki.h"
#include "dev/temperature-sensor.h"


const struct sensors_sensor temperature_sensor;

/*---------------------------------------------------------------------------*/
/**
 * \brief Returns device temperature
 * \param type ignored
 * \return Device temperature in degrees Celsius
 */
static int
value(int type)
{
#ifndef SOFTDEVICE_PRESENT
  return nrf_temp_read();
#else
  int32_t temp;
  sd_temp_get(&temp);
  return temp >> 2;
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configures temperature sensor
 * \param type initializes the hardware sensor when \a type is set to
 *             \a SENSORS_HW_INIT
 * \param c ignored
 * \return 1
 * \note  This function does nothing when SoftDevice is present
 */
static int
configure(int type, int c)
{
#ifndef SOFTDEVICE_PRESENT
  if (type == SENSORS_HW_INIT) {
    nrf_temp_init();
  }
#endif
  return 1;
}
/**
 * \brief Return temperature sensor status
 * \param type ignored
 * \return 1
 */
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(temperature_sensor, TEMPERATURE_SENSOR, value, configure, status);
/**
 * @}
 * @}
 */
