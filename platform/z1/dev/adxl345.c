/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * 3. Neither the name of the Institute nor the names of its contributors
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
 *
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Device drivers for adxl345 accelerometer in Zolertia Z1.
 * \author
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Antonio Lignan, Zolertia <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "adxl345.h"
#include "cc2420.h"
#include "i2cmaster.h"
#include "isr_compat.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
/*---------------------------------------------------------------------------*/
/* Callback pointers when interrupt occurs */
void (*accm_int1_cb)(uint8_t reg);
void (*accm_int2_cb)(uint8_t reg);
/*---------------------------------------------------------------------------*/
/* Bitmasks for the interrupts */
static uint16_t int1_mask = 0, int2_mask = 0;

/* Default values for adxl345 at startup.
 * This will be sent to the adxl345 in a
 * stream at init to set it up in a default state
 */

static uint8_t adxl345_default_settings[] = {
  /* Note, as the two first two bulks are to be written in a stream, they contain
   * the register address as first byte in that section.
   * 0--14 are in one stream, start at ADXL345_THRESH_TAP
   */
  /* XXX NB Register address, not register value!! */
  ADXL345_THRESH_TAP,
  ADXL345_THRESH_TAP_DEFAULT,
  ADXL345_OFSX_DEFAULT,
  ADXL345_OFSY_DEFAULT,
  ADXL345_OFSZ_DEFAULT,
  ADXL345_DUR_DEFAULT,
  ADXL345_LATENT_DEFAULT,
  ADXL345_WINDOW_DEFAULT,
  ADXL345_THRESH_ACT_DEFAULT,
  ADXL345_THRESH_INACT_DEFAULT,
  ADXL345_TIME_INACT_DEFAULT,
  ADXL345_ACT_INACT_CTL_DEFAULT,
  ADXL345_THRESH_FF_DEFAULT,
  ADXL345_TIME_FF_DEFAULT,
  ADXL345_TAP_AXES_DEFAULT,

  /* 15--19 start at ADXL345_BW_RATE */
  /* XXX NB Register address, not register value!! */
  ADXL345_BW_RATE,    
  ADXL345_BW_RATE_DEFAULT,
  ADXL345_POWER_CTL_DEFAULT,
  ADXL345_INT_ENABLE_DEFAULT,
  ADXL345_INT_MAP_DEFAULT,

  /* These two: 20, 21 write separately */
  ADXL345_DATA_FORMAT_DEFAULT,
  ADXL345_FIFO_CTL_DEFAULT
};
/*---------------------------------------------------------------------------*/
PROCESS(accmeter_process, "Accelerometer process");
/*---------------------------------------------------------------------------*/
static void
accm_write_reg(uint8_t reg, uint8_t val)
{
  uint8_t tx_buf[] = {reg, val};

  i2c_transmitinit(ADXL345_ADDR);
  while (i2c_busy());
  PRINTF("ADXL345: I2C Ready to TX\n");

  i2c_transmit_n(2, tx_buf);
  while (i2c_busy());
  PRINTF("ADXL345: WRITE_REG 0x%02X @ reg 0x%02X\n", val, reg);
}
/*---------------------------------------------------------------------------*/
/* First byte in stream must be the register address to begin writing to.
 * The data is then written from second byte and increasing.
 */
static void
accm_write_stream(uint8_t len, uint8_t *data)
{
  i2c_transmitinit(ADXL345_ADDR);
  while (i2c_busy());
  PRINTF("ADXL345: I2C Ready to TX(stream)\n");

  i2c_transmit_n(len, data);	// start tx and send conf reg 
  while (i2c_busy());
  PRINTF("ADXL345: WRITE_STR %u B to 0x%02X\n", len, data[0]);
}

