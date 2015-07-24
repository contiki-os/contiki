/**
 ******************************************************************************
 * @file    x_nucleo_iks01a1_pressure.h
 * @author  CL
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file contains definitions for x_nucleo_iks01a1_pressure.c
 *          firmware driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __X_NUCLEO_IKS01A1_PRESSURE_H
#define __X_NUCLEO_IKS01A1_PRESSURE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "x_nucleo_iks01a1.h"
/* Include Pressure sensor component driver */
#include "lps25h.h"
#include "lps25hb.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1_PRESSURE
 * @{
 */

/** @defgroup X_NUCLEO_IKS01A1_PRESSURE_Exported_Functions X_NUCLEO_IKS01A1_PRESSURE_Exported_Functions
 * @{
 */
/* Sensor Configuration Functions */
PRESSURE_StatusTypeDef BSP_PRESSURE_Init(void);
uint8_t BSP_PRESSURE_isInitialized(void);
PRESSURE_StatusTypeDef BSP_PRESSURE_Reset(void);
PRESSURE_StatusTypeDef BSP_PRESSURE_ReadID(uint8_t *p_id);
PRESSURE_StatusTypeDef BSP_PRESSURE_CheckID(void);
PRESSURE_StatusTypeDef BSP_PRESSURE_GetPressure(float* pfData);
PRESSURE_StatusTypeDef BSP_PRESSURE_GetTemperature(float* pfData);
PRESSURE_ComponentTypeDef BSP_PRESSURE_GetComponentType(void);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __X_NUCLEO_IKS01A1_PRESSURE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
