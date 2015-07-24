/**
 ******************************************************************************
 * @file    lps25hb.c
 * @author  MEMS Application Team
 * @version V1.0.0
 * @date    11-February-2015
 * @brief   This file provides a set of functions needed to manage the lps25hb.
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
#include "lps25hb.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup LPS25HB
 * @{
 */

static PRESSURE_StatusTypeDef LPS25HB_Init(PRESSURE_InitTypeDef *LPS25HB_Init);
static PRESSURE_StatusTypeDef LPS25HB_ReadID(uint8_t *p_id);
static PRESSURE_StatusTypeDef LPS25HB_RebootCmd(void);
static PRESSURE_StatusTypeDef LPS25HB_GetPressure(float* pfData);
static PRESSURE_StatusTypeDef LPS25HB_GetTemperature(float* pfData);
static PRESSURE_StatusTypeDef LPS25HB_PowerOff(void);
static void LPS25HB_SlaveAddrRemap(uint8_t SA0_Bit_Status);

/** @defgroup LPS25HB_Private_Variables LPS25HB_Private_Variables
 * @{
 */
PRESSURE_DrvTypeDef LPS25HBDrv =
{
  LPS25HB_Init,
  LPS25HB_PowerOff,
  LPS25HB_ReadID,
  LPS25HB_RebootCmd,
  0,
  0,
  0,
  0,
  0,
  LPS25HB_GetPressure,
  LPS25HB_GetTemperature,
  LPS25HB_SlaveAddrRemap,
  NULL
};

/* ------------------------------------------------------- */
/* Here you should declare the variable that implements    */
/* the internal struct of extended features of LPS25HB.    */
/* Then you must update the NULL pointer in the variable   */
/* of the extended features below.                         */
/* See the example of LSM6DS3 in lsm6ds3.c                 */
/* ------------------------------------------------------- */

PRESSURE_DrvExtTypeDef LPS25HBDrv_ext =
{
  PRESSURE_LPS25HB_DIL24_COMPONENT, /* unique ID for LPS25HB in the PRESSURE driver class */
  NULL /* pointer to internal struct of extended features of LPS25HB */
};

uint8_t LPS25HB_SlaveAddress = LPS25HB_ADDRESS_LOW;

/**
 * @}
 */

static PRESSURE_StatusTypeDef LPS25HB_PowerOn(void);
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawPressure(int32_t *raw_press);
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawTemperature(int16_t *raw_data);

/** @defgroup LPS25HB_Private_Functions LPS25HB_Private_Functions
 * @{
 */

