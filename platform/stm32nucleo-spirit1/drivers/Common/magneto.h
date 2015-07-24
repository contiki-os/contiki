/**
 ******************************************************************************
 * @file    magneto.h
 * @author  MEMS Application Team
 * @version V1.2.0
 * @date    28-January-2015
 * @brief   This header file contains the functions prototypes for the
 *          magneto driver.
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
#ifndef __MAGNETO_H
#define __MAGNETO_H

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

/** @addtogroup MAGNETO
  * @{
  */

/** @defgroup MAGNETO_Exported_Types
  * @{
  */

/**
* @brief  MAGNETO init structure definition
*/
typedef struct
{
  uint8_t M_OutputDataRate;
  uint8_t M_OperatingMode;
  uint8_t M_FullScale;
  uint8_t M_XYOperativeMode;
} MAGNETO_InitTypeDef;

/**
* @brief  MAGNETO status enumerator definition
*/
typedef enum
{
  MAGNETO_OK = 0,
  MAGNETO_ERROR = 1,
  MAGNETO_TIMEOUT = 2,
  MAGNETO_NOT_IMPLEMENTED = 3
} MAGNETO_StatusTypeDef;

/**
 * @brief  MAGNETO component id enumerator definition
 */
typedef enum
{
  MAGNETO_NONE_COMPONENT = 0,
  MAGNETO_LIS3MDL_COMPONENT = 1
} MAGNETO_ComponentTypeDef;

/**
 * @brief  MAGNETO driver extended structure definition
 */
typedef struct
{
  MAGNETO_ComponentTypeDef
  id; /* This id must be unique for each component belonging to this class that wants to extend common class */
  void *pData; /* This pointer is specific for each component */
} MAGNETO_DrvExtTypeDef;

/**
* @brief  MAGNETO driver structure definition
*/
typedef struct
{
  MAGNETO_StatusTypeDef       (*Init)(MAGNETO_InitTypeDef *);
  MAGNETO_StatusTypeDef       (*Read_M_ID)(uint8_t *);
  MAGNETO_StatusTypeDef       (*Get_M_Axes)(int32_t *);
  MAGNETO_StatusTypeDef       (*Get_M_AxesRaw)(int16_t *);
  MAGNETO_DrvExtTypeDef       *extData;
} MAGNETO_DrvTypeDef;

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

#endif /* __MAGNETO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