/*---------------------------------------------------------------------------*/
static uint8_t
accm_read_reg(uint8_t reg)
{
  uint8_t retVal = 0;
  uint8_t rtx = reg;
  PRINTF("ADXL345: READ_REG 0x%02X\n", reg);

  /* transmit the register to read */
  i2c_transmitinit(ADXL345_ADDR);
  while (i2c_busy());
  i2c_transmit_n(1, &rtx);
  while (i2c_busy());

  /* receive the data */
  i2c_receiveinit(ADXL345_ADDR);
  while (i2c_busy());
  i2c_receive_n(1, &retVal);
  while (i2c_busy());

  return retVal;
}
/*---------------------------------------------------------------------------*/
static void
accm_read_stream(uint8_t reg, uint8_t len, uint8_t *whereto)
{
  uint8_t rtx = reg;
  PRINTF("ADXL345: READ_STR %u B from 0x%02X\n", len, reg);

  /* transmit the register to start reading from */
  i2c_transmitinit(ADXL345_ADDR);
  while (i2c_busy());
  i2c_transmit_n(1, &rtx);
  while (i2c_busy());

  /* receive the data */
  i2c_receiveinit(ADXL345_ADDR);
  while (i2c_busy());
  i2c_receive_n(len, whereto);
  while (i2c_busy());
}

/*---------------------------------------------------------------------------*/
/* Read an axis of the accelerometer (x, y or z). Return value is a signed
 * 10 bit int.
 * The resolution of the acceleration measurement can be increased up to 13 bit,
 * but will change the data format of this read out. Refer to the data sheet if
 * so is wanted/needed.
 */
int16_t
accm_read_axis(enum ADXL345_AXIS axis)
{
  int16_t rd = 0;
  uint8_t tmp[2];
  if(axis > Z_AXIS){
    return 0;
  }
  accm_read_stream(ADXL345_DATAX0 + axis, 2, &tmp[0]);
  rd = (int16_t)(tmp[0] | (tmp[1]<<8));  
  return rd;
}
/*---------------------------------------------------------------------------*/
int
accm_set_grange(uint8_t grange)
{
  uint8_t tempreg = 0;

  if(grange > ADXL345_RANGE_16G) {
    PRINTF("ADXL345: grange invalid: %u\n", grange);
    return ADXL345_ERROR;
  }

  if(!enabled) {
    return ADXL345_ERROR;
  }

  /* Keep the previous contents of the register, zero out the last two bits */
  tempreg = (accm_read_reg(ADXL345_DATA_FORMAT) & 0xFC);
  tempreg |= grange;
  accm_write_reg(ADXL345_DATA_FORMAT, tempreg);
  return ADXL345_SUCCESS;
}

