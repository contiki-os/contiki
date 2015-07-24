/**
 ******************************************************************************
 * @file    x_nucleo_iks01a1_imu_6axes.c
 * @author  CL
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file provides a set of functions needed to manage the lsm6ds0 and lsm6ds3 sensors.
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
#include "x_nucleo_iks01a1_imu_6axes.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1_IMU_6AXES
 * @{
 */

/** @defgroup X_NUCLEO_IKS01A1_IMU_6AXES_Private_Defines X_NUCLEO_IKS01A1_IMU_6AXES_Private_Defines
 * @{
 */
#ifndef NULL
#define NULL      (void *) 0
#endif
/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_IMU_6AXES_Private_Variables X_NUCLEO_IKS01A1_IMU_6AXES_Private_Variables
 * @{
 */
static IMU_6AXES_DrvTypeDef *Imu6AxesDrv = NULL;
static uint8_t Imu6AxesInitialized = 0;
static uint8_t imu_6axes_sensor_type = 1; /* 1 activates LSM6DS3, 0 activates LSM6DS0 */

/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_IMU_6AXES_Exported_Functions X_NUCLEO_IKS01A1_IMU_6AXES_Exported_Functions
 * @{
 */

/**
 * @brief  Initialize the IMU 6 axes sensor
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_Init(void)
{
  IMU_6AXES_InitTypeDef InitStructure;
  uint8_t xg_id = 0;
  int done = 0;
  
  if(!Imu6AxesInitialized)
  {
    do
    {
      switch(imu_6axes_sensor_type)
      {
        case 1: /* Try to initialized LSM6DS3 */
        {
          /* Initialize the six axes driver structure */
          Imu6AxesDrv = &LSM6DS3Drv;
          
          /* Configure sensor */
          InitStructure.G_FullScale      = 2000.0f; /* 2000DPS */
          InitStructure.G_OutputDataRate = 104.0f;  /* 104HZ */
          InitStructure.G_X_Axis         = 1;       /* Enable */
          InitStructure.G_Y_Axis         = 1;       /* Enable */
          InitStructure.G_Z_Axis         = 1;       /* Enable */
          
          InitStructure.X_FullScale      = 2.0f;    /* 2G */
          InitStructure.X_OutputDataRate = 104.0f;  /* 104HZ */
          InitStructure.X_X_Axis         = 1;       /* Enable */
          InitStructure.X_Y_Axis         = 1;       /* Enable */
          InitStructure.X_Z_Axis         = 1;       /* Enable */
          
          if( Imu6AxesDrv->Init == NULL )
          {
            Imu6AxesDrv = NULL;
            imu_6axes_sensor_type--;
            break;
          }
          
          if( Imu6AxesDrv->Init(&InitStructure) != IMU_6AXES_OK)
          {
            Imu6AxesDrv = NULL;
            imu_6axes_sensor_type--;
            break;
          }
          
          if ( Imu6AxesDrv->Read_XG_ID == NULL )
          {
            Imu6AxesDrv = NULL;
            imu_6axes_sensor_type--;
            break;
          }
          
          if(Imu6AxesDrv->Read_XG_ID(&xg_id) != IMU_6AXES_OK)
          {
            Imu6AxesDrv = NULL;
            imu_6axes_sensor_type--;
            break;
          }
          
          if(xg_id == I_AM_LSM6DS3_XG)
          {
            Imu6AxesDrv->extData = (IMU_6AXES_DrvExtTypeDef *)&LSM6DS3Drv_ext;
            Imu6AxesInitialized = 1;
            done = 1;
            break;
          }
          else
          {
            Imu6AxesDrv = NULL;
            imu_6axes_sensor_type--;
            break;
          }
        }
        case 0: /* Try to initialized LSM6DS0 */
        default:
        {
          imu_6axes_sensor_type = 0;
          /* Initialize the six axes driver structure */
          Imu6AxesDrv = &LSM6DS0Drv;
          
          /* Configure sensor */
          InitStructure.G_FullScale       = 2000.0f; /* 2000DPS */
          InitStructure.G_OutputDataRate  = 119.0f;  /* 119HZ */
          InitStructure.G_X_Axis          = 1;       /* Enable */
          InitStructure.G_Y_Axis          = 1;       /* Enable */
          InitStructure.G_Z_Axis          = 1;       /* Enable */
          
          InitStructure.X_FullScale       = 2.0f;    /* 2G */
          InitStructure.X_OutputDataRate  = 119.0f;  /* 119HZ */
          InitStructure.X_X_Axis          = 1;       /* Enable */
          InitStructure.X_Y_Axis          = 1;       /* Enable */
          InitStructure.X_Z_Axis          = 1;       /* Enable */
          
          if( Imu6AxesDrv->Init == NULL )
          {
            Imu6AxesDrv = NULL;
            return IMU_6AXES_ERROR;
          }
          
          if( Imu6AxesDrv->Init(&InitStructure) != IMU_6AXES_OK)
          {
            Imu6AxesDrv = NULL;
            return IMU_6AXES_ERROR;
          }
          
          if ( Imu6AxesDrv->Read_XG_ID == NULL )
          {
            Imu6AxesDrv = NULL;
            return IMU_6AXES_ERROR;
          }
          
          if(Imu6AxesDrv->Read_XG_ID(&xg_id) != IMU_6AXES_OK)
          {
            Imu6AxesDrv = NULL;
            return IMU_6AXES_ERROR;
          }
          
          if(xg_id == I_AM_LSM6DS0_XG)
          {
            Imu6AxesDrv->extData = (IMU_6AXES_DrvExtTypeDef *)&LSM6DS0Drv_ext;
            Imu6AxesInitialized = 1;
            done = 1;
            break;
          }
        }
      }
    }
    while(!done);
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Check if the IMU 6 axes sensor is initialized
 * @retval 0 if the sensor is not initialized, 1 if the sensor is already initialized
 */
