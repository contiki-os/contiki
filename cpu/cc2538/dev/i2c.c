/**
 * \addtogroup cc2538-i2c cc2538 I2C Control
 * @{
 *
 * \file
 * /cpu/cc2538/dev/i2c.c
 * Implementation file of the I2C Control module
 *
 * \author
 * Mehdi Migault
 */

#include "i2c.h"

/*---------------------------------------------------------------------------*/
void
i2c_init(uint8_t port_sda, uint8_t pin_sda, uint8_t port_scl, uint8_t pin_scl, uint32_t bus_speed)
{
  volatile uint32_t ui32Delay;

  /* Enable I2C clock in different modes */
  REG(SYS_CTRL_RCGCI2C) |= 1; /* Run mode */
  /* REG(SYS_CTRL_SCGCI2C) |= 1;	//Sleep mode */
  /* REG(SYS_CTRL_DCGCI2C) |= 1;	//PM0 mode */

  /*Reset I2C peripheral*/
  REG(SYS_CTRL_SRI2C) |= 1; /* Reset position */
  for(ui32Delay = 0; ui32Delay < 16; ui32Delay++) { /* Delay for a little bit. */
  }
  REG(SYS_CTRL_SRI2C) &= ~1;  /* Normal position */

  /* Set pins in input */
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(port_sda), GPIO_PIN_MASK(pin_sda));
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(port_scl), GPIO_PIN_MASK(pin_scl));

  /* Set peripheral control for the pins */
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(port_sda), GPIO_PIN_MASK(pin_sda));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(port_scl), GPIO_PIN_MASK(pin_scl));

  /* Set the pad to no drive type */
  ioc_set_over(port_sda, pin_sda, IOC_OVERRIDE_DIS);
  ioc_set_over(port_scl, pin_scl, IOC_OVERRIDE_DIS);

  /* Set pins as peripheral inputs */
  REG(IOC_I2CMSSDA) = ioc_input_sel(port_sda, pin_sda);
  REG(IOC_I2CMSSCL) = ioc_input_sel(port_scl, pin_scl);

  /* Set pins as peripheral outputs */
  ioc_set_sel(port_sda, pin_sda, IOC_PXX_SEL_I2C_CMSSDA);
  ioc_set_sel(port_scl, pin_scl, IOC_PXX_SEL_I2C_CMSSCL);

  /* Enable the I2C master module */
  i2c_master_enable();

  /* t the master clock frequency */
  i2c_set_frequency(bus_speed);
}
/*---------------------------------------------------------------------------*/
void
i2c_master_enable(void)
{
  REG(I2CM_CR) |= 0x10; /* Set MFE bit */
}
/*---------------------------------------------------------------------------*/
void
i2c_master_disable(void)
{
  REG(I2CM_CR) &= ~0x10;  /* Reset MFE bit */
}
/*---------------------------------------------------------------------------*/
static uint32_t
get_sys_clock(void)
{
  return SYS_CTRL_32MHZ / ((REG(SYS_CTRL_CLOCK_STA) & SYS_CTRL_CLOCK_STA_SYS_DIV) + 1); /* Get the clock status diviser */
}
/*---------------------------------------------------------------------------*/
void
i2c_set_frequency(uint32_t freq)
{
  /* Peripheral clock setting, using the system clock */
  REG(I2CM_TPR) = ((get_sys_clock() + (2 * 10 * freq) - 1) / (2 * 10 * freq)) - 1;
}
/*---------------------------------------------------------------------------*/
void
i2c_master_set_slave_address(uint8_t slave_addr, uint8_t access_mode)
{
  if(access_mode) {
    REG(I2CM_SA) = ((slave_addr << 1) | 1);
  } else {
    REG(I2CM_SA) = (slave_addr << 1);
  }
}
/*---------------------------------------------------------------------------*/
void
i2c_master_data_put(uint8_t data)
{
  REG(I2CM_DR) = data;
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_master_data_get(void)
{
  return REG(I2CM_DR);
}
/*---------------------------------------------------------------------------*/
void
i2c_master_command(uint8_t cmd)
{
  REG(I2CM_CTRL) = cmd;
  /* Here we need a delay, otherwise the I2C module keep the receiver mode */
  clock_delay_usec(1);
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_master_busy(void)
{
  return REG(I2CM_STAT) & I2CM_STAT_BUSY;
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_master_error(void)
{
  uint8_t temp = REG(I2CM_STAT);  /* Get all status */
  if(temp & I2CM_STAT_BUSY) {   /* No valid if BUSY bit is set */
    return I2C_MASTER_ERR_NONE;
  } else if(temp & (I2CM_STAT_ERROR | I2CM_STAT_ARBLST)) {
    return temp;  /* Compare later */
  }
  return I2C_MASTER_ERR_NONE;
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_single_send(uint8_t slave_addr, uint8_t data)
{
  i2c_master_set_slave_address(slave_addr, I2C_SEND);
  i2c_master_data_put(data);
  i2c_master_command(I2C_MASTER_CMD_SINGLE_SEND);
  while(i2c_master_busy()) {
  }
  /* Return the STAT register of I2C module if error occured, I2C_MASTER_ERR_NONE otherwise */
  return i2c_master_error();
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_single_receive(uint8_t slave_addr, uint8_t *data)
{
  uint32_t temp;

  i2c_master_set_slave_address(slave_addr, I2C_RECEIVE);
  i2c_master_command(I2C_MASTER_CMD_SINGLE_RECEIVE);
  while(i2c_master_busy()) {
  }
  if((temp = i2c_master_error())) {
    return temp;
  } else {
    *data = i2c_master_data_get();
    return I2C_MASTER_ERR_NONE;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
