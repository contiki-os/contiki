/**
 * \addtogroup mbxxx-platform
 *
 * @{
 */
/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : hal_led.c
* Author             : MCD Application Team
* Version            : V1.0
* Date               : September 2009
* Description        : Driver for leds management on STM32W108 MBXXX boards
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include PLATFORM_HEADER
#include "mems.h"
#include "timer.h"
#include "i2c.h"

/* Private define -- ---------------------------------------------------------*/
#define SUCCESS 1
#define FAIL    0

/* Private variables ---------------------------------------------------------*/
static boolean fullscale_state;

/* Functions -----------------------------------------------------------------*/

/*******************************************************************************
* Function Name  : MEMS_Write_Reg
* Description    : It writes a register on the I2C target
* Input          : slave addr is the I2C target device
*                  reg_addr is the address of the register to be written
*                  reg_value is the value of the register to be written
* Output         : None
* Return         : 1 if the register has been successfully written, 0 otherwise.
*******************************************************************************/
uint8_t
MEMS_Write_Reg (uint8_t slave_addr, uint8_t reg_addr, uint8_t reg_value)
{
  i2c_start();

  /* send the address low byte */
  i2c_write(slave_addr);

  /* send register address */
  i2c_write(reg_addr);

  /* send register value */
  i2c_write(reg_value);

  i2c_stop();

  return SUCCESS;
}/* end MEMS_Write_Reg() */

/*******************************************************************************
* Function Name  : MEMS_Read_Reg
* Description    : It reads  a register on the I2C target
* Input          : slave addr is the I2C target device
*                  reg_addr is the address of the register to be read
*                  pBuffer is the storage destination for the read data
*                  NoOfBytes is the amount of data to read
* Output         : I2C frame
* Return         : 1 if the register has been successfully read, 0 otherwise.
*******************************************************************************/
uint8_t
MEMS_Read_Reg (uint8_t slave_addr, uint8_t reg_addr, uint8_t *pBuffer,
		      uint8_t NoOfBytes)
{
  uint8_t i, ack, addr = reg_addr;

  if (NoOfBytes > 1)
    addr += REPETIR;

  i2c_start();

  /* send the address low byte */
  i2c_write(slave_addr | 0x00);

  i2c_write(addr);

  i2c_start();

  /* send the address low byte */
  i2c_write(slave_addr | 0x01);

  /* loop receiving the data */
  for (i = 0; i < NoOfBytes; i++){

    if (i < (NoOfBytes - 1))
      /* ack on receipt of data */
      ack = 1;
    else
      /* don't ack if last one */
      ack = 0;

    /* receive data */
    *(pBuffer+i) = i2c_read(ack);
  }

  i2c_stop();

  return SUCCESS;
}/* end MEMS_Read_Reg() */

/*******************************************************************************
* Function Name  : MEMS_Init
* Description    : It inits mems 
* Input          : None
* Output         : status
* Return         : None
*******************************************************************************/
uint8_t
MEMS_Init(void)
{
  TIM2_CCER &= 0xFFFFEEEE;
  MEMS_Write_Reg (kLIS3L02DQ_SLAVE_ADDR, STATUS_REG, 0x00);    //no flag
  MEMS_Write_Reg (kLIS3L02DQ_SLAVE_ADDR, FF_WU_CFG, 0x00);     // all off
  MEMS_Write_Reg (kLIS3L02DQ_SLAVE_ADDR, DD_CFG, 0x00);        // all off
  MEMS_Write_Reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG2, 0x00);
  MEMS_Write_Reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0x87);  
  fullscale_state = MEMS_LOW_RANGE;

  return 1;
}/* end MEMS_Init */

/*******************************************************************************
* Function Name  : I2C_MEMS_On
* Description    : It turn on the device. 
* Input          : None
* Output         : None
* Return         : 1 if the device has been successfully set to normal mode, 0 otherwise.
*******************************************************************************/
uint8_t
MEMS_On (void)
{
  return MEMS_Write_Reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0xC7);
}

/*******************************************************************************
* Function Name  : MEMS_Off
* Description    : It turn off the device. 
* Input          : None
* Output         : None
* Return         : 1 if the device has been successfully set to power-down mode, 0 otherwise.
*******************************************************************************/
uint8_t
MEMS_Off (void)
{
  return MEMS_Write_Reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0x87);
}

/*******************************************************************************
* Function Name  : I2C_MEMS_SetFullScale
* Description    : It sets the full-scale range of the device.
* Input          : range  HIGH for high scale selection, LOW for low range.
* Output         : None
* Return         : 1 if the device has been successfully set to full scale mode, 0 otherwise.
*******************************************************************************/
uint8_t
MEMS_SetFullScale (boolean range)
{
  uint8_t i2c_buffer;

  MEMS_Read_Reg(kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, &i2c_buffer, 1);

  if(range==MEMS_HIGH_RANGE){
    i2c_buffer |= 0x20;
  }
  else {
    i2c_buffer &= ~0x20;
  }

  MEMS_Write_Reg(kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, i2c_buffer);

  fullscale_state = range;

  return 1;
}

/*******************************************************************************
* Function Name  : MEMS_GetFullScale
* Description    : It get the full-scale range of the device. 
* Input          : None
* Output         : None
* Return         : range  HIGH for high scale selection, LOW for low range.
*******************************************************************************/
boolean
MEMS_GetFullScale (void)
{
  return fullscale_state;  
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
/** @} */

