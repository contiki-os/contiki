/**
 * \addtogroup mb851-platform
 *
 * @{
 */
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

#include PLATFORM_HEADER
#include "mems.h"
#include "timer.h"

/*--------------------------------------------------------------------------*/
#define TIMEOUT     20000
#define SUCCESS     1
#define FAIL        0

#define SEND_BYTE(data) do { SC2_DATA=(data); SC2_TWICTRL1 |= SC_TWISEND;      \
                        } while(0)

#define WAIT_CMD_FIN()  {                                                      \
                          struct timer t;                                      \
                          timer_set(&t, CLOCK_SECOND/100);                     \
                          while((SC2_TWISTAT&SC_TWICMDFIN)!=SC_TWICMDFIN){     \
                            if(timer_expired(&t)){                             \
                            return FAIL;                                       \
                           }                                                   \
                          }                                                    \
                        }

#define WAIT_TX_FIN()   {                                                      \
                          struct timer t;                                      \
                          timer_set(&t, CLOCK_SECOND/100);                     \
                          while((SC2_TWISTAT&SC_TWITXFIN)!=SC_TWITXFIN){       \
                            if(timer_expired(&t)){                             \
                            return FAIL;                                       \
                           }                                                   \
                          }                                                    \
                        }
#define WAIT_RX_FIN()    {                                                     \
                          struct timer t;                                      \
                          timer_set(&t, CLOCK_SECOND/100);                     \
                          while((SC2_TWISTAT&SC_TWIRXFIN)!=SC_TWIRXFIN){       \
                            if(timer_expired(&t)){                             \
                            return FAIL;                                       \
                           }                                                   \
                          }                                                    \
                        }

/*--------------------------------------------------------------------------*/
static boolean fullscale_state;

static uint8_t i2c_mems_init(void);

static uint8_t i2c_send_frame(uint8_t address, uint8_t *p_buf, uint8_t len);

uint8_t i2c_write_reg(uint8_t slave_addr, uint8_t reg_addr,
                      uint8_t reg_value);

 /* static uint8_t i2c_mems_read (mems_data_t *mems_data); */
/*--------------------------------------------------------------------------*/
/**
 * \brief      Init MEMS
 * \return     None
 */
uint8_t
mems_init(void)
{
  uint8_t ret = 0;

  /* GPIO assignments */
  /* PA1: SC2SDA (Serial Data) */
  /* PA2: SC2SCL (Serial Clock) */
  /* -----SC2 I2C Master GPIO configuration */
  TIM2_CCER &= 0xFFFFEEEE;
  SC2_MODE = SC2_MODE_I2C;
  GPIO_PACFGL &= 0xFFFFF00F;
  GPIO_PACFGL |= 0x00000DD0;

  SC2_RATELIN = 14;             /* generates standard 100kbps or 400kbps */
  SC2_RATEEXP = 1;              /* 3 yields 100kbps; 1 yields 400kbps */
  SC2_TWICTRL1 = 0;             /* start from a clean state */
  SC2_TWICTRL2 = 0;             /* start from a clean state */

  ret = i2c_mems_init();
  fullscale_state = MEMS_LOW_RANGE;

 /* Add later if really needed */
#ifdef ST_DBG
  if(!ret)
    I2C_DeInit(MEMS_I2C);
#endif

  return ret;
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Returns the 3 mems acceleration values related to x,y,z axes
 * \param mems_data   pointer to mems_data_t that will be filled with the values
 * \return     None
 */
#if 0
uint8_t mems_get_value(mems_data_t *mems_data) 
{ 
  uint8_t i; 
  i = i2c_mems_read(mems_data); 
  return i; 
} 
#endif
/*--------------------------------------------------------------------------*/
/**
 * \brief           Send I2C frame 
 * \param address   destination device address
 * \param p_buf     pointer to data buffer
 * \param len       length of data
 * \retval 0        frame has not been successfully sent
 * \retval 1        frame has been successfully sent
 */
static uint8_t
i2c_send_frame(uint8_t address, uint8_t *p_buf, uint8_t len)
{
  uint8_t i, data;

  SC2_TWICTRL1 |= SC_TWISTART;  /* send start */
  WAIT_CMD_FIN();
  SEND_BYTE(address);           /* send the address low byte */
  WAIT_TX_FIN();

  for(i = 0; i < len; i++) {
    halInternalResetWatchDog();
    data = *(p_buf + i);
    SEND_BYTE(data);
    WAIT_TX_FIN();
  }

  SC2_TWICTRL1 |= SC_TWISTOP;
  WAIT_CMD_FIN();

  return SUCCESS;
}
/*--------------------------------------------------------------------------*/
/**
 * \brief           Receive I2C frame 
 * \param address   slave device address
 * \param p_buf     pointer to data buffer
 * \param len       length of data
 * \retval 0        frame has not been successfully received
 * \retval 1        frame has been successfully received
 */
static uint8_t
i2c_receive_frame(uint8_t slave_addr, uint8_t reg_addr, uint8_t *p_buf,
                  uint8_t len)
{
  uint8_t i, addr = reg_addr;

  if(len > 1) {
    addr += REPETIR;
  }

  SC2_TWICTRL1 |= SC_TWISTART;  /* send start */
  WAIT_CMD_FIN();

  SEND_BYTE(slave_addr | 0x00); /* send the address low byte */
  WAIT_TX_FIN();

  SEND_BYTE(addr);
  WAIT_TX_FIN();

  SC2_TWICTRL1 |= SC_TWISTART;  /* send start */
  WAIT_CMD_FIN();

  SEND_BYTE(slave_addr | 0x01); /* send the address low byte */
  WAIT_TX_FIN();

  for(i = 0; i < len; i++) {
    halInternalResetWatchDog();
    if(i < (len - 1)) {
      SC2_TWICTRL2 |= SC_TWIACK;        /* ack on receipt of data */
    } else {
      SC2_TWICTRL2 &= ~SC_TWIACK;       /* don't ack if last one */
    }
    SC2_TWICTRL1 |= SC_TWIRECV; /* set to receive */
    WAIT_RX_FIN();
    *(p_buf + i) = SC2_DATA;    /* receive data */
  }

  SC2_TWICTRL1 |= SC_TWISTOP;   /* send STOP */
  WAIT_CMD_FIN();

  return SUCCESS;
}
/*--------------------------------------------------------------------------*/
/**
 * \brief               write a register on the I2C target
 * \param slave_addr    slave device address
 * \param reg_addr      address of the register to be written
 * \param reg_value     value of the register to be written
 * \retval 0            register has not been successfully written
 * \retval 1            register has been successfully written
 */
uint8_t
i2c_write_reg(uint8_t slave_addr, uint8_t reg_addr, uint8_t reg_value)
{
  uint8_t i2c_buffer[2];

  i2c_buffer[0] = reg_addr;
  i2c_buffer[1] = reg_value;
  return i2c_send_frame(slave_addr, i2c_buffer, 2);
}
/*--------------------------------------------------------------------------*/
/**
 * \brief               read a register from the I2C target
 * \param slave_addr    slave device address
 * \param reg_addr      address of the register
 * \param p_buf         storage destination for the read data
 * \retval 0            register has not been successfully read
 * \retval 1            register has been successfully read
 */
uint8_t
i2c_read_reg(uint8_t slave_addr, uint8_t reg_addr, uint8_t *p_buf,
             uint8_t len)
{
  return i2c_receive_frame(slave_addr, reg_addr, p_buf, len);
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Init MEMS
 * \return     None
 * \retval 0   the device has not been successfully initialized
 * \retval 1   the device has been successfully initialized
 */
static uint8_t
i2c_mems_init(void)
{
  uint8_t i = 0;

  i += i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, STATUS_REG, 0x00);  /* no flag */
  i += i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, FF_WU_CFG, 0x00);   /* all off */
  i += i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, DD_CFG, 0x00);      /* all off */
  /* i += i2c_write_reg (KLIS3L02DQ_SLAVE_ADDR, CTRL_REG2, (1<<4) | (1<<1) | (1 << 0)); */

  i += i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, CTRL_REG2, 0x00);
  /* i += i2c_write_reg (KLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0xC7); */
  i += i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0x87);

  if(i != 5) {
    return 0;
  }
  return 1;
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      turn on the MEMS device
 * \retval 0   the device has not been successfully set to normal mode
 * \retval 1   the device has been successfully set to normal mode
 */
