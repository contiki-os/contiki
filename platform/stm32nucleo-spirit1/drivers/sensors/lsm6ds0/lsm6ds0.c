/**
 ******************************************************************************
 * @file    lsm6ds0.c
 * @author  MEMS Application Team
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file provides a set of functions needed to manage the lsm6ds0.
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
#include "lsm6ds0.h"
#include <math.h>

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup LSM6DS0
 * @{
 */

static IMU_6AXES_StatusTypeDef LSM6DS0_Init(IMU_6AXES_InitTypeDef *LSM6DS0_Init);
static IMU_6AXES_StatusTypeDef LSM6DS0_Read_XG_ID(uint8_t *xg_id);
static IMU_6AXES_StatusTypeDef LSM6DS0_X_GetAxes(int32_t *pData);
static IMU_6AXES_StatusTypeDef LSM6DS0_X_GetAxesRaw(int16_t *pData);
static IMU_6AXES_StatusTypeDef LSM6DS0_G_GetAxes(int32_t *pData);
static IMU_6AXES_StatusTypeDef LSM6DS0_G_GetAxesRaw(int16_t *pData);
static IMU_6AXES_StatusTypeDef LSM6DS0_X_Get_ODR( float *odr );
static IMU_6AXES_StatusTypeDef LSM6DS0_X_Set_ODR( float odr );
static IMU_6AXES_StatusTypeDef LSM6DS0_X_GetSensitivity( float *pfData );
static IMU_6AXES_StatusTypeDef LSM6DS0_X_Get_FS( float *fullScale );
static IMU_6AXES_StatusTypeDef LSM6DS0_X_Set_FS( float fullScale );
static IMU_6AXES_StatusTypeDef LSM6DS0_G_Get_ODR( float *odr );
static IMU_6AXES_StatusTypeDef LSM6DS0_G_Set_ODR( float odr );
static IMU_6AXES_StatusTypeDef LSM6DS0_G_GetSensitivity( float *pfData );
static IMU_6AXES_StatusTypeDef LSM6DS0_G_Get_FS( float *fullScale );
static IMU_6AXES_StatusTypeDef LSM6DS0_G_Set_FS( float fullScale );

/** @defgroup LSM6DS0_Private_Variables LSM6DS0_Private_Variables
 * @{
 */

IMU_6AXES_DrvTypeDef LSM6DS0Drv =
{
  LSM6DS0_Init,
  LSM6DS0_Read_XG_ID,
  LSM6DS0_X_GetAxes,
  LSM6DS0_X_GetAxesRaw,
  LSM6DS0_G_GetAxes,
  LSM6DS0_G_GetAxesRaw,
  LSM6DS0_X_Get_ODR,
  LSM6DS0_X_Set_ODR,
  LSM6DS0_X_GetSensitivity,
  LSM6DS0_X_Get_FS,
  LSM6DS0_X_Set_FS,
  LSM6DS0_G_Get_ODR,
  LSM6DS0_G_Set_ODR,
  LSM6DS0_G_GetSensitivity,
  LSM6DS0_G_Get_FS,
  LSM6DS0_G_Set_FS,
  NULL
};

/* ------------------------------------------------------- */
/* Here you should declare the variable that implements    */
/* the internal struct of extended features of LSM6DS0.    */
/* Then you must update the NULL pointer in the variable   */
/* of the extended features below.                         */
/* See the example of LSM6DS3 in lsm6ds3.c                 */
/* ------------------------------------------------------- */

IMU_6AXES_DrvExtTypeDef LSM6DS0Drv_ext =
{
  IMU_6AXES_LSM6DS0_COMPONENT, /* unique ID for LSM6DS0 in the IMU 6-axes driver class */
  NULL /* pointer to internal struct of extended features of LSM6DS0 */
};

/**
 * @}
 */

static IMU_6AXES_StatusTypeDef LSM6DS0_X_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ);
static IMU_6AXES_StatusTypeDef LSM6DS0_G_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ);