/**
 * @brief  Set LPS25HB Initialization
 * @param  LPS25HB_Init the configuration setting for the LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_Init(PRESSURE_InitTypeDef *LPS25HB_Init)
{
  uint8_t tmp1 = 0x00;
  
  /* Configure the low level interface ---------------------------------------*/
  if(LPS25HB_IO_Init() != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  if(LPS25HB_PowerOn() != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  if(LPS25HB_IO_Read(&tmp1, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Output Data Rate selection */
  tmp1 &= ~(LPS25HB_ODR_MASK);
  tmp1 |= LPS25HB_Init->OutputDataRate;
  
  /* Interrupt circuit selection */
  tmp1 &= ~(LPS25HB_DIFF_EN_MASK);
  tmp1 |= LPS25HB_Init->DiffEnable;
  
  /* Block Data Update selection */
  tmp1 &= ~(LPS25HB_BDU_MASK);
  tmp1 |= LPS25HB_Init->BlockDataUpdate;
  
  /* Serial Interface Mode selection */
  tmp1 &= ~(LPS25HB_SPI_SIM_MASK);
  tmp1 |= LPS25HB_Init->SPIMode;
  
  if(LPS25HB_IO_Write(&tmp1, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  if(LPS25HB_IO_Read(&tmp1, LPS25HB_SlaveAddress, LPS25HB_RES_CONF_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Serial Interface Mode selection */
  tmp1 &= ~(LPS25HB_P_RES_MASK);
  tmp1 |= LPS25HB_Init->PressureResolution;
  
  /* Serial Interface Mode selection */
  tmp1 &= ~(LPS25HB_T_RES_MASK);
  tmp1 |= LPS25HB_Init->TemperatureResolution;
  
  if(LPS25HB_IO_Write(&tmp1, LPS25HB_SlaveAddress, LPS25HB_RES_CONF_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  LPS25HB_IO_ITConfig();
  
  return PRESSURE_OK;
}

/**
 * @brief  Read ID address of LPS25HB
 * @param  ht_id the pointer where the ID of the device is stored
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_ReadID(uint8_t *p_id)
{
  if(!p_id)
  {
    return PRESSURE_ERROR;
  }
  
  return LPS25HB_IO_Read(p_id, LPS25HB_SlaveAddress, LPS25HB_WHO_AM_I_ADDR, 1);
}

/**
 * @brief  Reboot memory content of LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_RebootCmd(void)
{
  uint8_t tmpreg;
  
  /* Read CTRL_REG5 register */
  if(LPS25HB_IO_Read(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Enable or Disable the reboot memory */
  tmpreg |= LPS25HB_RESET_MEMORY;
  
  /* Write value to MEMS CTRL_REG5 regsister */
  if(LPS25HB_IO_Write(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG2_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  return PRESSURE_OK;
}


/**
 * @brief  Read LPS25HB output register, and calculate the raw pressure
 * @param  raw_press the pressure raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawPressure(int32_t *raw_press)
{
  uint8_t buffer[3], i;
  uint32_t tempVal = 0;
  
  /* Read the register content */
  
  if(LPS25HB_IO_Read(buffer, LPS25HB_SlaveAddress, (LPS25HB_PRESS_POUT_XL_ADDR | LPS25HB_I2C_MULTIPLEBYTE_CMD),
                     3) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Build the raw data */
  for (i = 0 ; i < 3 ; i++)
    tempVal |= (((uint32_t) buffer[i]) << (8 * i));
    
  /* convert the 2's complement 24 bit to 2's complement 32 bit */
  if (tempVal & 0x00800000)
    tempVal |= 0xFF000000;
    
  /* return the built value */
  *raw_press = ((int32_t) tempVal);
  
  return PRESSURE_OK;
}

/**
 * @brief  Read LPS25HB output register, and calculate the pressure in mbar
 * @param  pfData the pressure value in mbar
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_GetPressure(float* pfData)
{
  int32_t raw_press = 0;
  
  if(LPS25HB_I2C_ReadRawPressure(&raw_press) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  *pfData = (float)raw_press / 4096.0f;
  
  return PRESSURE_OK;
}

/**
 * @brief  Read LPS25HB output register, and calculate the raw temperature
 * @param  raw_data the temperature raw value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_I2C_ReadRawTemperature(int16_t *raw_data)
{
  uint8_t buffer[2];
  uint16_t tempVal = 0;
  
  /* Read the register content */
  if(LPS25HB_IO_Read(buffer, LPS25HB_SlaveAddress, (LPS25HB_TEMP_OUT_L_ADDR | LPS25HB_I2C_MULTIPLEBYTE_CMD),
                     2) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Build the raw value */
  tempVal = (((uint16_t)buffer[1]) << 8) + (uint16_t)buffer[0];
  
  /* Return it */
  *raw_data = ((int16_t)tempVal);
  
  return PRESSURE_OK;
}

/**
 * @brief  Read LPS25HB output register, and calculate the temperature
 * @param  pfData the temperature value
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_GetTemperature(float *pfData)
{
  int16_t raw_data;
  
  if(LPS25HB_I2C_ReadRawTemperature(&raw_data) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  *pfData = (float)((((float)raw_data / 480.0f) + 42.5f));
  
  return PRESSURE_OK;
}
/**
 * @brief  Exit the shutdown mode for LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_PowerOn(void)
{
  uint8_t tmpreg;
  
  /* Read the register content */
  if(LPS25HB_IO_Read(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Set the power down bit */
  tmpreg |= LPS25HB_MODE_ACTIVE;
  
  /* Write register */
  if(LPS25HB_IO_Write(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  return PRESSURE_OK;
}


/**
 * @brief  Enter the shutdown mode for LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static PRESSURE_StatusTypeDef LPS25HB_PowerOff(void)
{
  uint8_t tmpreg;
  
  /* Read the register content */
  if(LPS25HB_IO_Read(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  /* Reset the power down bit */
  tmpreg &= ~(LPS25HB_MODE_ACTIVE);
  
  /* Write register */
  if(LPS25HB_IO_Write(&tmpreg, LPS25HB_SlaveAddress, LPS25HB_CTRL_REG1_ADDR, 1) != PRESSURE_OK)
  {
    return PRESSURE_ERROR;
  }
  
  return PRESSURE_OK;
}

/**
 * @brief  Set the slave address according to SA0 bit
 * @param  SA0_Bit_Status LPS25HB_SA0_LOW or LPS25HB_SA0_HIGH
 * @retval None
 */
static void LPS25HB_SlaveAddrRemap(uint8_t SA0_Bit_Status)
{
  LPS25HB_SlaveAddress = (SA0_Bit_Status == LPS25HB_SA0_LOW ? LPS25HB_ADDRESS_LOW : LPS25HB_ADDRESS_HIGH);
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
