/*
 * Copyright (c) 2013, Loughborough University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538dk-accelerometer
 * @{
 *
 * \file
 *         TDriver for the SmartRF06EB accelerometer
 *
 *
 * \author
 *         Vasilis Michopoulos <basilismicho@gmail.com>
 */

#include "contiki.h"
#include "dev/accel-sensor.h"
#include "sys/clock.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "spi-arch.h"
#include "dev/spi.h"

#include <stdint.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief read bytes function for the accelerometer.
 *
 * reads multiple bytes.
 *
 * TBD
 */
/*---------------------------------------------------------------------------*/
/**
 * \brief read byte function for the accelerometer.
 *
 * reads 1 bytes.
 */
void
read_reg(uint8_t reg_addr, uint8_t * value)
{
  uint16_t tx_buf, rx_buf;
  /*use type as ur read address and add the read mask*/
  reg_addr |= ACC_READ_M;

  tx_buf = (uint16_t)(reg_addr << 8);

  /*wait for end of transmissions if any
   * and empty RX FIFO*/
  SPI_WAITFOREOTx();
  SPI_FLUSH();

#if SOFT_CS
  GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ACC_PORT), GPIO_PIN_MASK(ACC_CS_PIN));
#endif
  /*initiate communication with accelerometer*/
	SPI_WRITE(tx_buf);
	rx_buf = SPI_RXBUF;
#if SOFT_CS
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(ACC_PORT), GPIO_PIN_MASK(ACC_CS_PIN));
#endif

  *value = (uint8_t) (rx_buf & 0xff);

}
/*---------------------------------------------------------------------------*/
/**
 * \brief read value function for the accelerometer.
 *
 * returns 1 reading
 */
static int
value(int type)
{
  int ret = 0;
  uint8_t tmp_buffer[2] = {0};
  uint8_t reg_addr;
  uint8_t tmp=0;

  switch(type) {
  case ACC_X_AXIS:
    reg_addr = ACC_X_LSB;
    break;
  case ACC_Y_AXIS:
    reg_addr = ACC_Y_LSB;
    break;
  case ACC_Z_AXIS:
    reg_addr = ACC_Z_LSB;
    break;
  case ACC_TMP:
    reg_addr = ACC_TEMP;
    tmp = 1;
    break;
  default:
    return 0;
  }

  if(!tmp){
	  /*LSB and MSB bytes*/
	  read_reg(reg_addr, &tmp_buffer[0]);
	  /*wait for accel idle*/
	  clock_delay_usec(6);
	  read_reg((reg_addr + 1), &tmp_buffer[1]);

	  /*after you read with read_bytes all the register values you want
	   * ..Process them here...*/
	  ret = (((int)(tmp_buffer[1]) << 2) | ((tmp_buffer[0] >> 6) & 0x03));
  }else{
	  read_reg(reg_addr, tmp_buffer);
	  ret = (int) tmp_buffer[0];
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief write byte function for the accelerometer.
 *
 * write 1 byte
 */
void
write_reg(uint8_t reg_addr, uint8_t value)
{
  uint16_t tx_buf;
  /*add the write mask*/
  reg_addr |= ACC_WRITE_M;

  tx_buf = (uint16_t)(reg_addr << 8) | value;

  /*wait for end of transmissions if anny
   * and empty RX FIFO*/
  SPI_WAITFOREOTx();
  SPI_FLUSH();

#if SOFT_CS
  GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ACC_PORT), GPIO_PIN_MASK(ACC_CS_PIN));
#endif
  /*write byte to accelerometer*/
  SPI_WRITE(tx_buf);
#if SOFT_CS
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(ACC_PORT), GPIO_PIN_MASK(ACC_CS_PIN));
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * \brief on function for the accelerometer.
 *
 * power VDD
 */
void
accel_on(void)
{
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(ACC_PORT), GPIO_PIN_MASK(ACC_PWD_PIN));
  /*wait 2ms in order to enable accelerometer*/
  clock_delay_usec(2);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief on function for the accelerometer.
 *
 * power off VDD
 */
void
accel_off(void)
{
  GPIO_CLR_PIN(GPIO_PORT_TO_BASE(ACC_PORT), GPIO_PIN_MASK(ACC_PWD_PIN));
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    /*prepare PINS and SPI*/
    spi_init();
    spi_cs_init(ACC_PORT, ACC_CS_PIN, SOFT_CS);
    /*with 8 bit data there were some complications
     * 16 bit transmissions mode*/
    spi_set_mode(0, 0x80,0x40, 16);

    /*Configure PWD PIN*/
    GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(ACC_PORT),
                          GPIO_PIN_MASK(ACC_PWD_PIN));
    ioc_set_over(ACC_PORT, ACC_PWD_PIN, IOC_OVERRIDE_DIS);
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(ACC_PORT),
                    GPIO_PIN_MASK(ACC_PWD_PIN));
    accel_on();

    /*set Range*/
    write_reg(ACC_RANGE, ACC_RANGE_2G);
    /*set filter detection bandwidth*/
    write_reg(ACC_BW, ACC_BW_250HZ);

    if(!value)
      accel_off();
    break;
  case SENSORS_ACTIVE:
    if(value) {
      accel_on();
    } else {
      accel_off();
    } break;
  case SENSORS_INT:
    /*TBD use write_reg to configure registers and GPIO to
     * set the PINs to appropriate mode*/
    break;
  default:
    return 0;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(accel_sensor, ACCEL_SENSOR, value, configure, NULL);

/** @} */