uint8_t
mems_on(void)
{
  return i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0xC7);
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      turn off the MEMS device
 * \retval 0   the device has not been successfully turned off
 * \retval 1   the device has been successfully turned off
 */
uint8_t
mems_off(void)
{
  return i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, 0x87);
}
/*--------------------------------------------------------------------------*/
/**
 * \brief         set full-scale range of the device
 * \param range   HIGH for high scale selection, LOW for low range.
 * \retval 0      the device has not been successfully set to full scale mode
 * \retval 1      the device has been successfully set to full scale mode
 */
uint8_t
mems_set_fullscale(boolean range)
{
  uint8_t i2c_buffer;
  if (!i2c_read_reg(KLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, &i2c_buffer, 1)) {
    return 0;
  }

  if(range == MEMS_HIGH_RANGE) {
    i2c_buffer |= 0x20;
  } else {
    i2c_buffer &= ~0x20;
  }

  if (!i2c_write_reg(KLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, i2c_buffer)) {
    return 0;
  }
  fullscale_state = range;
  return 1;
}
/*--------------------------------------------------------------------------*/
/**
 * \brief         Get full-scale range of the device
 * \retval HIGH   high scale selection
 * \retval LOW    low range
 */
boolean
mems_get_fullscale(void)
{
  return fullscale_state;
}
/*--------------------------------------------------------------------------*/
/**
 * \brief      Returns the 3 mems acceleration values related to x,y,z axes
 * \param mems_data   pointer to mems_data_t that will be filled with the values
 * \retval 0      acceleration data has not been successfully read
 * \retval 1      acceleration data has been successfully read
 */
#if 0
static uint8_t
i2c_mems_read(mems_data_t *mems_data) 
{ 
  uint8_t i, i2c_buffer[8]; 

  i = i2c_read_reg (KLIS3L02DQ_SLAVE_ADDR, OUTX_L, i2c_buffer, 8); 
  mems_data->outx_h = i2c_buffer[0]; 
  mems_data->outx_l = i2c_buffer[1]; 
  mems_data->outy_h = i2c_buffer[2]; 
  mems_data->outy_l = i2c_buffer[3]; 
  mems_data->outz_h = i2c_buffer[4]; 
  mems_data->outz_l = i2c_buffer[5]; 

  return i; 
}
#endif
/*--------------------------------------------------------------------------*/
/** @} */
