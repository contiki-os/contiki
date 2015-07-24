/**
 ******************************************************************************
 * @file    x_nucleo_iks01a1_magneto.c
 * @author  CL
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file provides a set of functions needed to manage the lis3mdl sensor.
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
#include "x_nucleo_iks01a1_magneto.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1_MAGNETO
 * @{
 */

/** @defgroup X_NUCLEO_IKS01A1_MAGNETO_Private_Defines X_NUCLEO_IKS01A1_MAGNETO_Private_Defines
 * @{
 */
#ifndef NULL
#define NULL      (void *) 0
#endif
/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_MAGNETO_Private_Variables X_NUCLEO_IKS01A1_MAGNETO_Private_Variables
 * @{
 */
static MAGNETO_DrvTypeDef *MagnetoDrv = NULL;
static uint8_t MagnetoInitialized = 0;

/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_MAGNETO_Exported_Functions X_NUCLEO_IKS01A1_MAGNETO_Exported_Functions
 * @{
 */

/**
 * @brief  Initialize the magneto sensor
 * @retval MAGNETO_OK in case of success, MAGNETO_ERROR otherwise
 */
MAGNETO_StatusTypeDef BSP_MAGNETO_Init(void)
{
  uint8_t m_id = 0;
  MAGNETO_InitTypeDef InitStructure;
  
  if(!MagnetoInitialized)
  {
    /* Initialize the magneto driver structure */
    MagnetoDrv = &LIS3MDLDrv;
    
    /* Configure sensor */
    InitStructure.M_FullScale = LIS3MDL_M_FS_4;
    InitStructure.M_OperatingMode = LIS3MDL_M_MD_CONTINUOUS;
    InitStructure.M_XYOperativeMode = LIS3MDL_M_OM_HP;
    InitStructure.M_OutputDataRate = LIS3MDL_M_DO_80;
    
    /* magneto sensor init */
    if ( MagnetoDrv->Init == NULL )
    {
      MagnetoDrv = NULL;
      return MAGNETO_ERROR;
    }
    
    if(MagnetoDrv->Init(&InitStructure) != MAGNETO_OK)
    {
      MagnetoDrv = NULL;
      return MAGNETO_ERROR;
    }
    
    if ( MagnetoDrv->Read_M_ID == NULL )
    {
      MagnetoDrv = NULL;
      return MAGNETO_ERROR;
    }
    
    if(MagnetoDrv->Read_M_ID(&m_id) != MAGNETO_OK)
    {
      MagnetoDrv = NULL;
      return MAGNETO_ERROR;
    }
    
    if(m_id == I_AM_LIS3MDL_M)
    {
      MagnetoDrv->extData = (MAGNETO_DrvExtTypeDef *)&LIS3MDLDrv_ext;
      MagnetoInitialized = 1;
    }
  }
  
  return MAGNETO_OK;
}

/**
 * @brief  Check if the magnetic sensor is initialized
 * @retval 0 if the sensor is not initialized, 1 if the sensor is already initialized
 */
uint8_t BSP_MAGNETO_isInitialized(void)
{
  return MagnetoInitialized;
}


/**
 * @brief  Read the ID of the magnetic sensor
 * @param  m_id the pointer where the who_am_i of the device is stored
 * @retval MAGNETO_OK in case of success, MAGNETO_ERROR otherwise
 */
MAGNETO_StatusTypeDef BSP_MAGNETO_Read_M_ID(uint8_t *m_id)
{
  if ( MagnetoDrv->Read_M_ID == NULL )
  {
    return MAGNETO_ERROR;
  }
  
  return MagnetoDrv->Read_M_ID(m_id);
}


/**
 * @brief  Check the ID of the magnetic sensor
 * @retval MAGNETO_OK if the ID matches, MAGNETO_ERROR if the ID does not match or error occurs
 */
MAGNETO_StatusTypeDef BSP_MAGNETO_Check_M_ID(void)
{
  uint8_t m_id;
  
  if(BSP_MAGNETO_Read_M_ID(&m_id) != MAGNETO_OK)
  {
    return MAGNETO_ERROR;
  }
  
  if(m_id == I_AM_LIS3MDL_M)
  {
    return MAGNETO_OK;
  }
  else
  {
    return MAGNETO_ERROR;
  }
}

/**
 * @brief  Get the magnetic sensor axes
 * @param  pData the pointer where the output data are stored
 * @retval MAGNETO_OK in case of success, MAGNETO_ERROR otherwise
 */
MAGNETO_StatusTypeDef BSP_MAGNETO_M_GetAxes(Axes_TypeDef *pData)
{
  if ( MagnetoDrv->Get_M_Axes == NULL )
  {
    return MAGNETO_ERROR;
  }
  
  return MagnetoDrv->Get_M_Axes((int32_t *)pData);
}

/**
 * @brief  Get the magnetic sensor raw axes
 * @param  pData the pointer where the output data are stored
 * @retval MAGNETO_OK in case of success, MAGNETO_ERROR otherwise
 */
MAGNETO_StatusTypeDef BSP_MAGNETO_M_GetAxesRaw(AxesRaw_TypeDef *pData)
{
  if ( MagnetoDrv->Get_M_AxesRaw == NULL )
  {
    return MAGNETO_ERROR;
  }
  
  return MagnetoDrv->Get_M_AxesRaw((int16_t *)pData);
}

/**
 * @brief  Get component type currently used
 * @retval MAGNETO_NONE_COMPONENT if none component is currently used, the component unique id otherwise
 */
MAGNETO_ComponentTypeDef BSP_MAGNETO_GetComponentType( void )
{
  if( MagnetoDrv == NULL )
  {
    return MAGNETO_NONE_COMPONENT;
  }
  
  if( MagnetoDrv == &LIS3MDLDrv )
  {
    return MAGNETO_LIS3MDL_COMPONENT;
  }
  
  return MAGNETO_NONE_COMPONENT;
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
