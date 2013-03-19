/**
 * \addtogroup mb851-platform
 *
 * @{
 */
/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
* \file
*			Temperature sensor.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/

#include PLATFORM_HEADER
#include BOARD_HEADER
#include "hal/error.h"
#include "hal/hal.h"
#include "micro/adc.h"

#include "dev/temperature-sensor.h"

#undef  TEMPERATURE_SENSOR_GPIO
#define TEMPERATURE_SENSOR_GPIO  PORTB_PIN(7)

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  halGpioConfig(TEMPERATURE_SENSOR_GPIO, GPIOCFG_ANALOG);
  halInternalInitAdc();
  halAdcSetRange(TRUE);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  static uint16_t ADCvalue;
  static int16_t volts;
  halStartAdcConversion(ADC_USER_APP, ADC_REF_INT, ADC_SOURCE_ADC2_VREF2,
                        ADC_CONVERSION_TIME_US_4096);

  /* This blocks for a while, about 4ms. */
  halReadAdcBlocking(ADC_USER_APP, &ADCvalue);

  /* 100 uVolts */
  volts = halConvertValueToVolts(ADCvalue);

  /* +- 0.23 degC in the range (-10;65) degC */
  /*  return ((18641 - (int32_t)volts)*100)/1171;*/

  /* +- 0.004 degC in the range (20;30) degC */
  return ((18663 - (int32_t) volts) * 100) / 1169;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch (type) {
  case SENSORS_HW_INIT:
    init();
    return 1;
  case SENSORS_ACTIVE:
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch (type) {
  case SENSORS_READY:
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(temperature_sensor, TEMPERATURE_SENSOR,
               value, configure, status);
/*--------------------------------------------------------------------------*/
/** @} */
