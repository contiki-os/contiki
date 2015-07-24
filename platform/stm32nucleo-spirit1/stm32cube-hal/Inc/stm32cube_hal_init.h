/**
******************************************************************************
* @file    stm32cube_hal_init.h
* @author  MCD Application Team
* @version V1.0.0
* @date    18-February-2014 
* @brief   This file contains all the functions prototypes for the 
*          stm32cube_hal_init.c file.
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
*
* Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
* You may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*        http://www.st.com/software_license_agreement_liberty_v2
*
* Unless required by applicable law or agreed to in writing, software 
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32CUBE_HAL_INIT_H
#define __STM32CUBE_HAL_INIT_H

#ifdef __cplusplus
extern "C" {
#endif
  
  /* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "stm32l1xx_nucleo.h"
#include "platform-conf.h"  

#if COMPILE_SENSORS
#include "x_nucleo_iks01a1_pressure.h"
#include "x_nucleo_iks01a1_imu_6axes.h"
#include "x_nucleo_iks01a1_magneto.h"
#include "x_nucleo_iks01a1_hum_temp.h"  
#endif /*COMPILE_SENSORS*/

  
  /* Exported types ------------------------------------------------------------*/
  /* Exported constants --------------------------------------------------------*/ 
  
  /* Uncomment to enable the adaquate RTC Clock Source */

#define RTC_CLOCK_SOURCE_LSI


#ifdef RTC_CLOCK_SOURCE_LSI
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x0130
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x00FF
#endif  
  
  
  
  /* Exported macro ------------------------------------------------------------*/
  
  /* Exported functions ------------------------------------------------------- */

void stm32cube_hal_init();
void RTC_TimeRegulate(uint8_t hh, uint8_t mm, uint8_t ss);

#ifdef __cplusplus
}
#endif



#endif /* __STM32CUBE_HAL_INIT */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

