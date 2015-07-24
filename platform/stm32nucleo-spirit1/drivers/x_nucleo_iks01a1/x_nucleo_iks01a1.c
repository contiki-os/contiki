/**
 ******************************************************************************
 * @file    x_nucleo_iks01a1.c
 * @author  CL
 * @version V1.3.0
 * @date    28-May-2015
 * @brief   This file provides X_NUCLEO_IKS01A1 MEMS shield board specific functions
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
#include "x_nucleo_iks01a1.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1
 * @{
 */

/** @defgroup X_NUCLEO_IKS01A1_Private_Defines X_NUCLEO_IKS01A1_Private_Defines
 * @{
 */
#ifndef NULL
#define NULL      (void *) 0
#endif
/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_Private_Variables X_NUCLEO_IKS01A1_Private_Variables
 * @{
 */

uint32_t I2C_EXPBD_Timeout = NUCLEO_I2C_EXPBD_TIMEOUT_MAX;    /*<! Value of Timeout when I2C communication fails */

static I2C_HandleTypeDef    I2C_EXPBD_Handle;

/**
 * @}
 */


/* Link function for 6 Axes IMU peripheral */
IMU_6AXES_StatusTypeDef LSM6DS0_IO_Init( void );
void LSM6DS0_IO_ITConfig( void );
IMU_6AXES_StatusTypeDef LSM6DS3_IO_Init( void );
void LSM6DS3_IO_ITConfig( void );
IMU_6AXES_StatusTypeDef LSM6DS0_IO_Write( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite );
IMU_6AXES_StatusTypeDef LSM6DS3_IO_Write( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite );
IMU_6AXES_StatusTypeDef LSM6DS0_IO_Read( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead );
IMU_6AXES_StatusTypeDef LSM6DS3_IO_Read( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead );

/* Link function for MAGNETO peripheral */
MAGNETO_StatusTypeDef LIS3MDL_IO_Init(void);
void LIS3MDL_IO_ITConfig( void );
MAGNETO_StatusTypeDef LIS3MDL_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToWrite);
MAGNETO_StatusTypeDef LIS3MDL_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                      uint16_t NumByteToRead);

/* Link function for PRESSURE peripheral */
PRESSURE_StatusTypeDef LPS25H_IO_Init(void);
void LPS25H_IO_ITConfig( void );
PRESSURE_StatusTypeDef LPS25HB_IO_Init(void);
void LPS25HB_IO_ITConfig( void );
PRESSURE_StatusTypeDef LPS25H_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToWrite);
PRESSURE_StatusTypeDef LPS25HB_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                        uint16_t NumByteToWrite);
PRESSURE_StatusTypeDef LPS25H_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                      uint16_t NumByteToRead);
PRESSURE_StatusTypeDef LPS25HB_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToRead);

/* Link function for HUM_TEMP peripheral */
HUM_TEMP_StatusTypeDef HTS221_IO_Init(void);
void HTS221_IO_ITConfig( void );
HUM_TEMP_StatusTypeDef HTS221_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToWrite);
HUM_TEMP_StatusTypeDef HTS221_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                      uint16_t NumByteToRead);

static IMU_6AXES_StatusTypeDef IMU_6AXES_IO_Init(void);
static IMU_6AXES_StatusTypeDef IMU_6AXES_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite);
static IMU_6AXES_StatusTypeDef IMU_6AXES_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead);
static MAGNETO_StatusTypeDef MAGNETO_IO_Init(void);
static MAGNETO_StatusTypeDef MAGNETO_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite);
static MAGNETO_StatusTypeDef MAGNETO_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead);
static PRESSURE_StatusTypeDef PRESSURE_IO_Init(void);
static PRESSURE_StatusTypeDef PRESSURE_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite);
static PRESSURE_StatusTypeDef PRESSURE_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead);
static HUM_TEMP_StatusTypeDef HUM_TEMP_IO_Init(void);
static HUM_TEMP_StatusTypeDef HUM_TEMP_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite);
static HUM_TEMP_StatusTypeDef HUM_TEMP_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead);
static void I2C_EXPBD_MspInit(void);
static void I2C_EXPBD_Error(uint8_t Addr);
static HAL_StatusTypeDef I2C_EXPBD_Init(void);
static HAL_StatusTypeDef I2C_EXPBD_WriteData(uint8_t* pBuffer, uint8_t Addr, uint8_t Reg, uint16_t Size);
static HAL_StatusTypeDef I2C_EXPBD_ReadData(uint8_t* pBuffer, uint8_t Addr, uint8_t Reg, uint16_t Size);

