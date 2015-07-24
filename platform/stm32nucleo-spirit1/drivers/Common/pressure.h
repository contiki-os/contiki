/**
 ******************************************************************************
 * @file    pressure.h
 * @author  MEMS Application Team
 * @version V1.2.0
 * @date    28-January-2015
 * @brief   This header file contains the functions prototypes for the
 *          pressure driver.
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
#ifndef __PRESSURE_H
#define __PRESSURE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup PRESSURE
  * @{
  */

/** @defgroup PRESSURE_Exported_Types
  * @{
  */

/**
 * @brief  PRESSURE init structure definition
 */
typedef struct
{
  uint8_t OutputDataRate;
  uint8_t PressureResolution;
  uint8_t TemperatureResolution;
  uint8_t DiffEnable;
  uint8_t BlockDataUpdate;
  uint8_t SPIMode;
} PRESSURE_InitTypeDef;

/**
 * @brief  PRESSURE status enumerator definition
 */
typedef enum
{
  PRESSURE_OK = 0,
  PRESSURE_ERROR = 1,
  PRESSURE_TIMEOUT = 2,
  PRESSURE_NOT_IMPLEMENTED = 3
} PRESSURE_StatusTypeDef;

/**
 * @brief  PRESSURE component id enumerator definition
 */
typedef enum
{
  PRESSURE_NONE_COMPONENT = 0,
  PRESSURE_LPS25H_COMPONENT = 1,
  PRESSURE_LPS25HB_DIL24_COMPONENT = 2
} PRESSURE_ComponentTypeDef;

/**
 * @brief  PRESSURE driver extended structure definition
 */
typedef struct
{
  PRESSURE_ComponentTypeDef
  id; /* This id must be unique for each component belonging to this class that wants to extend common class */
  void *pData; /* This pointer is specific for each component */
} PRESSURE_DrvExtTypeDef;

/**
 * @brief  PRESSURE driver structure definition
 */
typedef struct
{
  PRESSURE_StatusTypeDef       (*Init)(PRESSURE_InitTypeDef *);
  PRESSURE_StatusTypeDef       (*PowerOff)(void);
  PRESSURE_StatusTypeDef       (*ReadID)(uint8_t *);
  PRESSURE_StatusTypeDef       (*Reset)(void);
  void                         (*ConfigIT)(uint16_t);
  void                         (*EnableIT)(uint8_t);
  void                         (*DisableIT)(uint8_t);
  uint8_t                      (*ITStatus)(uint16_t, uint16_t);
  void                         (*ClearIT)(uint16_t, uint16_t);
  PRESSURE_StatusTypeDef       (*GetPressure)(float *);
  PRESSURE_StatusTypeDef       (*GetTemperature)(float *);
  void                         (*SlaveAddrRemap)(uint8_t);
  PRESSURE_DrvExtTypeDef       *extData;
} PRESSURE_DrvTypeDef;

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

#endif /* __PRESSURE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
