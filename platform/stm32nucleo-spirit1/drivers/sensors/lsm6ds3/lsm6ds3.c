/**
 ******************************************************************************
 * @file    lsm6ds3.c
 * @author  MEMS Application Team
 * @version V1.2.0
 * @date    28-May-2015
 * @brief   This file provides a set of functions needed to manage the LSM6DS3 sensor
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
#include "lsm6ds3.h"
#include <math.h>

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup LSM6DS3
 * @{
 */

static IMU_6AXES_StatusTypeDef    LSM6DS3_Init( IMU_6AXES_InitTypeDef *LSM6DS3_Init );
static IMU_6AXES_StatusTypeDef    LSM6DS3_Read_XG_ID( uint8_t *xg_id);
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_GetAxes( int32_t *pData );
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_GetAxesRaw(int16_t *pData);
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_GetAxes( int32_t *pData );
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_GetAxesRaw(int16_t *pData);
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Get_ODR( float *odr );
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Set_ODR( float odr );
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_GetSensitivity( float *pfData );
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Get_FS( float *fullScale );
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Set_FS( float fullScale );
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Get_ODR( float *odr );
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Set_ODR( float odr );
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_GetSensitivity( float *pfData );
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Get_FS( float *fullScale );
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Set_FS( float fullScale );
static IMU_6AXES_StatusTypeDef    LSM6DS3_Enable_Free_Fall_Detection( void );
static IMU_6AXES_StatusTypeDef    LSM6DS3_Disable_Free_Fall_Detection( void );
static IMU_6AXES_StatusTypeDef    LSM6DS3_Get_Status_Free_Fall_Detection( uint8_t *status );

/** @addtogroup LSM6DS3_Private_Variables LSM6DS3_Private_Variables
 * @{
 */
IMU_6AXES_DrvTypeDef LSM6DS3Drv =
{
  LSM6DS3_Init,
  LSM6DS3_Read_XG_ID,
  LSM6DS3_X_GetAxes,
  LSM6DS3_X_GetAxesRaw,
  LSM6DS3_G_GetAxes,
  LSM6DS3_G_GetAxesRaw,
  LSM6DS3_X_Get_ODR,
  LSM6DS3_X_Set_ODR,
  LSM6DS3_X_GetSensitivity,
  LSM6DS3_X_Get_FS,
  LSM6DS3_X_Set_FS,
  LSM6DS3_G_Get_ODR,
  LSM6DS3_G_Set_ODR,
  LSM6DS3_G_GetSensitivity,
  LSM6DS3_G_Get_FS,
  LSM6DS3_G_Set_FS,
  NULL
};

LSM6DS3_DrvExtTypeDef LSM6DS3Drv_ext_internal =
{

  LSM6DS3_Enable_Free_Fall_Detection,
  LSM6DS3_Disable_Free_Fall_Detection,
  LSM6DS3_Get_Status_Free_Fall_Detection
};

IMU_6AXES_DrvExtTypeDef LSM6DS3Drv_ext =
{
  IMU_6AXES_LSM6DS3_DIL24_COMPONENT, /* unique ID for LSM6DS3 in the IMU 6-axes driver class */
  &LSM6DS3Drv_ext_internal /* pointer to internal struct of extended features of LSM6DS3 */
};

/**
 * @}
 */
static IMU_6AXES_StatusTypeDef LSM6DS3_Common_Sensor_Enable(void);
static IMU_6AXES_StatusTypeDef LSM6DS3_X_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ);
static IMU_6AXES_StatusTypeDef LSM6DS3_G_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ);

/** @addtogroup LSM6DS3_Private_Functions LSM6DS3_Private_Functions
 * @{
 */

