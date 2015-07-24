/**
  ******************************************************************************
  * @file    hum_temp.h
  * @author  MEMS Application Team
  * @version V1.2.0
  * @date    28-January-2015
  * @brief   This header file contains the functions prototypes for the
  *          humidity and temperature driver.
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
#ifndef __HUM_TEMP_H
#define __HUM_TEMP_H

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

/** @addtogroup HUM_TEMP
  * @{
  */

/** @defgroup HUM_TEMP_Exported_Types
  * @{
  */

/**
  * @brief  Humidity and temperature init structure definition
  */
typedef struct
{
  uint8_t Power_Mode;                         /* Power-down/Sleep/Normal Mode */
  uint8_t Data_Update_Mode;                   /* continuous update/output registers not updated until MSB and LSB reading*/
  uint8_t Reboot_Mode;                        /* Normal Mode/Reboot memory content */
  uint8_t Humidity_Resolutin;                 /* Humidity Resolution */
  uint8_t Temperature_Resolution;             /* Temperature Resolution */
  uint8_t OutputDataRate;                     /* One-shot / 1Hz / 7 Hz / 12.5 Hz */
} HUM_TEMP_InitTypeDef;

/**
  * @brief  Humidity and temperature status enumerator definition
  */
typedef enum
{
  HUM_TEMP_OK = 0,
  HUM_TEMP_ERROR = 1,
  HUM_TEMP_TIMEOUT = 2,
  HUM_TEMP_NOT_IMPLEMENTED = 3
} HUM_TEMP_StatusTypeDef;

/**
 * @brief  Humidity and temperature component id enumerator definition
 */
typedef enum
{
  HUM_TEMP_NONE_COMPONENT = 0,
  HUM_TEMP_HTS221_COMPONENT = 1
} HUM_TEMP_ComponentTypeDef;

/**
 * @brief  Humidity and temperature driver extended structure definition
 */
typedef struct
{
  HUM_TEMP_ComponentTypeDef
  id; /* This id must be unique for each component belonging to this class that wants to extend common class */
  void *pData; /* This pointer is specific for each component */
} HUM_TEMP_DrvExtTypeDef;

/**
  * @brief  Humidity and temperature driver structure definition
  */
typedef struct
{
  HUM_TEMP_StatusTypeDef       (*Init)(HUM_TEMP_InitTypeDef *);
  HUM_TEMP_StatusTypeDef       (*PowerOFF)(void);
  HUM_TEMP_StatusTypeDef       (*ReadID)(uint8_t *);
  HUM_TEMP_StatusTypeDef       (*Reset)(void);
  void                         (*ConfigIT)(uint16_t);
  void                         (*EnableIT)(uint8_t);
  void                         (*DisableIT)(uint8_t);
  uint8_t                      (*ITStatus)(uint16_t, uint16_t);
  void                         (*ClearIT)(uint16_t, uint16_t);
  HUM_TEMP_StatusTypeDef       (*GetHumidity)(float *);
  HUM_TEMP_StatusTypeDef       (*GetTemperature)(float *);
  HUM_TEMP_DrvExtTypeDef       *extData;
} HUM_TEMP_DrvTypeDef;

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

#endif /* __HUM_TEMP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
