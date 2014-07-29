/** @file cpu/stm32w108/hal/micro/cortexm3/mems.c
 * @brief MB851 MEMS drivers
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#include PLATFORM_HEADER
#include "hal/hal.h"
#include "hal/error.h"
#include "hal/micro/mems.h"

#define TIMEOUT 20000

#define SUCCESS 1

#define SEND_BYTE(data) do{ SC2_DATA=(data); SC2_TWICTRL1 |= SC_TWISEND; }while(0)

#define WAIT_CMD_FIN()  do{}while((SC2_TWISTAT&SC_TWICMDFIN)!=SC_TWICMDFIN)
#define WAIT_TX_FIN()   do{}while((SC2_TWISTAT&SC_TWITXFIN)!=SC_TWITXFIN)
#define WAIT_RX_FIN()   do{}while((SC2_TWISTAT&SC_TWIRXFIN)!=SC_TWIRXFIN)

static uint8_t i2c_MEMS_Init (void);
static uint8_t i2c_MEMS_Read (t_mems_data *mems_data);
//extern void halInternalResetWatchDog(void);
static uint8_t i2c_Send_Frame (uint8_t DeviceAddress, uint8_t *pBuffer, uint8_t NoOfBytes);
static uint8_t i2c_Send_Frame (uint8_t DeviceAddress, uint8_t *pBuffer, uint8_t NoOfBytes);
uint8_t i2c_write_reg (uint8_t slave_addr, uint8_t reg_addr, uint8_t reg_value);
static uint8_t i2c_MEMS_Init (void);
static uint8_t i2c_MEMS_Read (t_mems_data *mems_data);

/* Functions -----------------------------------------------------------------*/
uint8_t mems_Init(void)
{
  uint8_t ret = 0;

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

  ret = i2c_MEMS_Init();

//Add later if really needed
#ifdef ST_DBG
  if (!ret)
   i2c_DeInit(MEMS_I2C);
#endif

  return ret;
}/* end mems_Init */

uint8_t mems_GetValue(t_mems_data *mems_data)
{
  uint8_t i;
  i = i2c_MEMS_Read(mems_data);
  return i;
}/* end mems_GetValue() */


/* Private Functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : i2c_Send_Frame
* Description    : It sends I2C frame
* Input          : DeviceAddress is the destination device address
*                  pBUffer is the buffer data
*                  NoOfBytes is the number of bytes
* Output         : None
* Return         : status
*******************************************************************************/
static uint8_t i2c_Send_Frame (uint8_t DeviceAddress, uint8_t *pBuffer, uint8_t NoOfBytes)
{
  uint8_t i, data;

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
}/* end i2c_Send_Frame() */

/*******************************************************************************
* Function Name  : i2c_Receive_Frame
* Description    : It receives an I2C frame and stores it in pBUffer parameter
* Input          : slave_addr is the slave address
*                  reg_addr is the register address
*                  NoOfBytes is the numenr of bytes to read starting from reg_addr
* Output         : buffer
* Return         : status
*******************************************************************************/
static uint8_t i2c_Receive_Frame (uint8_t slave_addr, uint8_t reg_addr, uint8_t *pBuffer, uint8_t NoOfBytes)
{
  uint8_t i, addr = reg_addr;

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
}/* end i2c_Receive_Frame() */


/*******************************************************************************
* Function Name  : i2c_write_reg
* Description    : It writes a register on the I2C target
* Input          : slave addr is the I2C target device
*                  reg_addr is the address of the register to be written
*                  reg_value is the value of the register to be written
*                  NoOfBytes is the numenr of bytes to read starting from reg_addr
* Output         : None
* Return         : I2C frame
*******************************************************************************/
uint8_t i2c_write_reg (uint8_t slave_addr, uint8_t reg_addr, uint8_t reg_value)
{
  uint8_t i2c_buffer[2];

  i2c_buffer[0] = reg_addr;
  i2c_buffer[1] = reg_value;

  return i2c_Send_Frame (slave_addr, i2c_buffer, 2);
}/* end i2c_write_reg() */

/*******************************************************************************
* Function Name  : i2c_read_reg
* Description    : It reads  a register on the I2C target
* Input          : slave addr is the I2C target device
*                  reg_addr is the address of the register to be read
*                  pBuffer is the storage destination for the read data
*                  NoOfBytes is the amount of data to read
* Output         : None
* Return         : I2C frame
*******************************************************************************/
uint8_t i2c_read_reg (uint8_t slave_addr, uint8_t reg_addr, uint8_t *pBuffer, uint8_t NoOfBytes)
{
  return i2c_Receive_Frame (slave_addr, reg_addr, pBuffer, NoOfBytes);
}/* end i2c_read_reg() */

/*******************************************************************************
* Function Name  : i2c_MEMS_Init
* Description    : It performs basic MEMS register writes for initialization
*                  purposes
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
static uint8_t i2c_MEMS_Init (void)
{
  uint8_t i = 0;

  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, STATUS_REG, 0x00);    //no flag
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, FF_WU_CFG, 0x00);     // all off
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, DD_CFG, 0x00);        // all off
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG2, (0<<4) | (0<<1) | (0 << 0));
  i += i2c_write_reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0x47);

  if (i != 5)
    return 0;

  return 1;
}/* end i2c_MEMS_Init() */

/*******************************************************************************
* Function Name  : i2c_MEMS_Read
* Description    : It reads 3 axes acceleration data from mems
* Input          : None
* Output         : mems_data
* Return         : I2C frame
*******************************************************************************/
static uint8_t i2c_MEMS_Read (t_mems_data *mems_data)
{
  uint8_t i, i2c_buffer[8];

  /* Wait for new set of data to be available */
  while (1) {
    i = i2c_read_reg (kLIS3L02DQ_SLAVE_ADDR, STATUS_REG, i2c_buffer, 1);
    if (i2c_buffer[0] & (1 << 3))
      break;
  }
  i = i2c_read_reg (kLIS3L02DQ_SLAVE_ADDR, OUTX_L, i2c_buffer, 8);

  mems_data->outx_h = i2c_buffer[0];
  mems_data->outx_l = i2c_buffer[1];
  mems_data->outy_h = i2c_buffer[2];
  mems_data->outy_l = i2c_buffer[3];
  mems_data->outz_h = i2c_buffer[4];
  mems_data->outz_l = i2c_buffer[5];

  return i;
}/* end i2c_MEMS_Read() */