uint8_t BSP_IMU_6AXES_isInitialized(void)
{
  return Imu6AxesInitialized;
}


/**
 * @brief  Read the ID of the IMU 6 axes sensor
 * @param  xg_id the pointer where the who_am_i of the device is stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_Read_XG_ID(uint8_t *xg_id)
{
  if ( Imu6AxesDrv->Read_XG_ID == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Read_XG_ID(xg_id);
}


/**
 * @brief  Check the ID of the IMU 6 axes sensor
 * @retval IMU_6AXES_OK if the ID matches, IMU_6AXES_ERROR if the ID does not match or error occurs
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_Check_XG_ID(void)
{
  uint8_t xg_id;
  
  if(BSP_IMU_6AXES_Read_XG_ID(&xg_id) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  switch(imu_6axes_sensor_type)
  {
    case 1:
    {
      if(xg_id == I_AM_LSM6DS3_XG)
      {
        return IMU_6AXES_OK;
      }
      else
      {
        return IMU_6AXES_ERROR;
      }
    }
    case 0:
    default:
    {
      if(xg_id == I_AM_LSM6DS0_XG)
      {
        return IMU_6AXES_OK;
      }
      else
      {
        return IMU_6AXES_ERROR;
      }
    }
  }
}

/**
 * @brief  Get the accelerometer axes of the IMU 6 axes sensor
 * @param  pData the pointer where the output data are stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_X_GetAxes(Axes_TypeDef *pData)
{
  if ( Imu6AxesDrv->Get_X_Axes == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_X_Axes((int32_t *)pData);
}

/**
 * @brief  Get the accelerometer raw axes of the IMU 6 axes sensor
 * @param  pData the pointer where the output data are stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_X_GetAxesRaw(AxesRaw_TypeDef *pData)
{
  if ( Imu6AxesDrv->Get_X_AxesRaw == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_X_AxesRaw((int16_t *)pData);
}

/**
 * @brief  Get the gyroscope axes of the IMU 6 axes sensor
 * @param  pData the pointer where the output data are stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_G_GetAxes(Axes_TypeDef *pData)
{
  if ( Imu6AxesDrv->Get_G_Axes == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_G_Axes((int32_t *)pData);
}

/**
 * @brief  Get the gyroscope raw axes of the IMU 6 axes sensor
 * @param  pData the pointer where the output data are stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_G_GetAxesRaw(AxesRaw_TypeDef *pData)
{
  if ( Imu6AxesDrv->Get_G_AxesRaw == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_G_AxesRaw((int16_t *)pData);
}

/**
 * @brief  Get the accelerometer output data rate
 * @param  odr the pointer where the accelerometer output data rate is stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_X_Get_ODR(float *odr)
{
  if( Imu6AxesDrv->Get_X_ODR == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_X_ODR( odr );
}

/**
 * @brief  Set the accelerometer output data rate
 * @param  odr the accelerometer output data rate to be set
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_X_Set_ODR(float odr)
{
  if( Imu6AxesDrv->Set_X_ODR == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Set_X_ODR( odr );
}

/**
 * @brief  Get accelerometer sensitivity
 * @param  pfData the pointer where accelerometer sensitivity is stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_X_GetSensitivity( float *pfData )
{
  if( Imu6AxesDrv->Get_X_Sensitivity == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_X_Sensitivity( pfData );
}

/**
 * @brief  Get the accelerometer full scale
 * @param  fullScale the pointer where the accelerometer full scale is stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_X_Get_FS(float *fullScale)
{
  if( Imu6AxesDrv->Get_X_FS == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_X_FS( fullScale );
}

/**
 * @brief  Set the accelerometer full scale
 * @param  fullScale the accelerometer full scale to be set
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_X_Set_FS(float fullScale)
{
  if( Imu6AxesDrv->Set_X_FS == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Set_X_FS( fullScale );
}

/**
 * @brief  Get the gyroscope output data rate
 * @param  odr the pointer where the gyroscope output data rate is stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_G_Get_ODR(float *odr)
{
  if( Imu6AxesDrv->Get_G_ODR == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_G_ODR( odr );
}

/**
 * @brief  Set the gyroscope output data rate
 * @param  odr the gyroscope output data rate to be set
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_G_Set_ODR(float odr)
{
  if( Imu6AxesDrv->Set_G_ODR == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Set_G_ODR( odr );
}

/**
 * @brief  Get gyroscope sensitivity
 * @param  pfData the pointer where the gyroscope sensitivity is stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_G_GetSensitivity( float *pfData )
{
  if( Imu6AxesDrv->Get_G_Sensitivity == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_G_Sensitivity( pfData );
}

/**
 * @brief  Get the gyroscope full scale
 * @param  fullScale the pointer where the gyroscope full scale is stored
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_G_Get_FS(float *fullScale)
{
  if( Imu6AxesDrv->Get_G_FS == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Get_G_FS( fullScale );
}

/**
 * @brief  Set the gyroscope full scale
 * @param  fullScale the gyroscope full scale to be set
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_G_Set_FS(float fullScale)
{
  if( Imu6AxesDrv->Set_G_FS == NULL )
  {
    return IMU_6AXES_ERROR;
  }
  
  return Imu6AxesDrv->Set_G_FS( fullScale );
}

/**
 * @brief  Get component type currently used
 * @retval IMU_6AXES_NONE_COMPONENT if none component is currently used, the component unique id otherwise
 */