/** @defgroup LSM6DS0_Private_Functions LSM6DS0_Private_Functions
 * @{
 */

/**
 * @brief  Set LSM6DS0 Initialization
 * @param  LSM6DS0_Init the configuration setting for the LSM6DS0
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS0_Init(IMU_6AXES_InitTypeDef *LSM6DS0_Init)
{
  /* Configure the low level interface ---------------------------------------*/
  if(LSM6DS0_IO_Init() != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /******* Gyroscope init *******/
  
  if(LSM6DS0_G_Set_ODR( LSM6DS0_Init->G_OutputDataRate ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS0_G_Set_FS( LSM6DS0_Init->G_FullScale ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS0_G_Set_Axes_Status(LSM6DS0_Init->G_X_Axis, LSM6DS0_Init->G_Y_Axis, LSM6DS0_Init->G_Z_Axis) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /******************************/
  
  /***** Accelerometer init *****/
  
  if(LSM6DS0_X_Set_ODR( LSM6DS0_Init->X_OutputDataRate ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS0_X_Set_FS( LSM6DS0_Init->X_FullScale ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS0_X_Set_Axes_Status(LSM6DS0_Init->X_X_Axis, LSM6DS0_Init->X_Y_Axis, LSM6DS0_Init->X_Z_Axis) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Configure interrupt lines */
  LSM6DS0_IO_ITConfig();
  
  return IMU_6AXES_OK;
  
  /******************************/
}


/**
 * @brief  Read ID of LSM6DS0 Accelerometer and Gyroscope
 * @param  xg_id the pointer where the ID of the device is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS0_Read_XG_ID(uint8_t *xg_id)
{
  if(!xg_id)
  {
    return IMU_6AXES_ERROR;
  }
  
  return LSM6DS0_IO_Read(xg_id, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_WHO_AM_I_ADDR, 1);
}


/**
 * @brief  Read raw data from LSM6DS0 Accelerometer output register
 * @param  pData the pointer where the accelerometer raw data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS0_X_GetAxesRaw(int16_t *pData)
{
  uint8_t tempReg[2] = {0, 0};
  
  if(LSM6DS0_IO_Read(&tempReg[0], LSM6DS0_XG_MEMS_ADDRESS, (LSM6DS0_XG_OUT_X_L_XL | LSM6DS0_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[0] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS0_IO_Read(&tempReg[0], LSM6DS0_XG_MEMS_ADDRESS, (LSM6DS0_XG_OUT_Y_L_XL | LSM6DS0_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[1] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS0_IO_Read(&tempReg[0], LSM6DS0_XG_MEMS_ADDRESS, (LSM6DS0_XG_OUT_Z_L_XL | LSM6DS0_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[2] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  return IMU_6AXES_OK;
}


/**
 * @brief  Read data from LSM6DS0 Accelerometer and calculate linear acceleration in mg
 * @param  pData the pointer where the accelerometer data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS0_X_GetAxes(int32_t *pData)
{
  int16_t pDataRaw[3];
  float sensitivity = 0;
  
  if(LSM6DS0_X_GetAxesRaw(pDataRaw) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS0_X_GetSensitivity( &sensitivity ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[0] = (int32_t)(pDataRaw[0] * sensitivity);
  pData[1] = (int32_t)(pDataRaw[1] * sensitivity);
  pData[2] = (int32_t)(pDataRaw[2] * sensitivity);
  
  return IMU_6AXES_OK;
}


/**
 * @brief  Read raw data from LSM6DS0 Gyroscope output register
 * @param  pData the pointer where the gyroscope raw data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS0_G_GetAxesRaw(int16_t *pData)
{
  uint8_t tempReg[2] = {0, 0};
  
  if(LSM6DS0_IO_Read(&tempReg[0], LSM6DS0_XG_MEMS_ADDRESS, (LSM6DS0_XG_OUT_X_L_G | LSM6DS0_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[0] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS0_IO_Read(&tempReg[0], LSM6DS0_XG_MEMS_ADDRESS, (LSM6DS0_XG_OUT_Y_L_G | LSM6DS0_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[1] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS0_IO_Read(&tempReg[0], LSM6DS0_XG_MEMS_ADDRESS, (LSM6DS0_XG_OUT_Z_L_G | LSM6DS0_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[2] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Set the status of the axes for accelerometer
 * @param  enableX the status of the x axis to be set
 * @param  enableY the status of the y axis to be set
 * @param  enableZ the status of the z axis to be set
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS0_X_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ)
{
  uint8_t tmp1 = 0x00;
  uint8_t eX = 0x00;
  uint8_t eY = 0x00;
  uint8_t eZ = 0x00;
  
  eX = ( enableX == 0 ) ? LSM6DS0_XL_XEN_DISABLE : LSM6DS0_XL_XEN_ENABLE;
  eY = ( enableY == 0 ) ? LSM6DS0_XL_YEN_DISABLE : LSM6DS0_XL_YEN_ENABLE;
  eZ = ( enableZ == 0 ) ? LSM6DS0_XL_ZEN_DISABLE : LSM6DS0_XL_ZEN_ENABLE;
  
  if(LSM6DS0_IO_Read(&tmp1, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG5_XL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Enable X axis selection */
  tmp1 &= ~(LSM6DS0_XL_XEN_MASK);
  tmp1 |= eX;
  
  /* Enable Y axis selection */
  tmp1 &= ~(LSM6DS0_XL_YEN_MASK);
  tmp1 |= eY;
  
  /* Enable Z axis selection */
  tmp1 &= ~(LSM6DS0_XL_ZEN_MASK);
  tmp1 |= eZ;
  
  if(LSM6DS0_IO_Write(&tmp1, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG5_XL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Set the status of the axes for gyroscope
 * @param  enableX the status of the x axis to be set
 * @param  enableY the status of the y axis to be set
 * @param  enableZ the status of the z axis to be set
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS0_G_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ)
{
  uint8_t tmp1 = 0x00;
  uint8_t eX = 0x00;
  uint8_t eY = 0x00;
  uint8_t eZ = 0x00;
  
  eX = ( enableX == 0 ) ? LSM6DS0_G_XEN_DISABLE : LSM6DS0_G_XEN_ENABLE;
  eY = ( enableY == 0 ) ? LSM6DS0_G_YEN_DISABLE : LSM6DS0_G_YEN_ENABLE;
  eZ = ( enableZ == 0 ) ? LSM6DS0_G_ZEN_DISABLE : LSM6DS0_G_ZEN_ENABLE;
  
  if(LSM6DS0_IO_Read(&tmp1, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG4, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Enable X axis selection */
  tmp1 &= ~(LSM6DS0_G_XEN_MASK);
  tmp1 |= eX;
  
  /* Enable Y axis selection */
  tmp1 &= ~(LSM6DS0_G_YEN_MASK);
  tmp1 |= eY;
  
  /* Enable Z axis selection */
  tmp1 &= ~(LSM6DS0_G_ZEN_MASK);
  tmp1 |= eZ;
  
  if(LSM6DS0_IO_Write(&tmp1, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG4, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}


/**
 * @brief  Read data from LSM6DS0 Gyroscope and calculate angular rate in mdps
 * @param  pData the pointer where the gyroscope data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS0_G_GetAxes(int32_t *pData)
{
  int16_t pDataRaw[3];
  float sensitivity = 0;
  
  if(LSM6DS0_G_GetAxesRaw(pDataRaw) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS0_G_GetSensitivity( &sensitivity ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[0] = (int32_t)(pDataRaw[0] * sensitivity);
  pData[1] = (int32_t)(pDataRaw[1] * sensitivity);
  pData[2] = (int32_t)(pDataRaw[2] * sensitivity);
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read Accelero Output Data Rate
 * @param  odr the pointer where the accelerometer output data rate is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS0_X_Get_ODR( float *odr )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG6_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS0_XL_ODR_MASK;
  
  switch( tempReg )
  {
    case LSM6DS0_XL_ODR_PD:
      *odr = 0.0f;
      break;
    case LSM6DS0_XL_ODR_10HZ:
      *odr = 10.0f;
      break;
    case LSM6DS0_XL_ODR_50HZ:
      *odr = 50.0f;
      break;
    case LSM6DS0_XL_ODR_119HZ:
      *odr = 119.0f;
      break;
    case LSM6DS0_XL_ODR_238HZ:
      *odr = 238.0f;
      break;
    case LSM6DS0_XL_ODR_476HZ:
      *odr = 476.0f;
      break;
    case LSM6DS0_XL_ODR_952HZ:
      *odr = 952.0f;
      break;
    default:
      break;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Write Accelero Output Data Rate
 * @param  odr the accelerometer output data rate to be set
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS0_X_Set_ODR( float odr )
{
  uint8_t new_odr = 0x00;
  uint8_t tempReg = 0x00;
  
  new_odr = ( odr <= 0.0f   ) ? LSM6DS0_XL_ODR_PD          /* Power Down */
            : ( odr <= 10.0f  ) ? LSM6DS0_XL_ODR_10HZ
            : ( odr <= 50.0f  ) ? LSM6DS0_XL_ODR_50HZ
            : ( odr <= 119.0f ) ? LSM6DS0_XL_ODR_119HZ
            : ( odr <= 238.0f ) ? LSM6DS0_XL_ODR_238HZ
            : ( odr <= 476.0f ) ? LSM6DS0_XL_ODR_476HZ
            :                     LSM6DS0_XL_ODR_952HZ;
            
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG6_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= ~(LSM6DS0_XL_ODR_MASK);
  tempReg |= new_odr;
  
  if(LSM6DS0_IO_Write(&tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG6_XL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read Accelero Sensitivity
 * @param  pfData the pointer where the accelerometer sensitivity is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS0_X_GetSensitivity( float *pfData )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG6_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS0_XL_FS_MASK;
  
  switch( tempReg )
  {
    case LSM6DS0_XL_FS_2G:
      *pfData = 0.061f;
      break;
    case LSM6DS0_XL_FS_4G:
      *pfData = 0.122f;
      break;
    case LSM6DS0_XL_FS_8G:
      *pfData = 0.244f;
      break;
    case LSM6DS0_XL_FS_16G:
      *pfData = 0.732f;
      break;
    default:
      break;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read Accelero Full Scale
 * @param  fullScale the pointer where the accelerometer full scale is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS0_X_Get_FS( float *fullScale )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG6_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS0_XL_FS_MASK;
  
  switch( tempReg )
  {
    case LSM6DS0_XL_FS_2G:
      *fullScale = 2.0f;
      break;
    case LSM6DS0_XL_FS_4G:
      *fullScale = 4.0f;
      break;
    case LSM6DS0_XL_FS_8G:
      *fullScale = 8.0f;
      break;
    case LSM6DS0_XL_FS_16G:
      *fullScale = 16.0f;
      break;
    default:
      break;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Write Accelero Full Scale
 * @param  fullScale the accelerometer full scale to be set
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS0_X_Set_FS( float fullScale )
{
  uint8_t new_fs = 0x00;
  uint8_t tempReg = 0x00;
  
  new_fs = ( fullScale <= 2.0f ) ? LSM6DS0_XL_FS_2G
           : ( fullScale <= 4.0f ) ? LSM6DS0_XL_FS_4G
           : ( fullScale <= 8.0f ) ? LSM6DS0_XL_FS_8G
           :                         LSM6DS0_XL_FS_16G;
           
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG6_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= ~(LSM6DS0_XL_FS_MASK);
  tempReg |= new_fs;
  
  if(LSM6DS0_IO_Write(&tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG6_XL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read Gyro Output Data Rate
 * @param  odr the pointer where the gyroscope output data rate is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS0_G_Get_ODR( float *odr )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG1_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS0_G_ODR_MASK;
  
  switch( tempReg )
  {
    case LSM6DS0_G_ODR_PD:
      *odr = 0.0f;
      break;
    case LSM6DS0_G_ODR_14_9HZ:
      *odr = 14.9f;
      break;
    case LSM6DS0_G_ODR_59_5HZ:
      *odr = 59.5f;
      break;
    case LSM6DS0_G_ODR_119HZ:
      *odr = 119.0f;
      break;
    case LSM6DS0_G_ODR_238HZ:
      *odr = 238.0f;
      break;
    case LSM6DS0_G_ODR_476HZ:
      *odr = 476.0f;
      break;
    case LSM6DS0_G_ODR_952HZ:
      *odr = 952.0f;
      break;
    default:
      break;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Write Gyro Output Data Rate
 * @param  odr the gyroscope output data rate to be set
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS0_G_Set_ODR( float odr )
{
  uint8_t new_odr = 0x00;
  uint8_t tempReg = 0x00;
  
  new_odr = ( odr <= 0.0f   ) ? LSM6DS0_G_ODR_PD          /* Power Down */
            : ( odr <= 14.9f  ) ? LSM6DS0_G_ODR_14_9HZ
            : ( odr <= 59.5f  ) ? LSM6DS0_G_ODR_59_5HZ
            : ( odr <= 119.0f ) ? LSM6DS0_G_ODR_119HZ
            : ( odr <= 238.0f ) ? LSM6DS0_G_ODR_238HZ
            : ( odr <= 476.0f ) ? LSM6DS0_G_ODR_476HZ
            :                     LSM6DS0_G_ODR_952HZ;
            
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG1_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= ~(LSM6DS0_G_ODR_MASK);
  tempReg |= new_odr;
  
  if(LSM6DS0_IO_Write(&tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG1_G, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read Gyro Sensitivity
 * @param  pfData the pointer where the gyroscope sensitivity is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS0_G_GetSensitivity( float *pfData )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG1_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS0_G_FS_MASK;
  
  switch( tempReg )
  {
    case LSM6DS0_G_FS_245:
      *pfData = 8.75f;
      break;
    case LSM6DS0_G_FS_500:
      *pfData = 17.50f;
      break;
    case LSM6DS0_G_FS_2000:
      *pfData = 70.0f;
      break;
    default:
      break;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read Gyro Full Scale
 * @param  fullScale the pointer where the gyroscope full scale is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS0_G_Get_FS( float *fullScale )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG1_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS0_G_FS_MASK;
  
  switch( tempReg )
  {
    case LSM6DS0_G_FS_245:
      *fullScale = 245.0f;
      break;
    case LSM6DS0_G_FS_500:
      *fullScale = 500.0f;
      break;
    case LSM6DS0_G_FS_2000:
      *fullScale = 2000.0f;
      break;
    default:
      break;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Write Gyro Full Scale
 * @param  fullScale the gyroscope full scale to be set
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS0_G_Set_FS( float fullScale )
{
  uint8_t new_fs = 0x00;
  uint8_t tempReg = 0x00;
  
  new_fs = ( fullScale <= 245.0f ) ? LSM6DS0_G_FS_245
           : ( fullScale <= 500.0f ) ? LSM6DS0_G_FS_500
           :                           LSM6DS0_G_FS_2000;
           
  if(LSM6DS0_IO_Read( &tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG1_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= ~(LSM6DS0_G_FS_MASK);
  tempReg |= new_fs;
  
  if(LSM6DS0_IO_Write(&tempReg, LSM6DS0_XG_MEMS_ADDRESS, LSM6DS0_XG_CTRL_REG1_G, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
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