/** @defgroup X_NUCLEO_IKS01A1_Exported_Functions X_NUCLEO_IKS01A1_Exported_Functions
 * @{
 */



/********************************* LINK IMU 6 AXES *****************************/
/**
 * @brief  Configures LSM6DS0 I2C interface
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS0_IO_Init( void )
{
  return IMU_6AXES_IO_Init();
}

/**
 * @brief  Configures LSM6DS0 interrupt lines for NUCLEO boards
 * @retval None
 */
void LSM6DS0_IO_ITConfig( void )
{
  /* To be implemented */
}

/**
 * @brief  Configures LSM6DS3 I2C interface
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS3_IO_Init( void )
{
  return IMU_6AXES_IO_Init();
}


/**
 * @brief  Configures LSM6DS3 interrupt lines for NUCLEO boards
 * @retval None
 */
void LSM6DS3_IO_ITConfig( void )
{
  GPIO_InitTypeDef GPIO_InitStructureInt1;
  GPIO_InitTypeDef GPIO_InitStructureInt2;
  /* Enable INT1 GPIO clock */
  MEMS_INT1_GPIO_CLK_ENABLE();
  
  /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStructureInt1.Pin = MEMS_INT1_PIN;
  GPIO_InitStructureInt1.Mode = GPIO_MODE_IT_RISING;
#if ((defined (USE_STM32F4XX_NUCLEO)) || (defined (USE_STM32L0XX_NUCLEO)))
  GPIO_InitStructureInt1.Speed = GPIO_SPEED_FAST;
#endif
  
#if (defined (USE_STM32L1XX_NUCLEO))
  GPIO_InitStructureInt1.Speed = GPIO_SPEED_MEDIUM;
#endif
  GPIO_InitStructureInt1.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT1_GPIO_PORT, &GPIO_InitStructureInt1);
  
  /* Enable and set EXTI Interrupt priority */
  HAL_NVIC_SetPriority(MEMS_INT1_EXTI_IRQn, 0x00, 0x00);
  HAL_NVIC_EnableIRQ(MEMS_INT1_EXTI_IRQn);
  
  /* Enable INT2 GPIO clock */
  MEMS_INT2_GPIO_CLK_ENABLE();
  
  /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStructureInt2.Pin = MEMS_INT2_PIN;
  GPIO_InitStructureInt2.Mode = GPIO_MODE_IT_RISING;
#if ((defined (USE_STM32F4XX_NUCLEO)) || (defined (USE_STM32L0XX_NUCLEO)))
  GPIO_InitStructureInt2.Speed = GPIO_SPEED_FAST;
#endif
  
#if (defined (USE_STM32L1XX_NUCLEO))
  GPIO_InitStructureInt2.Speed = GPIO_SPEED_MEDIUM;
#endif
  GPIO_InitStructureInt2.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_PORT, &GPIO_InitStructureInt2);
  
  /* Enable and set EXTI Interrupt priority */
  HAL_NVIC_SetPriority(MEMS_INT2_EXTI_IRQn, 0x00, 0x00);
  HAL_NVIC_EnableIRQ(MEMS_INT2_EXTI_IRQn);
}

/**
 * @brief  Writes a buffer to the LSM6DS0 sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the IMU 6 axes internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS0_IO_Write( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite )
{
  return IMU_6AXES_IO_Write( pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite );
}

/**
 * @brief  Writes a buffer to the LSM6DS3 sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the IMU 6 axes internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS3_IO_Write(  uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite )
{
  return IMU_6AXES_IO_Write( pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite );
}

/**
 * @brief  Reads a buffer from the LSM6DS0 sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the IMU 6 axes internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS0_IO_Read( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead )
{
  return IMU_6AXES_IO_Read( pBuffer, DeviceAddr, RegisterAddr, NumByteToRead );
}

/**
 * @brief  Reads a buffer from the LSM6DS3 sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the IMU 6 axes internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS3_IO_Read( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead )
{
  return IMU_6AXES_IO_Read( pBuffer, DeviceAddr, RegisterAddr, NumByteToRead );
}


/********************************* LINK MAGNETO *****************************/
/**
 * @brief  Configures LIS3MDL I2C interface
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
MAGNETO_StatusTypeDef LIS3MDL_IO_Init(void)
{
  return MAGNETO_IO_Init();
}

/**
 * @brief  Configures LIS3MDL interrupt lines for NUCLEO boards
 * @retval None
 */
void LIS3MDL_IO_ITConfig( void )
{
  /* To be implemented */
}