/*---------------------------------------------------------------------------*/
void
accm_init(void)
{
  PRINTF("ADXL345: init\n");
  accm_int1_cb = NULL;
  accm_int2_cb = NULL;

  /* Set up ports and pins for interrups. */
  ADXL345_DIR  &=~ (ADXL345_INT1_PIN | ADXL345_INT2_PIN);
  ADXL345_SEL  &=~ (ADXL345_INT1_PIN | ADXL345_INT2_PIN);
  ADXL345_SEL2 &=~ (ADXL345_INT1_PIN | ADXL345_INT2_PIN);

  /* Set up ports and pins for I2C communication */
  i2c_enable();

  /* set default register values. */
  accm_write_stream(15, &adxl345_default_settings[0]);
  accm_write_stream(5, &adxl345_default_settings[15]);
  accm_write_reg(ADXL345_DATA_FORMAT, adxl345_default_settings[20]);
  accm_write_reg(ADXL345_FIFO_CTL, adxl345_default_settings[21]);

  process_start(&accmeter_process, NULL);

  /* Enable msp430 interrupts on the two interrupt pins. */
  dint();
  /* low to high transition interrupts */
  ADXL345_IES &=~ (ADXL345_INT1_PIN | ADXL345_INT2_PIN);
  /* enable interrupts */
  ADXL345_IE |= (ADXL345_INT1_PIN | ADXL345_INT2_PIN);
  eint();

  enabled = 1;
}
/*---------------------------------------------------------------------------*/
void
accm_stop(void)
{
  dint();
  ADXL345_IE &= ~(ADXL345_INT1_PIN | ADXL345_INT2_PIN);
  accm_write_reg(ADXL345_INT_ENABLE, ~(int1_mask | int2_mask));
  accm_write_reg(ADXL345_INT_MAP, ~int2_mask);
  eint();
  enabled = 0;
}
/*---------------------------------------------------------------------------*/
int
accm_set_irq(uint8_t int1, uint8_t int2)
{
  if(!enabled) {
    return ADXL345_ERROR;
  }

  /* Set the corresponding interrupt mapping to INT1 or INT2 */
  PRINTF("ADXL345: IRQs set to INT1: 0x%02X IRQ2: 0x%02X\n", int1, int2);

  int1_mask = int1;
  int2_mask = int2;

  accm_write_reg(ADXL345_INT_ENABLE, (int1 | int2));
  /* int1 bits are zeroes in the map register so this is for both ints */
  accm_write_reg(ADXL345_INT_MAP, int2);
  return ADXL345_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/* Invoked after an interrupt happened. Reads the interrupt source reg at the
 * accelerometer, which resets the interrupts, and invokes the corresponding
 * callback. It passes the source register value so the callback can determine
 * what interrupt happened, if several interrupts are mapped to the same pin.
 */
static void
poll_handler(void)
{
  uint8_t ireg = 0;
  ireg = accm_read_reg(ADXL345_INT_SOURCE);

  /* Invoke callbacks for the corresponding interrupts */
  if(ireg & int1_mask){
    if(accm_int1_cb != NULL){
      PRINTF("ADXL345: INT1 cb invoked\n");
      accm_int1_cb(ireg);
    }
  } else if(ireg & int2_mask){
    if(accm_int2_cb != NULL){
      PRINTF("ADXL345: INT2 cb invoked\n");
      accm_int2_cb(ireg);
    }
  }
}
/*---------------------------------------------------------------------------*/
/* This process is sleeping until an interrupt from the accelerometer occurs,
 * which polls this process from the interrupt service routine. */
PROCESS_THREAD(accmeter_process, ev, data)
{
  PROCESS_POLLHANDLER(poll_handler());
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(0);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* This interrupt vector is shared with the interrupts from CC2420, so that
 * was moved here
 */
static struct timer suppressTimer1, suppressTimer2;

ISR(PORT1, port1_isr)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* ADXL345_IFG.x goes high when interrupt occurs, use to check what
   * interrupted
   */
  if((ADXL345_IFG & ADXL345_INT1_PIN) && !(ADXL345_IFG & BV(CC2420_FIFOP_PIN))){
    /* Check if this should be suppressed or not */
    if(timer_expired(&suppressTimer1)) {
      timer_set(&suppressTimer1, SUPPRESS_TIME_INT1);
      ADXL345_IFG &= ~ADXL345_INT1_PIN;   // clear interrupt flag
      process_poll(&accmeter_process);
      LPM4_EXIT;
    }
  } else if((ADXL345_IFG & ADXL345_INT2_PIN) &&
           !(ADXL345_IFG & BV(CC2420_FIFOP_PIN))){
    /* Check if this should be suppressed or not */
    if(timer_expired(&suppressTimer2)) {
      timer_set(&suppressTimer2, SUPPRESS_TIME_INT2);
      /* clear interrupt flag */
      ADXL345_IFG &= ~ADXL345_INT2_PIN;
      process_poll(&accmeter_process);
      LPM4_EXIT;
    }
  } else {
    /* CC2420 interrupt */
    if(cc2420_interrupt()) {
      LPM4_EXIT;
    }
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != SENSORS_ACTIVE) {
    return ADXL345_ERROR;
  }

  if(value) {
    accm_init();
  } else {
    accm_stop();
  }
  enabled = value;
  return ADXL345_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return enabled;
  }
  return ADXL345_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if(!enabled) {
    return ADXL345_ERROR;
  }

  if((type != X_AXIS) && (type != Y_AXIS) && (type != Z_AXIS)) {
    return ADXL345_ERROR;
  }

  switch(type) {
    case X_AXIS:
      return accm_read_axis(X_AXIS);
    case Y_AXIS:
      return accm_read_axis(Y_AXIS);
    case Z_AXIS:
      return accm_read_axis(Z_AXIS);
    default:
      return ADXL345_ERROR;
  }
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(adxl345, ADXL345_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
