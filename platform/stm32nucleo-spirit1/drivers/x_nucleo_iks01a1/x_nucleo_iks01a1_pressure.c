/**
 ******************************************************************************
 * @file    x_nucleo_iks01a1_pressure.c
 * @author  CL
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file provides a set of functions needed to manage the lps25h and lps25hb sensors.
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
#include "x_nucleo_iks01a1_pressure.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1_PRESSURE
 * @{
 */


/** @defgroup X_NUCLEO_IKS01A1_PRESSURE_Private_Defines X_NUCLEO_IKS01A1_PRESSURE_Private_Defines
 * @{
 */
#ifndef NULL
#define NULL      (void *) 0
#endif
/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_PRESSURE_Private_Variables X_NUCLEO_IKS01A1_PRESSURE_Private_Variables
 * @{
 */
static PRESSURE_DrvTypeDef *PressureDrv = NULL;
static uint8_t PressureInitialized = 0;
static uint8_t pressure_sensor_type = 1; /* 1 activates LPS25HB, 0 activates LPS25H */

/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_PRESSURE_Exported_Functions X_NUCLEO_IKS01A1_PRESSURE_Exported_Functions
 * @{
 */

/**
 * @brief  Initialize the pressure sensor
 * @retval PRESSURE_OK in case of success, PRESSURE_ERROR otherwise
 */
PRESSURE_StatusTypeDef BSP_PRESSURE_Init(void)
{
  uint8_t p_id = 0;
  PRESSURE_InitTypeDef InitStructure;
  int done = 0;
  
  if(!PressureInitialized)
  {
    do
    {
      switch(pressure_sensor_type)
      {
        case 1:
        {
          /* Initialize the pressure driver structure */
          PressureDrv = &LPS25HBDrv;
          
          /* Configure sensor */
          InitStructure.OutputDataRate = LPS25HB_ODR_1Hz;
          InitStructure.BlockDataUpdate = LPS25HB_BDU_CONT;
          InitStructure.DiffEnable = LPS25HB_DIFF_ENABLE;
          InitStructure.SPIMode = LPS25HB_SPI_SIM_3W;
          InitStructure.PressureResolution = LPS25HB_P_RES_AVG_32;
          InitStructure.TemperatureResolution = LPS25HB_T_RES_AVG_16;
          
          /* Pressure sensor init */
          if ( PressureDrv->Init == NULL )
          {
            PressureDrv = NULL;
            pressure_sensor_type--;
            break;
          }
          
          if(PressureDrv->Init(&InitStructure) != PRESSURE_OK)
          {
            PressureDrv = NULL;
            pressure_sensor_type--;
            break;
          }
          
          if ( PressureDrv->ReadID == NULL )
          {
            PressureDrv = NULL;
            pressure_sensor_type--;
            break;
          }
          
          if(PressureDrv->ReadID(&p_id) != PRESSURE_OK)
          {
            PressureDrv = NULL;
            pressure_sensor_type--;
            break;
          }
          
          if(p_id == I_AM_LPS25HB)
          {
            PressureDrv->extData = (PRESSURE_DrvExtTypeDef *)&LPS25HBDrv_ext;
            PressureInitialized = 1;
            done = 1;
            break;
          }
          else
          {
            PressureDrv = NULL;
            pressure_sensor_type--;
            break;
          }
        }
        case 0:
        default:
        {
          /* Initialize the pressure driver structure */
          PressureDrv = &LPS25HDrv;
          
          /* Configure sensor */
          InitStructure.OutputDataRate = LPS25H_ODR_1Hz;
          InitStructure.BlockDataUpdate = LPS25H_BDU_CONT;
          InitStructure.DiffEnable = LPS25H_DIFF_ENABLE;
          InitStructure.SPIMode = LPS25H_SPI_SIM_3W;
          InitStructure.PressureResolution = LPS25H_P_RES_AVG_32;
          InitStructure.TemperatureResolution = LPS25H_T_RES_AVG_16;
          
          /* Pressure sensor init */
          if ( PressureDrv->Init == NULL )
          {
            PressureDrv = NULL;
            return PRESSURE_ERROR;
          }
          
          if(PressureDrv->Init(&InitStructure) != PRESSURE_OK)
          {
            PressureDrv = NULL;
            return PRESSURE_ERROR;
          }
          
          if ( PressureDrv->ReadID == NULL )
          {
            PressureDrv = NULL;
            return PRESSURE_ERROR;
          }
          
          if(PressureDrv->ReadID(&p_id) != PRESSURE_OK)
          {
            PressureDrv = NULL;
            return PRESSURE_ERROR;
          }
          
          if(p_id == I_AM_LPS25H)
          {
            PressureDrv->extData = (PRESSURE_DrvExtTypeDef *)&LPS25HDrv_ext;
            PressureInitialized = 1;
            done = 1;
            break;
          }
        }
      }
    }
    while(!done);
  }
  
  return PRESSURE_OK;
}

/**
 * @brief  Check if the pressure sensor is initialized
 * @retval 0 if the sensor is not initialized, 1 if the sensor is already initialized
 */
uint8_t BSP_PRESSURE_isInitialized(void)
{
  return PressureInitialized;
}

/**
 * @brief  Read the ID of the pressure sensor
 * @param  p_id the pointer where the who_am_i of the device is stored
 * @retval PRESSURE_OK in case of success, PRESSURE_ERROR otherwise
 */
PRESSURE_StatusTypeDef BSP_PRESSURE_ReadID(uint8_t *p_id)
{
  if ( PressureDrv->ReadID == NULL )
  {
    return PRESSURE_ERROR;
  }
  
  return PressureDrv->ReadID(p_id);
}


/**
 * @brief  Check the ID of the pressure sensor
 * @retval PRESSURE_OK if the ID matches, PRESSURE_ERROR if the ID does not match or error occurs
 */
PRESSURE_StatusTypeDef BSP_PRESSURE_CheckID(void)
{
  uint8_t p_id;
  
  if(BSP_PRESSURE_ReadID(&p_id) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  switch(pressure_sensor_type)
  {
    case 1:
    {
      if(p_id == I_AM_LPS25HB)
      {
        return PRESSURE_OK;
      }
      else
      {
        return PRESSURE_ERROR;
      }
    }
    case 0:
    default:
    {
      if(p_id == I_AM_LPS25H)
      {
        return PRESSURE_OK;
      }
      else
      {
        return PRESSURE_ERROR;
      }
    }
  }
}


/**
 * @brief  Reboot the memory content of the pressure sensor
 * @retval PRESSURE_OK in case of success, PRESSURE_ERROR otherwise
 */
PRESSURE_StatusTypeDef BSP_PRESSURE_Reset(void)
{
  if ( PressureDrv->Reset == NULL )
  {
    return PRESSURE_ERROR;
  }
  
  return PressureDrv->Reset();
}


/**
 * @brief  Get the pressure
 * @param  pfData the pointer where the output data are stored
 * @retval PRESSURE_OK in case of success, PRESSURE_ERROR otherwise
 */
PRESSURE_StatusTypeDef BSP_PRESSURE_GetPressure(float* pfData)
{
  if ( PressureDrv->GetPressure == NULL )
  {
    return PRESSURE_ERROR;
  }
  
  return PressureDrv->GetPressure(pfData);
}

/**
 * @brief  Get the temperature
 * @param  pfData the pointer where the output data are stored
 * @retval PRESSURE_OK in case of success, PRESSURE_ERROR otherwise
 */
PRESSURE_StatusTypeDef BSP_PRESSURE_GetTemperature(float* pfData)
{
  if ( PressureDrv->GetTemperature == NULL )
  {
    return PRESSURE_ERROR;
  }
  
  return PressureDrv->GetTemperature(pfData);
}

/**
 * @brief  Get component type currently used
 * @retval PRESSURE_NONE_COMPONENT if none component is currently used, the component unique id otherwise
 */
PRESSURE_ComponentTypeDef BSP_PRESSURE_GetComponentType( void )
{
  if( PressureDrv == NULL )
  {
    return PRESSURE_NONE_COMPONENT;
  }
  
  if( PressureDrv == &LPS25HDrv )
  {
    return PRESSURE_LPS25H_COMPONENT;
  }
  
  if( PressureDrv == &LPS25HBDrv )
  {
    return PRESSURE_LPS25HB_DIL24_COMPONENT;
  }
  
  return PRESSURE_NONE_COMPONENT;
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