IMU_6AXES_ComponentTypeDef BSP_IMU_6AXES_GetComponentType( void )
{
  if( Imu6AxesDrv == NULL )
  {
    return IMU_6AXES_NONE_COMPONENT;
  }
  
  if( Imu6AxesDrv == &LSM6DS0Drv )
  {
    return IMU_6AXES_LSM6DS0_COMPONENT;
  }
  
  if( Imu6AxesDrv == &LSM6DS3Drv )
  {
    return IMU_6AXES_LSM6DS3_DIL24_COMPONENT;
  }
  
  return IMU_6AXES_NONE_COMPONENT;
}

/**
 * @brief  Enable free fall detection (available only for LSM6DS3 sensor)
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_NOT_IMPLEMENTED if the feature is not supported, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_Enable_Free_Fall_Detection_Ext(void)
{
  /* At the moment this feature is only implemented for LSM6DS3 */
  if( Imu6AxesDrv->extData == NULL || Imu6AxesDrv->extData->id != IMU_6AXES_LSM6DS3_DIL24_COMPONENT
      || Imu6AxesDrv->extData->pData == NULL)
  {
    return IMU_6AXES_NOT_IMPLEMENTED;
  }
  
  if(((LSM6DS3_DrvExtTypeDef *)(Imu6AxesDrv->extData->pData))->Enable_Free_Fall_Detection == NULL)
  {
    return IMU_6AXES_NOT_IMPLEMENTED;
  }
  
  return ((LSM6DS3_DrvExtTypeDef *)(Imu6AxesDrv->extData->pData))->Enable_Free_Fall_Detection();
}

/**
 * @brief  Disable free fall detection (available only for LSM6DS3 sensor)
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_NOT_IMPLEMENTED if the feature is not supported, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_Disable_Free_Fall_Detection_Ext(void)
{
  /* At the moment this feature is only implemented for LSM6DS3 */
  if( Imu6AxesDrv->extData == NULL || Imu6AxesDrv->extData->id != IMU_6AXES_LSM6DS3_DIL24_COMPONENT
      || Imu6AxesDrv->extData->pData == NULL)
  {
    return IMU_6AXES_NOT_IMPLEMENTED;
  }
  
  if(((LSM6DS3_DrvExtTypeDef *)(Imu6AxesDrv->extData->pData))->Disable_Free_Fall_Detection == NULL)
  {
    return IMU_6AXES_NOT_IMPLEMENTED;
  }
  
  return ((LSM6DS3_DrvExtTypeDef *)(Imu6AxesDrv->extData->pData))->Disable_Free_Fall_Detection();
}

/**
 * @brief  Get status of free fall detection (available only for LSM6DS3 sensor)
 * @param  status the pointer where the status of free fall detection is stored; 0 means no detection, 1 means detection happened
 * @retval IMU_6AXES_OK in case of success, IMU_6AXES_NOT_IMPLEMENTED if the feature is not supported, IMU_6AXES_ERROR otherwise
 */
IMU_6AXES_StatusTypeDef BSP_IMU_6AXES_Get_Status_Free_Fall_Detection_Ext(uint8_t *status)
{
  /* At the moment this feature is only implemented for LSM6DS3 */
  if( Imu6AxesDrv->extData == NULL || Imu6AxesDrv->extData->id != IMU_6AXES_LSM6DS3_DIL24_COMPONENT
      || Imu6AxesDrv->extData->pData == NULL)
  {
    return IMU_6AXES_NOT_IMPLEMENTED;
  }
  
  if(((LSM6DS3_DrvExtTypeDef *)(Imu6AxesDrv->extData->pData))->Get_Status_Free_Fall_Detection == NULL)
  {
    return IMU_6AXES_NOT_IMPLEMENTED;
  }
  
  if(status == NULL)
  {
    return IMU_6AXES_ERROR;
  }
  
  return ((LSM6DS3_DrvExtTypeDef *)(Imu6AxesDrv->extData->pData))->Get_Status_Free_Fall_Detection(status);
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
