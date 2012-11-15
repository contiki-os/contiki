/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : hal_led.c
* Author             : MCD Application Team
* Version            : V1.0
* Date               : September 2009
* Description        : Driver for leds management on STM32W108 MB851 board
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

/* Private define -- ---------------------------------------------------------*/

#define TIMEOUT 20000

#define SUCCESS 1
#define FAIL    0  

#define SEND_BYTE(data) do{ SC2_DATA=(data); SC2_TWICTRL1 |= SC_TWISEND; }while(0)

#define WAIT_CMD_FIN()  {                                                       \
                          struct timer t;                                       \
                          timer_set(&t, CLOCK_SECOND/100);                      \
                          while((SC2_TWISTAT&SC_TWICMDFIN)!=SC_TWICMDFIN){      \
                            if(timer_expired(&t)){                              \
                            return FAIL;                                        \
                           }                                                    \
                          }                                                     \
                        }

#define WAIT_TX_FIN()   {                                                       \
                          struct timer t;                                       \
                          timer_set(&t, CLOCK_SECOND/100);                      \
                          while((SC2_TWISTAT&SC_TWITXFIN)!=SC_TWITXFIN){        \
                            if(timer_expired(&t)){                              \
                            return FAIL;                                        \
                           }                                                    \
                          }                                                     \
                        }
#define WAIT_RX_FIN()    {                                                      \
                          struct timer t;                                       \
                          timer_set(&t, CLOCK_SECOND/100);                      \
                          while((SC2_TWISTAT&SC_TWIRXFIN)!=SC_TWIRXFIN){        \
                            if(timer_expired(&t)){                              \
                            return FAIL;                                        \
                           }                                                    \
                          }                                                     \
                        }

/* Private variables ---------------------------------------------------------*/
static boolean fullscale_state;

/* Private functions ---------------------------------------------------------*/
static int8u I2C_MEMS_Init (void);
//extern void halInternalResetWatchDog(void);
static int8u I2C_Send_Frame (int8u DeviceAddress, int8u *pBuffer, int8u NoOfBytes);
int8u i2c_write_reg (int8u slave_addr, int8u reg_addr, int8u reg_value);
//static int8u I2C_MEMS_Read (t_mems_data *mems_data);

/* Functions -----------------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Mems_Init
* Description    : It inits mems 
* Input          : None
* Output         : status
* Return         : None
*******************************************************************************/
int8u Mems_Init(void)
{  
  int8u ret = 0;
  
  // GPIO assignments
  // PA1: SC2SDA (Serial Data)
  // PA2: SC2SCL (Serial Clock)

  //-----SC2 I2C Master GPIO configuration

  TIM2_CCER &= 0xFFFFEEEE;
  SC2_MODE =  SC2_MODE_I2C;
  GPIO_PACFGL &= 0xFFFFF00F;
  GPIO_PACFGL |= 0x00000DD0;
  
  SC2_RATELIN =  14;   // generates standard 100kbps or 400kbps
  SC2_RATEEXP =  1;    // 3 yields 100kbps; 1 yields 400kbps
  SC2_TWICTRL1 =  0;   // start from a clean state
  SC2_TWICTRL2 =  0;   // start from a clean state  
  
  ret = I2C_MEMS_Init();
  
  fullscale_state = MEMS_LOW_RANGE;

//Add later if really needed  
#ifdef ST_DBG  
  if (!ret)
   I2C_DeInit(MEMS_I2C);
#endif
  
  return ret;
}/* end Mems_Init */

/*******************************************************************************
* Function Name  : Mems_GetValue
* Description    : It returns the 3 mems acceleration values related to x,y,z 
*                  axes in mems_data
* Input          : mems_data
* Output         : status
* Return         : None
*******************************************************************************/
//int8u Mems_GetValue(t_mems_data *mems_data)
//{
//  int8u i; 
//  i = I2C_MEMS_Read(mems_data);   
//  return i;
//}


