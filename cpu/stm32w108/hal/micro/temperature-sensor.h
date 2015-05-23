/** @file cpu/stm32w108/hal/micro/temperature-sensor.h
 * @brief Header for temperature sensor driver
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_

/* Include --------------------------------------------------------------------*/
#include "hal/micro/adc.h"
/* Define --------------------------------------------------------------------*/

/* Typedef -------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/** @brief Temperature Sensor Initialization function
*/
void temperatureSensor_Init(void);

/** @brief  Get temperature sensor value
*/
uint32_t temperatureSensor_GetValue(void);

#endif /*TEMP_SENSOR_H_ */

