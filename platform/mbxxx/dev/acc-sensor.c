/**
 * \addtogroup mbxxx-platform
 *
 * @{
 */
/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*			Accelerometer.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/
/*---------------------------------------------------------------------------*/


#include "dev/acc-sensor.h"
#include "sys/clock.h"
#include "mems.h"

#define FALSE 0
#define TRUE  1

/*---------------------------------------------------------------------------*/
static int
active(void)
{
  uint8_t reg;
  if(!MEMS_Read_Reg (kLIS3L02DQ_SLAVE_ADDR, CTRL_REG1, &reg, 1))
    return FALSE;

  return (reg & 0x40) ? TRUE : FALSE ;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  int8_t i2c_data = 0;
  uint8_t reg_addr;

  switch(type) {
    case ACC_X_AXIS:
      reg_addr = OUTX_H;
      break;

    case ACC_Y_AXIS:
      reg_addr = OUTY_H;
      break;

    case ACC_Z_AXIS:
      reg_addr = OUTZ_H;
      break;

    default:
      return 0;
  }

  MEMS_Read_Reg(kLIS3L02DQ_SLAVE_ADDR, reg_addr, (uint8_t *)&i2c_data, 1);

  if(MEMS_GetFullScale()==ACC_HIGH_RANGE){
    return ((int16_t)i2c_data) * HIGH_RANGE_SENSITIVITY;
  }
  else {
    return ((int16_t)i2c_data) * LOW_RANGE_SENSITIVITY;
  }
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {

    case SENSORS_HW_INIT:
       return MEMS_Init();

    case SENSORS_ACTIVE:
      if(value){
          if(MEMS_On()){
              clock_wait(8);
              return 1;
          }
          return 0;
      }
      else
        return MEMS_Off();

    case ACC_RANGE:
      return MEMS_SetFullScale((boolean)value);

    case ACC_HPF:
      if(value < ACC_HPF_DISABLE){
        return MEMS_Write_Reg(kLIS3L02DQ_SLAVE_ADDR, CTRL_REG2,
			      (1<<4) | (uint8_t)value);
      }
      else {
        return MEMS_Write_Reg(kLIS3L02DQ_SLAVE_ADDR, CTRL_REG2, 0x00);
      }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {

  case SENSORS_READY:
    return active();
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(acc_sensor, ACC_SENSOR, value, configure, status);


/** @} */