/* Private Functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : I2C_Send_Frame
* Description    : It sends I2C frame 
* Input          : DeviceAddress is the destination device address
*                  pBUffer is the buffer data
*                  NoOfBytes is the number of bytes
* Output         : None
* Return         : 1 if the frame has been successfully sent, 0 otherwise.
*******************************************************************************/
static int8u I2C_Send_Frame (int8u DeviceAddress, int8u *pBuffer, int8u NoOfBytes)
{
  int8u i, data;

  SC2_TWICTRL1 |= SC_TWISTART;   // send start
  WAIT_CMD_FIN();
  
  SEND_BYTE(DeviceAddress);   // send the address low byte
  WAIT_TX_FIN();
  
   // loop sending the data
  for (i=0; i<NoOfBytes; i++) {
    halInternalResetWatchDog();
    
    data = *(pBuffer+i);
        
    SEND_BYTE(data);
    
    WAIT_TX_FIN();
  }

  SC2_TWICTRL1 |= SC_TWISTOP;
  WAIT_CMD_FIN();
  
  return SUCCESS;
}/* end I2C_Send_Frame() */

/*******************************************************************************
* Function Name  : I2C_Receive_Frame
* Description    : It receives an I2C frame and stores it in pBUffer parameter
* Input          : slave_addr is the slave address
*                  reg_addr is the register address
*                  NoOfBytes is the numenr of bytes to read starting from reg_addr
* Output         : I2C frame in pBUffer
* Return         : 1 if the frame has been successfully received, 0 otherwise.
*******************************************************************************/
static int8u I2C_Receive_Frame (int8u slave_addr, int8u reg_addr, int8u *pBuffer, int8u NoOfBytes)
{
  int8u i, addr = reg_addr;
  
  if (NoOfBytes > 1)
    addr += REPETIR;
  
  SC2_TWICTRL1 |= SC_TWISTART;   // send start
  WAIT_CMD_FIN();
   
  SEND_BYTE(slave_addr | 0x00);      // send the address low byte
  WAIT_TX_FIN();
  
  SEND_BYTE(addr);
  WAIT_TX_FIN();

  SC2_TWICTRL1 |= SC_TWISTART;     // send start
  WAIT_CMD_FIN();
  
  SEND_BYTE(slave_addr | 0x01);      // send the address low byte
  WAIT_TX_FIN();
 
  // loop receiving the data
  for (i=0;i<NoOfBytes;i++){
    halInternalResetWatchDog();

    if (i < (NoOfBytes - 1))
      SC2_TWICTRL2 |= SC_TWIACK;   // ack on receipt of data
    else
      SC2_TWICTRL2 &= ~SC_TWIACK;  // don't ack if last one

    SC2_TWICTRL1 |= SC_TWIRECV;    // set to receive
    WAIT_RX_FIN();
    *(pBuffer+i) = SC2_DATA;       // receive data
  }

  SC2_TWICTRL1 |= SC_TWISTOP;      // send STOP
  WAIT_CMD_FIN();  

  return SUCCESS;
}/* end I2C_Receive_Frame() */


/*******************************************************************************
* Function Name  : i2c_write_reg
* Description    : It writes a register on the I2C target
* Input          : slave addr is the I2C target device
*                  reg_addr is the address of the register to be written
*                  reg_value is the value of the register to be written
* Output         : None
* Return         : 1 if the register has been successfully written, 0 otherwise.
*******************************************************************************/
int8u i2c_write_reg (int8u slave_addr, int8u reg_addr, int8u reg_value)
{
  int8u i2c_buffer[2];
  
  i2c_buffer[0] = reg_addr;
  i2c_buffer[1] = reg_value;

  return I2C_Send_Frame (slave_addr, i2c_buffer, 2);
}/* end i2c_write_reg() */