/**
 * @brief  Writes a buffer to the LIS3MDL sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the magneto internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
MAGNETO_StatusTypeDef LIS3MDL_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToWrite)
{
  return MAGNETO_IO_Write(pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite);
}

/**
 * @brief  Reads a buffer from the LIS3MDL sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the magneto internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
MAGNETO_StatusTypeDef LIS3MDL_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                      uint16_t NumByteToRead)
{
  return MAGNETO_IO_Read(pBuffer, DeviceAddr, RegisterAddr, NumByteToRead);
}



/********************************* LINK PRESSURE *****************************/
/**
 * @brief  Configures LPS25H I2C interface
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H_IO_Init(void)
{
  return PRESSURE_IO_Init();
}

/**
 * @brief  Configures LPS25H interrupt lines for NUCLEO boards
 * @retval None
 */
void LPS25H_IO_ITConfig( void )
{
  /* To be implemented */
}

/**
 * @brief  Configures LPS25HB I2C interface
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25HB_IO_Init(void)
{
  return PRESSURE_IO_Init();
}

/**
 * @brief  Configures LPS25HB interrupt lines for NUCLEO boards
 * @retval None
 */
void LPS25HB_IO_ITConfig( void )
{
  /* To be implemented */
}

/**
 * @brief  Writes a buffer to the LPS25H sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the pressure internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToWrite)
{
  return PRESSURE_IO_Write(pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite);
}

/**
 * @brief  Writes a buffer to the LPS25HB sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the pressure internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25HB_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                        uint16_t NumByteToWrite)
{
  return PRESSURE_IO_Write(pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite);
}

/**
 * @brief  Reads a buffer from the LPS25H sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the pressure internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25H_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                      uint16_t NumByteToRead)
{
  return PRESSURE_IO_Read(pBuffer, DeviceAddr, RegisterAddr, NumByteToRead);
}

/**
 * @brief  Reads a buffer from the LPS25HB sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the pressure internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
PRESSURE_StatusTypeDef LPS25HB_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToRead)
{
  return PRESSURE_IO_Read(pBuffer, DeviceAddr, RegisterAddr, NumByteToRead);
}

/********************************* LINK HUM_TEMP *****************************/
/**
 * @brief  Configures HTS221 I2C interface
 * @retval HUM_TEMP_OK in case of success, an error code otherwise
 */
HUM_TEMP_StatusTypeDef HTS221_IO_Init(void)
{
  return HUM_TEMP_IO_Init();
}

/**
 * @brief  Configures HTS221 interrupt lines for NUCLEO boards
 * @retval None
 */
void HTS221_IO_ITConfig( void )
{
  /* To be implemented */
}

/**
 * @brief  Writes a buffer to the HTS221 sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the humidity and temperature internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval HUM_TEMP_OK in case of success, an error code otherwise
 */
HUM_TEMP_StatusTypeDef HTS221_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                       uint16_t NumByteToWrite)
{
  return HUM_TEMP_IO_Write(pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite);
}

/**
 * @brief  Reads a buffer from the HTS221 sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the humidity and temperature internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval HUM_TEMP_OK in case of success, an error code otherwise
 */
HUM_TEMP_StatusTypeDef HTS221_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
                                      uint16_t NumByteToRead)
{
  return HUM_TEMP_IO_Read(pBuffer, DeviceAddr, RegisterAddr, NumByteToRead);
}

/**
 * @}
 */

/** @defgroup X_NUCLEO_IKS01A1_Private_Functions X_NUCLEO_IKS01A1_Private_Functions
 * @{
 */

/**
 * @brief  Configures Imu 6 axes I2C interface
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef IMU_6AXES_IO_Init( void )
{
  if(I2C_EXPBD_Init() != HAL_OK)
  {
    return IMU_6AXES_ERROR;
  }
  
  return IMU_6AXES_OK;
}

/**
 * @brief  Writes a buffer to the IMU 6 axes sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the IMU 6 axes internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef IMU_6AXES_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite)
{
  IMU_6AXES_StatusTypeDef ret_val = IMU_6AXES_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_WriteData( pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite ) != HAL_OK)
  {
    ret_val = IMU_6AXES_ERROR;
  }
  
  return ret_val;
}

/**
 * @brief  Reads a buffer from the IMU 6 axes sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the IMU 6 axes internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
static IMU_6AXES_StatusTypeDef IMU_6AXES_IO_Read( uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead )
{
  IMU_6AXES_StatusTypeDef ret_val = IMU_6AXES_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_ReadData( pBuffer, DeviceAddr, RegisterAddr, NumByteToRead ) != HAL_OK)
  {
    ret_val = IMU_6AXES_ERROR;
  }
  
  return ret_val;
}


/**
 * @brief  Configures magneto I2C interface
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
static MAGNETO_StatusTypeDef MAGNETO_IO_Init(void)
{
  if(I2C_EXPBD_Init() != HAL_OK)
  {
    return MAGNETO_ERROR;
  }
  
  return MAGNETO_OK;
}

/**
 * @brief  Writes a buffer to the magneto sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the magneto internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
static MAGNETO_StatusTypeDef MAGNETO_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite)
{
  MAGNETO_StatusTypeDef ret_val = MAGNETO_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_WriteData(pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite) != HAL_OK)
  {
    ret_val = MAGNETO_ERROR;
  }
  
  return ret_val;
}

/**
 * @brief  Reads a buffer from the magneto sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the magneto internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval MAGNETO_OK in case of success, an error code otherwise
 */
