/** @file temperature_sensor.h
 * @brief Header for temperature sensor driver 
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#ifndef _TEMP_SENSOR_H_
#define _TEMP_SENSOR_H_

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
int32u temperatureSensor_GetValue(void);

#endif /* _TEMP_SENSOR_H_ */