/*******************************************************************************
* Function Name  : i2c_read_reg
* Description    : It reads  a register on the I2C target
* Input          : slave addr is the I2C target device
*                  reg_addr is the address of the register to be read
*                  pBuffer is the storage destination for the read data
*                  NoOfBytes is the amount of data to read
* Output         : I2C frame
* Return         : 1 if the register has been successfully read, 0 otherwise.
*******************************************************************************/
int8u i2c_read_reg (int8u slave_addr, int8u reg_addr, int8u *pBuffer, int8u NoOfBytes)
{
  return I2C_Receive_Frame (slave_addr, reg_addr, pBuffer, NoOfBytes);
}/* end i2c_read_reg() */

/*******************************************************************************
* Function Name  : I2C_MEMS_Init
* Description    : It performs basic MEMS register writes for initialization 
*                  purposes
* Input          : None
* Output         : None
* Return         : 1 if the device has been successfully initialized, 0 otherwise.
*******************************************************************************/
static int8u I2C_MEMS_Init (void)
{
  int8u i = 0;

  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, STATUS_REG, 0x00);    //no flag
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, FF_WU_CFG, 0x00);     // all off
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, DD_CFG, 0x00);        // all off
  //i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG2, (1<<4) | (1<<1) | (1 << 0));
  
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG2, 0x00);
  //i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0xC7);
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0x87);  

  if (i != 5)
    return 0;

  return 1;
}/* end I2C_MEMS_Init() */

/*******************************************************************************
* Function Name  : I2C_MEMS_On
* Description    : It turn on the device. 
* Input          : None
* Output         : None
* Return         : 1 if the device has been successfully set to normal mode, 0 otherwise.
*******************************************************************************/
int8u MEMS_On (void)
{
  return i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0xC7);  
}

/*******************************************************************************
* Function Name  : I2C_MEMS_Off
* Description    : It turn off the device. 
* Input          : None
* Output         : None
* Return         : 1 if the device has been successfully set to power-down mode, 0 otherwise.
*******************************************************************************/
int8u MEMS_Off (void)
{
  return i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0x87);  
}

/*******************************************************************************
* Function Name  : I2C_MEMS_SetFullScale
* Description    : It sets the full-scale range of the device.
* Input          : range  HIGH for high scale selection, LOW for low range.
* Output         : None
* Return         : 1 if the device has been successfully set to full scale mode, 0 otherwise.
*******************************************************************************/
int8u MEMS_SetFullScale (boolean range)
{
  int8u i2c_buffer;
  
  if(!i2c_read_reg(kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, &i2c_buffer, 1))
    return 0;
  
  if(range==MEMS_HIGH_RANGE){
    i2c_buffer |= 0x20;
  }
  else {
    i2c_buffer &= ~0x20;
  }

  if(!i2c_write_reg(kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, i2c_buffer))
    return 0;
  
  fullscale_state = range;
  
  return 1;
  
}

/*******************************************************************************
* Function Name  : I2C_MEMS_GetFullScale
* Description    : It get the full-scale range of the device. 
* Input          : None
* Output         : None
* Return         : range  HIGH for high scale selection, LOW for low range.
*******************************************************************************/
boolean MEMS_GetFullScale (void)
{  
  return fullscale_state;  
}

/*******************************************************************************
* Function Name  : I2C_MEMS_Read
* Description    : It reads 3 axes acceleration data from mems
* Input          : None
* Output         : mems_data
* Return         : 1 if acceleration data has been successfully read, 0 otherwise
*******************************************************************************/
//static int8u I2C_MEMS_Read (t_mems_data *mems_data)
//{
//  int8u i, i2c_buffer[8];
//
//  i = i2c_read_reg (kLIS3L02DQ_SLAVE_ADDR, OUTX_L, i2c_buffer, 8);  
//
//  mems_data->outx_h = i2c_buffer[0];
//  mems_data->outx_l = i2c_buffer[1];
//  mems_data->outy_h = i2c_buffer[2];
//  mems_data->outy_l = i2c_buffer[3];
//  mems_data->outz_h = i2c_buffer[4];
//  mems_data->outz_l = i2c_buffer[5];
//
//  return i;
//}/* end I2C_MEMS_Read() */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