static MAGNETO_StatusTypeDef MAGNETO_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead)
{
  MAGNETO_StatusTypeDef ret_val = MAGNETO_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_ReadData(pBuffer, DeviceAddr, RegisterAddr, NumByteToRead) != HAL_OK)
  {
    ret_val = MAGNETO_ERROR;
  }
  
  return ret_val;
}

/**
 * @brief  Configures pressure I2C interface
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef PRESSURE_IO_Init(void)
{
  if(I2C_EXPBD_Init() != HAL_OK)
  {
    return PRESSURE_ERROR;
  }
  
  return PRESSURE_OK;
}

/**
 * @brief  Writes a buffer to the pressure sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the pressure internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef PRESSURE_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite)
{
  PRESSURE_StatusTypeDef ret_val = PRESSURE_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_WriteData(pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite) != HAL_OK)
  {
    ret_val = PRESSURE_ERROR;
  }
  
  return ret_val;
}

/**
 * @brief  Reads a buffer from the pressure sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the pressure internal address register to be read
 * @param  NumByteToRead number of bytes to be read
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef PRESSURE_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead)
{
  PRESSURE_StatusTypeDef ret_val = PRESSURE_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_ReadData(pBuffer, DeviceAddr, RegisterAddr, NumByteToRead) != HAL_OK)
  {
    ret_val = PRESSURE_ERROR;
  }
  
  return ret_val;
}

/**
 * @brief  Configures humidity and temperature I2C interface
 * @retval HUM_TEMP_OK in case of success, an error code otherwise
 */
static HUM_TEMP_StatusTypeDef HUM_TEMP_IO_Init(void)
{
  if(I2C_EXPBD_Init() != HAL_OK)
  {
    return HUM_TEMP_ERROR;
  }
  
  return HUM_TEMP_OK;
}

/**
 * @brief  Writes a buffer to the humidity and temperature sensor
 * @param  pBuffer the pointer to data to be written
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the humidity and temperature internal address register to be written
 * @param  NumByteToWrite the number of bytes to be written
 * @retval HUM_TEMP_OK in case of success, an error code otherwise
 */
static HUM_TEMP_StatusTypeDef HUM_TEMP_IO_Write(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToWrite)
{
  HUM_TEMP_StatusTypeDef ret_val = HUM_TEMP_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_WriteData( pBuffer, DeviceAddr, RegisterAddr, NumByteToWrite ) != HAL_OK)
  {
    ret_val = HUM_TEMP_ERROR;
  }
  
  return ret_val;
}

/**
 * @brief  Reads a buffer from the humidity and temperature sensor
 * @param  pBuffer the pointer to data to be read
 * @param  DeviceAddr the slave address to be programmed
 * @param  RegisterAddr the humidity and temperature internal address register to be read
 * @param  NumByteToRead the number of bytes to be read
 * @retval HUM_TEMP_OK in case of success, an error code otherwise
 */
static HUM_TEMP_StatusTypeDef HUM_TEMP_IO_Read(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr,
    uint16_t NumByteToRead)
{
  HUM_TEMP_StatusTypeDef ret_val = HUM_TEMP_OK;
  
  /* call I2C_EXPBD Read data bus function */
  if(I2C_EXPBD_ReadData( pBuffer, DeviceAddr, RegisterAddr, NumByteToRead ) != HAL_OK)
  {
    ret_val = HUM_TEMP_ERROR;
  }
  
  return ret_val;
}

/******************************* I2C Routines**********************************/
/**
 * @brief  Configures I2C interface
 * @retval HAL status
 */
