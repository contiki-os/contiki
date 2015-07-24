/**
 ******************************************************************************
 * @file    x_nucleo_iks01a1_hum_temp.c
 * @author  CL
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file provides a set of functions needed to manage the hts221 sensor.
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
/* Includes ------------------------------------------------------------------*/
#include "x_nucleo_iks01a1_hum_temp.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1_HUM_TEMP
 * @{
 */


/** @defgroup X_NUCLEO_IKS01A1_HUM_TEMP_Private_Defines X_NUCLEO_IKS01A1_HUM_TEMP_Private_Defines
 * @{
 */
#ifndef NULL
#define NULL      (void *) 0
#endif
/**
 * @}
 */


/** @defgroup X_NUCLEO_IKS01A1_HUM_TEMP_Private_Variables X_NUCLEO_IKS01A1_HUM_TEMP_Private_Variables
 * @{
 */
static HUM_TEMP_DrvTypeDef *Hum_tempDrv = NULL;
static uint8_t HumTempInitialized = 0;
/**
 * @}
 */


/** @defgroup X_NUCLEO_IKS01A1_HUM_TEMP_Exported_Functions X_NUCLEO_IKS01A1_HUM_TEMP_Exported_Functions
 * @{
 */

/**
 * @brief  Initialize the humidity and temperature sensor
 * @retval HUM_TEMP_OK in case of success, HUM_TEMP_ERROR otherwise
 */
HUM_TEMP_StatusTypeDef    BSP_HUM_TEMP_Init(void)
{
  uint8_t ht_id = 0;
  HUM_TEMP_InitTypeDef InitStructure;
  
  if(!HumTempInitialized)
  {
    /* Initialize the hum_temp driver structure */
    Hum_tempDrv = &Hts221Drv;
    
    /* Configure sensor */
    InitStructure.OutputDataRate = HTS221_ODR_12_5Hz;
    
    /* Hts221 Init */
    if ( Hum_tempDrv->Init == NULL )
    {
      Hum_tempDrv = NULL;
      return HUM_TEMP_ERROR;
    }
    
    if(Hum_tempDrv->Init(&InitStructure) != HUM_TEMP_OK)
    {
      Hum_tempDrv = NULL;
      return HUM_TEMP_ERROR;
    }
    
    if ( Hum_tempDrv->ReadID == NULL )
    {
      Hum_tempDrv = NULL;
      return HUM_TEMP_ERROR;
    }
    
    if(Hum_tempDrv->ReadID(&ht_id) != HUM_TEMP_OK)
    {
      Hum_tempDrv = NULL;
      return HUM_TEMP_ERROR;
    }
    
    if(ht_id == I_AM_HTS221)
    {
      Hum_tempDrv->extData = (HUM_TEMP_DrvExtTypeDef *)&Hts221Drv_ext;
      HumTempInitialized = 1;
    }
  }
  
  return HUM_TEMP_OK;
}

/**
 * @brief  Check if the humidity and temperature sensor is initialized
 * @retval 0 if the sensor is not initialized, 1 if the sensor is already initialized
 */
uint8_t BSP_HUM_TEMP_isInitialized(void)
{
  return HumTempInitialized;
}

/**
 * @brief  Read the ID of the humidity and temperature component
 * @param  ht_id the pointer where the who_am_i of the device is stored
 * @retval HUM_TEMP_OK in case of success, HUM_TEMP_ERROR otherwise
 */
HUM_TEMP_StatusTypeDef    BSP_HUM_TEMP_ReadID(uint8_t *ht_id)
{
  if ( Hum_tempDrv->ReadID == NULL )
  {
    return HUM_TEMP_ERROR;
  }
  
  return Hum_tempDrv->ReadID(ht_id);
}


/**
 * @brief  Check the ID of the humidity and temperature sensor
 * @retval HUM_TEMP_OK if the ID matches, HUM_TEMP_ERROR if the ID does not match or error occurs
 */
HUM_TEMP_StatusTypeDef BSP_HUM_TEMP_CheckID(void)
{
  uint8_t ht_id;
  
  if(BSP_HUM_TEMP_ReadID(&ht_id) != HUM_TEMP_OK)
  {
    return HUM_TEMP_ERROR;
  }
  
  if(ht_id == I_AM_HTS221)
  {
    return HUM_TEMP_OK;
  }
  else
  {
    return HUM_TEMP_ERROR;
  }
}


/**
 * @brief  Reboot memory content of humidity and temperature sensor
 * @retval HUM_TEMP_OK in case of success, HUM_TEMP_ERROR otherwise
 */
HUM_TEMP_StatusTypeDef BSP_HUM_TEMP_Reset(void)
{
  if ( Hum_tempDrv->Reset == NULL )
  {
    return HUM_TEMP_ERROR;
  }
  
  return Hum_tempDrv->Reset();
}


/**
 * @brief  Power off the humidity and temperature sensor
 * @retval HUM_TEMP_OK in case of success, HUM_TEMP_ERROR otherwise
 */
HUM_TEMP_StatusTypeDef BSP_HUM_TEMP_PowerOFF()
{
  if ( Hum_tempDrv->PowerOFF == NULL )
  {
    return HUM_TEMP_ERROR;
  }
  
  return Hum_tempDrv->PowerOFF();
}


/**
 * @brief  Get the humidity value
 * @param  pfData the pointer to floating data
 * @retval HUM_TEMP_OK in case of success, HUM_TEMP_ERROR otherwise
 */
HUM_TEMP_StatusTypeDef BSP_HUM_TEMP_GetHumidity(float* pfData)
{
  if ( Hum_tempDrv->GetHumidity == NULL )
  {
    return HUM_TEMP_ERROR;
  }
  
  return Hum_tempDrv->GetHumidity(pfData);
}

/**
 * @brief  Get the temperature value
 * @param  pfData the pointer to floating data
 * @retval HUM_TEMP_OK in case of success, HUM_TEMP_ERROR otherwise
 */
HUM_TEMP_StatusTypeDef BSP_HUM_TEMP_GetTemperature(float* pfData)
{
  if ( Hum_tempDrv->GetTemperature == NULL )
  {
    return HUM_TEMP_ERROR;
  }
  
  return Hum_tempDrv->GetTemperature(pfData);
}

/**
 * @brief  Get component type currently used
 * @retval HUM_TEMP_NONE_COMPONENT if none component is currently used, the component unique id otherwise
 */
HUM_TEMP_ComponentTypeDef BSP_HUM_TEMP_GetComponentType( void )
{
  if( Hum_tempDrv == NULL )
  {
    return HUM_TEMP_NONE_COMPONENT;
  }
  
  if( Hum_tempDrv == &Hts221Drv )
  {
    return HUM_TEMP_HTS221_COMPONENT;
  }
  
  return HUM_TEMP_NONE_COMPONENT;
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