/**
 * @brief  Set LSM6DS3 Initialization
 * @param  LSM6DS3_Init the configuration setting for the LSM6DS3
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS3_Init( IMU_6AXES_InitTypeDef *LSM6DS3_Init )
{
  /*Here we have to add the check if the parameters are valid*/
  
  /* Configure the low level interface -------------------------------------*/
  if(LSM6DS3_IO_Init() != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  
  /******** Common init *********/
  
  if(LSM6DS3_Common_Sensor_Enable() != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  
  /******* Gyroscope init *******/
  
  if(LSM6DS3_G_Set_ODR( LSM6DS3_Init->G_OutputDataRate ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_G_Set_FS( LSM6DS3_Init->G_FullScale ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_G_Set_Axes_Status(LSM6DS3_Init->G_X_Axis, LSM6DS3_Init->G_Y_Axis, LSM6DS3_Init->G_Z_Axis) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  
  /***** Accelerometer init *****/
  
  if(LSM6DS3_X_Set_ODR( LSM6DS3_Init->X_OutputDataRate ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_X_Set_FS( LSM6DS3_Init->X_FullScale ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_X_Set_Axes_Status(LSM6DS3_Init->X_X_Axis, LSM6DS3_Init->X_Y_Axis, LSM6DS3_Init->X_Z_Axis) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Configure interrupt lines */
  LSM6DS3_IO_ITConfig();
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read ID of LSM6DS3 Accelerometer and Gyroscope
 * @param  xg_id the pointer where the ID of the device is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS3_Read_XG_ID( uint8_t *xg_id)
{
  if(!xg_id)
  {
    return IMU_6AXES_ERROR;
  }
  
  return LSM6DS3_IO_Read(xg_id, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WHO_AM_I_ADDR, 1);
}

/**
 * @brief  Set LSM6DS3 common initialization
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS3_Common_Sensor_Enable(void)
{
  uint8_t tmp1 = 0x00;
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL3_C, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Enable register address automatically incremented during a multiple byte
     access with a serial interface (I2C or SPI) */
  tmp1 &= ~(LSM6DS3_XG_IF_INC_MASK);
  tmp1 |= LSM6DS3_XG_IF_INC;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL3_C, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_FIFO_CTRL5, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* FIFO ODR selection */
  tmp1 &= ~(LSM6DS3_XG_FIFO_ODR_MASK);
  tmp1 |= LSM6DS3_XG_FIFO_ODR_NA;
  
  /* FIFO mode selection */
  tmp1 &= ~(LSM6DS3_XG_FIFO_MODE_MASK);
  tmp1 |= LSM6DS3_XG_FIFO_MODE_BYPASS;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_FIFO_CTRL5, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read raw data from LSM6DS3 Accelerometer output register
 * @param  pData the pointer where the accelerometer raw data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS3_X_GetAxesRaw( int16_t *pData )
{
  /*Here we have to add the check if the parameters are valid*/
  
  uint8_t tempReg[2] = {0, 0};
  
  
  if(LSM6DS3_IO_Read(&tempReg[0], LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_OUT_X_L_XL, 2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[0] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS3_IO_Read(&tempReg[0], LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_OUT_Y_L_XL, 2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[1] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS3_IO_Read(&tempReg[0], LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_OUT_Z_L_XL, 2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[2] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  return IMU_6AXES_OK;
}



/**
 * @brief  Read data from LSM6DS3 Accelerometer and calculate linear acceleration in mg
 * @param  pData the pointer where the accelerometer data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_GetAxes( int32_t *pData )
{
  /*Here we have to add the check if the parameters are valid*/
  int16_t pDataRaw[3];
  float sensitivity = 0.0f;
  
  if(LSM6DS3_X_GetAxesRaw(pDataRaw) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_X_GetSensitivity( &sensitivity ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[0] = (int32_t)(pDataRaw[0] * sensitivity);
  pData[1] = (int32_t)(pDataRaw[1] * sensitivity);
  pData[2] = (int32_t)(pDataRaw[2] * sensitivity);
  
  return IMU_6AXES_OK;
}



/**
 * @brief  Read raw data from LSM6DS3 Gyroscope output register
 * @param  pData the pointer where the gyroscope raw data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef LSM6DS3_G_GetAxesRaw( int16_t *pData )
{
  /*Here we have to add the check if the parameters are valid*/
  
  uint8_t tempReg[2] = {0, 0};
  
  
  if(LSM6DS3_IO_Read(&tempReg[0], LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_OUT_X_L_G, 2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[0] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS3_IO_Read(&tempReg[0], LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_OUT_Y_L_G, 2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  pData[1] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);
  
  if(LSM6DS3_IO_Read(&tempReg[0], LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_OUT_Z_L_G, 2) != IMU_6AXES_OK)
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
static IMU_6AXES_StatusTypeDef LSM6DS3_X_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ)
{
  uint8_t tmp1 = 0x00;
  uint8_t eX = 0x00;
  uint8_t eY = 0x00;
  uint8_t eZ = 0x00;
  
  eX = ( enableX == 0 ) ? LSM6DS3_XL_XEN_DISABLE : LSM6DS3_XL_XEN_ENABLE;
  eY = ( enableY == 0 ) ? LSM6DS3_XL_YEN_DISABLE : LSM6DS3_XL_YEN_ENABLE;
  eZ = ( enableZ == 0 ) ? LSM6DS3_XL_ZEN_DISABLE : LSM6DS3_XL_ZEN_ENABLE;
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL9_XL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Enable X axis selection */
  tmp1 &= ~(LSM6DS3_XL_XEN_MASK);
  tmp1 |= eX;
  
  /* Enable Y axis selection */
  tmp1 &= ~(LSM6DS3_XL_YEN_MASK);
  tmp1 |= eY;
  
  /* Enable Z axis selection */
  tmp1 &= ~(LSM6DS3_XL_ZEN_MASK);
  tmp1 |= eZ;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL9_XL, 1) != IMU_6AXES_OK)
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
static IMU_6AXES_StatusTypeDef LSM6DS3_G_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ)
{
  uint8_t tmp1 = 0x00;
  uint8_t eX = 0x00;
  uint8_t eY = 0x00;
  uint8_t eZ = 0x00;
  
  eX = ( enableX == 0 ) ? LSM6DS3_G_XEN_DISABLE : LSM6DS3_G_XEN_ENABLE;
  eY = ( enableY == 0 ) ? LSM6DS3_G_YEN_DISABLE : LSM6DS3_G_YEN_ENABLE;
  eZ = ( enableZ == 0 ) ? LSM6DS3_G_ZEN_DISABLE : LSM6DS3_G_ZEN_ENABLE;
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL10_C, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Enable X axis selection */
  tmp1 &= ~(LSM6DS3_G_XEN_MASK);
  tmp1 |= eX;
  
  /* Enable Y axis selection */
  tmp1 &= ~(LSM6DS3_G_YEN_MASK);
  tmp1 |= eY;
  
  /* Enable Z axis selection */
  tmp1 &= ~(LSM6DS3_G_ZEN_MASK);
  tmp1 |= eZ;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL10_C, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read data from LSM6DS3 Gyroscope and calculate angular rate in mdps
 * @param  pData the pointer where the gyroscope data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_GetAxes( int32_t *pData )
{
  /*Here we have to add the check if the parameters are valid*/
  int16_t pDataRaw[3];
  float sensitivity = 0.0f;
  
  if(LSM6DS3_G_GetAxesRaw(pDataRaw) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_G_GetSensitivity( &sensitivity ) != IMU_6AXES_OK)
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Get_ODR( float *odr )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS3_XL_ODR_MASK;
  
  switch( tempReg )
  {
    case LSM6DS3_XL_ODR_PD:
      *odr = 0.0f;
      break;
    case LSM6DS3_XL_ODR_13HZ:
      *odr = 13.0f;
      break;
    case LSM6DS3_XL_ODR_26HZ:
      *odr = 26.0f;
      break;
    case LSM6DS3_XL_ODR_52HZ:
      *odr = 52.0f;
      break;
    case LSM6DS3_XL_ODR_104HZ:
      *odr = 104.0f;
      break;
    case LSM6DS3_XL_ODR_208HZ:
      *odr = 208.0f;
      break;
    case LSM6DS3_XL_ODR_416HZ:
      *odr = 416.0f;
      break;
    case LSM6DS3_XL_ODR_833HZ:
      *odr = 833.0f;
      break;
    case LSM6DS3_XL_ODR_1660HZ:
      *odr = 1660.0f;
      break;
    case LSM6DS3_XL_ODR_3330HZ:
      *odr = 3330.0f;
      break;
    case LSM6DS3_XL_ODR_6660HZ:
      *odr = 6660.0f;
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Set_ODR( float odr )
{
  uint8_t new_odr = 0x00;
  uint8_t tempReg = 0x00;
  
  new_odr = ( odr <= 0.0f    ) ? LSM6DS3_XL_ODR_PD          /* Power Down */
            : ( odr <= 13.0f   ) ? LSM6DS3_XL_ODR_13HZ
            : ( odr <= 26.0f   ) ? LSM6DS3_XL_ODR_26HZ
            : ( odr <= 52.0f   ) ? LSM6DS3_XL_ODR_52HZ
            : ( odr <= 104.0f  ) ? LSM6DS3_XL_ODR_104HZ
            : ( odr <= 208.0f  ) ? LSM6DS3_XL_ODR_208HZ
            : ( odr <= 416.0f  ) ? LSM6DS3_XL_ODR_416HZ
            : ( odr <= 833.0f  ) ? LSM6DS3_XL_ODR_833HZ
            : ( odr <= 1660.0f ) ? LSM6DS3_XL_ODR_1660HZ
            : ( odr <= 3330.0f ) ? LSM6DS3_XL_ODR_3330HZ
            :                      LSM6DS3_XL_ODR_6660HZ;
            
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= ~(LSM6DS3_XL_ODR_MASK);
  tempReg |= new_odr;
  
  if(LSM6DS3_IO_Write(&tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1) != IMU_6AXES_OK)
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_GetSensitivity( float *pfData )
{
  /*Here we have to add the check if the parameters are valid*/
  
  uint8_t tempReg = 0x00;
  
  
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS3_XL_FS_MASK;
  
  switch( tempReg )
  {
    case LSM6DS3_XL_FS_2G:
      *pfData = 0.061f;
      break;
    case LSM6DS3_XL_FS_4G:
      *pfData = 0.122f;
      break;
    case LSM6DS3_XL_FS_8G:
      *pfData = 0.244f;
      break;
    case LSM6DS3_XL_FS_16G:
      *pfData = 0.488f;
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Get_FS( float *fullScale )
{
  /*Here we have to add the check if the parameters are valid*/
  
  uint8_t tempReg = 0x00;
  
  
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS3_XL_FS_MASK;
  
  switch( tempReg )
  {
    case LSM6DS3_XL_FS_2G:
      *fullScale = 2.0f;
      break;
    case LSM6DS3_XL_FS_4G:
      *fullScale = 4.0f;
      break;
    case LSM6DS3_XL_FS_8G:
      *fullScale = 8.0f;
      break;
    case LSM6DS3_XL_FS_16G:
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_X_Set_FS( float fullScale )
{
  uint8_t new_fs = 0x00;
  uint8_t tempReg = 0x00;
  
  new_fs = ( fullScale <= 2.0f ) ? LSM6DS3_XL_FS_2G
           : ( fullScale <= 4.0f ) ? LSM6DS3_XL_FS_4G
           : ( fullScale <= 8.0f ) ? LSM6DS3_XL_FS_8G
           :                         LSM6DS3_XL_FS_16G;
           
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= ~(LSM6DS3_XL_FS_MASK);
  tempReg |= new_fs;
  
  if(LSM6DS3_IO_Write(&tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1) != IMU_6AXES_OK)
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Get_ODR( float *odr )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;
  
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS3_G_ODR_MASK;
  
  switch( tempReg )
  {
    case LSM6DS3_G_ODR_PD:
      *odr = 0.0f;
      break;
    case LSM6DS3_G_ODR_13HZ:
      *odr = 13.0f;
      break;
    case LSM6DS3_G_ODR_26HZ:
      *odr = 26.0f;
      break;
    case LSM6DS3_G_ODR_52HZ:
      *odr = 52.0f;
      break;
    case LSM6DS3_G_ODR_104HZ:
      *odr = 104.0f;
      break;
    case LSM6DS3_G_ODR_208HZ:
      *odr = 208.0f;
      break;
    case LSM6DS3_G_ODR_416HZ:
      *odr = 416.0f;
      break;
    case LSM6DS3_G_ODR_833HZ:
      *odr = 833.0f;
      break;
    case LSM6DS3_G_ODR_1660HZ:
      *odr = 1660.0f;
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Set_ODR( float odr )
{
  uint8_t new_odr = 0x00;
  uint8_t tempReg = 0x00;
  
  new_odr = ( odr <= 0.0f   ) ? LSM6DS3_G_ODR_PD          /* Power Down */
            : ( odr <= 13.0f  ) ? LSM6DS3_G_ODR_13HZ
            : ( odr <= 26.0f  ) ? LSM6DS3_G_ODR_26HZ
            : ( odr <= 52.0f  ) ? LSM6DS3_G_ODR_52HZ
            : ( odr <= 104.0f ) ? LSM6DS3_G_ODR_104HZ
            : ( odr <= 208.0f ) ? LSM6DS3_G_ODR_208HZ
            : ( odr <= 416.0f ) ? LSM6DS3_G_ODR_416HZ
            : ( odr <= 833.0f ) ? LSM6DS3_G_ODR_833HZ
            :                     LSM6DS3_G_ODR_1660HZ;
            
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= ~(LSM6DS3_G_ODR_MASK);
  tempReg |= new_odr;
  
  if(LSM6DS3_IO_Write(&tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1) != IMU_6AXES_OK)
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
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_GetSensitivity( float *pfData )
{
  /*Here we have to add the check if the parameters are valid*/
  
  uint8_t tempReg = 0x00;
  
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS3_G_FS_125_MASK;
  
  if(tempReg == LSM6DS3_G_FS_125_ENABLE)
  {
    *pfData = 4.375f;
  }
  else
  {
    if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
    
    tempReg &= LSM6DS3_G_FS_MASK;
    
    switch( tempReg )
    {
      case LSM6DS3_G_FS_245:
        *pfData = 8.75f;
        break;
      case LSM6DS3_G_FS_500:
        *pfData = 17.50f;
        break;
      case LSM6DS3_G_FS_1000:
        *pfData = 35.0f;
        break;
      case LSM6DS3_G_FS_2000:
        *pfData = 70.0f;
        break;
      default:
        break;
    }
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Read Gyro Full Scale
 * @param  fullScale the pointer where the gyroscope full scale is stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Get_FS( float *fullScale )
{
  /*Here we have to add the check if the parameters are valid*/
  
  uint8_t tempReg = 0x00;
  
  if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tempReg &= LSM6DS3_G_FS_125_MASK;
  
  if(tempReg == LSM6DS3_G_FS_125_ENABLE)
  {
    *fullScale = 125.0f;
  }
  else
  {
    if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
    
    tempReg &= LSM6DS3_G_FS_MASK;
    
    switch( tempReg )
    {
      case LSM6DS3_G_FS_245:
        *fullScale = 245.0f;
        break;
      case LSM6DS3_G_FS_500:
        *fullScale = 500.0f;
        break;
      case LSM6DS3_G_FS_1000:
        *fullScale = 1000.0f;
        break;
      case LSM6DS3_G_FS_2000:
        *fullScale = 2000.0f;
        break;
      default:
        break;
    }
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Write Gyro Full Scale
 * @param  fullScale the gyroscope full scale to be set
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS3_G_Set_FS( float fullScale )
{
  uint8_t new_fs = 0x00;
  uint8_t tempReg = 0x00;
  
  if(fullScale <= 125.0f)
  {
    new_fs = LSM6DS3_G_FS_125_ENABLE;
    
    if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
    
    tempReg &= ~(LSM6DS3_G_FS_125_MASK);
    tempReg |= new_fs;
    
    if(LSM6DS3_IO_Write(&tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
  }
  else
  {
    /* Disable G FS 125dpp  */
    if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
    
    tempReg &= ~(LSM6DS3_G_FS_125_MASK);
    tempReg |= LSM6DS3_G_FS_125_DISABLE;
    
    if(LSM6DS3_IO_Write(&tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
    
    new_fs = ( fullScale <= 245.0f )  ? LSM6DS3_G_FS_245
             : ( fullScale <= 500.0f )  ? LSM6DS3_G_FS_500
             : ( fullScale <= 1000.0f ) ? LSM6DS3_G_FS_1000
             :                            LSM6DS3_G_FS_2000;
             
    if(LSM6DS3_IO_Read( &tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1 ) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
    
    tempReg &= ~(LSM6DS3_G_FS_MASK);
    tempReg |= new_fs;
    
    if(LSM6DS3_IO_Write(&tempReg, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL2_G, 1) != IMU_6AXES_OK)
    {
      return IMU_6AXES_ERROR;
    }
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Enable free fall detection
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS3_Enable_Free_Fall_Detection( void )
{
  uint8_t tmp1 = 0x00;
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* Output Data Rate selection */
  tmp1 &= ~(LSM6DS3_XL_ODR_MASK);
  tmp1 |= LSM6DS3_XL_ODR_416HZ;
  
  /* Full scale selection */
  tmp1 &= ~(LSM6DS3_XL_FS_MASK);
  tmp1 |= LSM6DS3_XL_FS_2G;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_CTRL1_XL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WAKE_UP_DUR, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* FF_DUR5 setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_UP_DUR_FF_DUR5_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_UP_DUR_FF_DUR5_DEFAULT;
  
  /* WAKE_DUR setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_UP_DUR_WAKE_DUR_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_UP_DUR_WAKE_DUR_DEFAULT;
  
  /* TIMER_HR setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_UP_DUR_TIMER_HR_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_UP_DUR_TIMER_HR_DEFAULT;
  
  /* SLEEP_DUR setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_UP_DUR_SLEEP_DUR_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_UP_DUR_SLEEP_DUR_DEFAULT;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WAKE_UP_DUR, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WAKE_FREE_FALL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* FF_DUR setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_FREE_FALL_FF_DUR_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_FREE_FALL_FF_DUR_TYPICAL;
  
  /* FF_THS setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_312MG;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WAKE_FREE_FALL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_MD1_CFG, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* INT1_FF setting */
  tmp1 &= ~(LSM6DS3_XG_MD1_CFG_INT1_FF_MASK);
  tmp1 |= LSM6DS3_XG_MD1_CFG_INT1_FF_ENABLE;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_MD1_CFG, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Disable free fall detection
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS3_Disable_Free_Fall_Detection( void )
{
  uint8_t tmp1 = 0x00;
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_MD1_CFG, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* INT1_FF setting */
  tmp1 &= ~(LSM6DS3_XG_MD1_CFG_INT1_FF_MASK);
  tmp1 |= LSM6DS3_XG_MD1_CFG_INT1_FF_DISABLE;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_MD1_CFG, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WAKE_FREE_FALL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  /* FF_DUR setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_FREE_FALL_FF_DUR_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_FREE_FALL_FF_DUR_DEFAULT;
  
  /* FF_THS setting */
  tmp1 &= ~(LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_MASK);
  tmp1 |= LSM6DS3_XG_WAKE_FREE_FALL_FF_THS_156MG;
  
  if(LSM6DS3_IO_Write(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WAKE_FREE_FALL, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Get status of free fall detection
 * @param  status the pointer where the status of free fall detection is stored; 0 means no detection, 1 means detection happened
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
*/
static IMU_6AXES_StatusTypeDef    LSM6DS3_Get_Status_Free_Fall_Detection( uint8_t *status )
{
  uint8_t tmp1 = 0x00;
  
  if(LSM6DS3_IO_Read(&tmp1, LSM6DS3_XG_MEMS_ADDRESS, LSM6DS3_XG_WAKE_UP_SRC, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  tmp1 &= LSM6DS3_XG_WAKE_UP_SRC_FF_IA_MASK;
  
  switch( tmp1 )
  {
    case LSM6DS3_XG_WAKE_UP_SRC_FF_IA_ENABLE:
      *status = 1;
      break;
    case LSM6DS3_XG_WAKE_UP_SRC_FF_IA_DISABLE:
    default:
      *status = 0;
      break;
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