static HAL_StatusTypeDef I2C_EXPBD_Init(void)
{
  HAL_StatusTypeDef ret_val = HAL_OK;
  
  if(HAL_I2C_GetState(&I2C_EXPBD_Handle) == HAL_I2C_STATE_RESET)
  {
    /* I2C_EXPBD peripheral configuration */
#if ((defined (USE_STM32F4XX_NUCLEO)) || (defined (USE_STM32L1XX_NUCLEO)))
    I2C_EXPBD_Handle.Init.ClockSpeed = NUCLEO_I2C_EXPBD_SPEED;
    I2C_EXPBD_Handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
#endif
    
#if (defined (USE_STM32L0XX_NUCLEO))
    I2C_EXPBD_Handle.Init.Timing = NUCLEO_I2C_EXPBD_TIMING_400KHZ;                            /* 400KHz */
#endif
    I2C_EXPBD_Handle.Init.OwnAddress1 = 0x33;
    I2C_EXPBD_Handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    I2C_EXPBD_Handle.Instance = NUCLEO_I2C_EXPBD;
    
    /* Init the I2C */
    I2C_EXPBD_MspInit();
    ret_val = HAL_I2C_Init(&I2C_EXPBD_Handle);
  }
  
  return ret_val;
}

/**
 * @brief  Write a value in a register of the device through the bus
 * @param  pBuffer the pointer to data to be written
 * @param  Addr the device address on bus
 * @param  Reg the target register address to be written
 * @param  Size the size in bytes of the value to be written
 * @retval HAL status
 */

static HAL_StatusTypeDef I2C_EXPBD_WriteData(uint8_t* pBuffer, uint8_t Addr, uint8_t Reg, uint16_t Size)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(&I2C_EXPBD_Handle, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Size,
                             I2C_EXPBD_Timeout);
                             
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    I2C_EXPBD_Error(Addr);
  }
  
  return status;
}


/**
 * @brief  Read the value of a register of the device through the bus
 * @param  pBuffer the pointer to data to be read
 * @param  Addr the device address on bus
 * @param  Reg the target register address to be read
 * @param  Size the size in bytes of the value to be read
 * @retval HAL status.
 */
static HAL_StatusTypeDef I2C_EXPBD_ReadData(uint8_t* pBuffer, uint8_t Addr, uint8_t Reg, uint16_t Size)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Read(&I2C_EXPBD_Handle, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Size,
                            I2C_EXPBD_Timeout);
                            
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    I2C_EXPBD_Error(Addr);
  }
  
  return status;
}

/**
 * @brief  Manages error callback by re-initializing I2C
 * @param  Addr I2C Address
 * @retval None
 */
static void I2C_EXPBD_Error(uint8_t Addr)
{
  /* De-initialize the I2C comunication bus */
  HAL_I2C_DeInit(&I2C_EXPBD_Handle);
  
  /*FIXME: We need to wait a while in order to have I2C that works fine after deinit */
  HAL_Delay(1);
  
  /* Re-Initiaize the I2C comunication bus */
  I2C_EXPBD_Init();
}

/**
 * @brief  I2C MSP Initialization
 * @retval None
 */

static void I2C_EXPBD_MspInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* Enable I2C GPIO clocks */
  NUCLEO_I2C_EXPBD_SCL_SDA_GPIO_CLK_ENABLE();
  
  /* I2C_EXPBD SCL and SDA pins configuration -------------------------------------*/
  GPIO_InitStruct.Pin = NUCLEO_I2C_EXPBD_SCL_PIN | NUCLEO_I2C_EXPBD_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
#if ((defined (USE_STM32F4XX_NUCLEO)) || (defined (USE_STM32L0XX_NUCLEO)))
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
#endif
  
#if (defined (USE_STM32L1XX_NUCLEO))
  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
#endif
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Alternate  = NUCLEO_I2C_EXPBD_SCL_SDA_AF;
  HAL_GPIO_Init(NUCLEO_I2C_EXPBD_SCL_SDA_GPIO_PORT, &GPIO_InitStruct);
  
  /* Enable the I2C_EXPBD peripheral clock */
  NUCLEO_I2C_EXPBD_CLK_ENABLE();
  
  /* Force the I2C peripheral clock reset */
  NUCLEO_I2C_EXPBD_FORCE_RESET();
  
  /* Release the I2C peripheral clock reset */
  NUCLEO_I2C_EXPBD_RELEASE_RESET();
  
  /* Enable and set I2C_EXPBD Interrupt to the highest priority */
  HAL_NVIC_SetPriority(NUCLEO_I2C_EXPBD_EV_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(NUCLEO_I2C_EXPBD_EV_IRQn);
  
#if ((defined (USE_STM32F4XX_NUCLEO)) || (defined (USE_STM32L1XX_NUCLEO)))
  /* Enable and set I2C_EXPBD Interrupt to the highest priority */
  HAL_NVIC_SetPriority(NUCLEO_I2C_EXPBD_ER_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(NUCLEO_I2C_EXPBD_ER_IRQn);
#endif
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
