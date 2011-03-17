/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

/**
 * \file
 *         Device drivers for adxl345 accelerometer in Zolertia Z1.
 * \author
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 */


#include <stdio.h>
#include <signal.h>
#include "contiki.h"
#include "adxl345.h"
#include "cc2420.h"
#include "i2cmaster.h"

/* Callback pointers when interrupt occurs */
extern void (*accm_int1_cb)(u8_t reg);
extern void (*accm_int2_cb)(u8_t reg);

/* Bitmasks for the interrupts */
static uint16_t int1_mask = 0, int2_mask = 0;

/* Keep track of when the interrupt was last seen in order to reduce the amount
  of interrupts. Kind of like button debouncing. This can't be per int-pin, as
  there can be several very different int per pin (eg tap && freefall). */
// XXX Not used now, only one global timer.
//static volatile clock_time_t ints_lasttime[] = {0, 0, 0, 0, 0, 0, 0, 0};

/* Bitmasks and bit flag variable for keeping track of adxl345 status. */
enum ADXL345_STATUSTYPES {
    /* must be a bit and not more, not using 0x00. */
    INITED = 0x01,
    RUNNING = 0x02,
    STOPPED = 0x04,
    LOW_POWER = 0x08,
    AAA = 0x10,   // available to extend this...
    BBB = 0x20,   // available to extend this...
    CCC = 0x40,   // available to extend this...
    DDD = 0x80,   // available to extend this...
};
static enum ADXL345_STATUSTYPES _ADXL345_STATUS = 0x00;

/* Default values for adxl345 at startup. This will be sent to the adxl345 in a
    stream at init to set it up in a default state */
static uint8_t adxl345_default_settings[] = {
  /* Note, as the two first two bulks are to be written in a stream, they contain
    the register address as first byte in that section. */
  /* 0--14 are in one stream, start at ADXL345_THRESH_TAP */
  ADXL345_THRESH_TAP,         // XXX NB Register address, not register value!!
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
  ADXL345_BW_RATE,    // XXX NB Register address, not register value!!
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
/* Write to a register.
    args:
      reg       register to write to
      val       value to write
*/

void
accm_write_reg(u8_t reg, u8_t val) {
  u8_t tx_buf[] = {reg, val};

  i2c_transmitinit(ADXL345_ADDR);
  while (i2c_busy());
  PRINTFDEBUG("I2C Ready to TX\n");

  i2c_transmit_n(2, tx_buf);
  while (i2c_busy());
  PRINTFDEBUG("WRITE_REG 0x%02X @ reg 0x%02X\n", val, reg);
}
/*---------------------------------------------------------------------------*/
/* Write several registers from a stream.
    args:
      len       number of bytes to read
      data      pointer to where the data is read from

  First byte in stream must be the register address to begin writing to.
  The data is then written from second byte and increasing. */

void
accm_write_stream(u8_t len, u8_t *data) {
  i2c_transmitinit(ADXL345_ADDR);
  while (i2c_busy());
  PRINTFDEBUG("I2C Ready to TX(stream)\n");

  i2c_transmit_n(len, data);	// start tx and send conf reg 
  while (i2c_busy());
  PRINTFDEBUG("WRITE_STR %u B to 0x%02X\n", len, data[0]);
}

/*---------------------------------------------------------------------------*/
/* Read one register.
    args:
      reg       what register to read
    returns the value of the read register
*/

u8_t
accm_read_reg(u8_t reg) {
  u8_t retVal = 0;
  u8_t rtx = reg;
  PRINTFDEBUG("READ_REG 0x%02X\n", reg);

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
/* Read several registers in a stream.
    args:
      reg       what register to start reading from
      len       number of bytes to read
      whereto   pointer to where the data is saved
*/

void
accm_read_stream(u8_t reg, u8_t len, u8_t *whereto) {
  u8_t rtx = reg;
  PRINTFDEBUG("READ_STR %u B from 0x%02X\n", len, reg);

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
/* Read an axis of the accelerometer (x, y or z). Return value is a signed 10 bit int.
  The resolution of the acceleration measurement can be increased up to 13 bit, but
  will change the data format of this read out. Refer to the data sheet if so is
  wanted/needed. */

int16_t
accm_read_axis(enum ADXL345_AXIS axis){
  int16_t rd = 0;
  u8_t tmp[2];
  if(axis > Z_AXIS){
    return 0;
  }
  accm_read_stream(ADXL345_DATAX0 + axis, 2, &tmp[0]);
  rd = (int16_t)(tmp[0] | (tmp[1]<<8));  
  return rd;
}

/*---------------------------------------------------------------------------*/
/* Sets the g-range, ie the range the accelerometer measures (ie 2g means -2 to +2 g
    on every axis). Possible values:
        ADXL345_RANGE_2G
        ADXL345_RANGE_4G
        ADXL345_RANGE_8G
        ADXL345_RANGE_16G
    Example:
        accm_set_grange(ADXL345_RANGE_4G);
    */

void
accm_set_grange(u8_t grange){
  if(grange > ADXL345_RANGE_16G) {
    // invalid g-range.
    PRINTFDEBUG("ADXL grange invalid: %u\n", grange);
    return;
  }
  u8_t tempreg = 0;

  /* preserve the previous contents of the register */
  tempreg = (accm_read_reg(ADXL345_DATA_FORMAT) & 0xFC);  // zero out the last two bits (grange)
  tempreg |= grange;                                      // set new range
  accm_write_reg(ADXL345_DATA_FORMAT, tempreg);
}

/*---------------------------------------------------------------------------*/
/* Init the accelerometer: ports, pins, registers, interrupts (none enabled), I2C,
    default threshold values etc. */

void
accm_init(void) {
  if(!(_ADXL345_STATUS & INITED)){
    PRINTFDEBUG("ADXL345 init\n");
    _ADXL345_STATUS |= INITED;
    accm_int1_cb = NULL;
    accm_int2_cb = NULL;
    int1_event = process_alloc_event();
    int2_event = process_alloc_event();

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
    ADXL345_IES &=~ (ADXL345_INT1_PIN | ADXL345_INT2_PIN);   // low to high transition interrupts
    ADXL345_IE |= (ADXL345_INT1_PIN | ADXL345_INT2_PIN);     // enable interrupts
    eint();
  }
}

/*---------------------------------------------------------------------------*/
/* Map interrupt (FF, tap, dbltap etc) to interrupt pin (IRQ_INT1, IRQ_INT2).
    This must come after accm_init() as the registers will otherwise be overwritten. */
    
void
accm_set_irq(uint8_t int1, uint8_t int2){
  /* Set the corresponding interrupt mapping to INT1 or INT2 */
  PRINTFDEBUG("IRQs set to INT1: 0x%02X IRQ2: 0x%02X\n", int1, int2);

  int1_mask = int1;
  int2_mask = int2;

  accm_write_reg(ADXL345_INT_ENABLE, (int1 | int2));
  accm_write_reg(ADXL345_INT_MAP, int2);  // int1 bits are zeroes in the map register so this is for both ints
}

/*---------------------------------------------------------------------------*/
#if 0
/* now unused code that is later supposed to be turned into keeping track of every
    interrupt by themselves instead of only one per INT1/2 */

/* XXX MUST HAVE some way of resetting the time so that we are not suppressing
        erronous due to clock overflow.... XXX XXX XXX */
/* Table with back off time periods */
static volatile clock_time_t ints_backoffs[] = {ADXL345_INT_OVERRUN_BACKOFF, ADXL345_INT_WATERMARK_BACKOFF,
                                                ADXL345_INT_FREEFALL_BACKOFF, ADXL345_INT_INACTIVITY_BACKOFF,
                                                ADXL345_INT_ACTIVITY_BACKOFF, ADXL345_INT_DOUBLETAP_BACKOFF,
                                                ADXL345_INT_TAP_BACKOFF, ADXL345_INT_DATAREADY_BACKOFF};

/*---------------------------------------------------------------------------*/
/* Checks to see if an event occurred after backoff period (returns time period
    past since) or not (returns 0) */

static clocktime_t
backoff_passed(clocktime_t happenedAt, const clocktime_t backoff){
  if(timenow-lasttime >= backoff) {
    return 0;
  } else {
    return (timenow-lasttime);
  }
}
#endif
/*---------------------------------------------------------------------------*/
/* Invoked after an interrupt happened. Reads the interrupt source reg at the
  accelerometer, which resets the interrupts, and invokes the corresponding
  callback. It passes the source register value so the callback can determine
  what interrupt happened, if several interrupts are mapped to the same pin. */

static void
poll_handler(void){
  uint8_t ireg = 0;
  ireg = accm_read_reg(ADXL345_INT_SOURCE);
  //printf("0x%02X, 0x%02X, 0x%02X, 0x%02X\n", ireg, ireg2, int1_mask, int2_mask);

  /* Invoke callbacks for the corresponding interrupts */
  if(ireg & int1_mask){
    if(accm_int1_cb != NULL){
      PRINTFDEBUG("INT1 cb invoked\n");
      accm_int1_cb(ireg);
    }
  } else if(ireg & int2_mask){
    if(accm_int2_cb != NULL){
      PRINTFDEBUG("INT2 cb invoked\n");
      accm_int2_cb(ireg);
    }
  }
}

/*---------------------------------------------------------------------------*/
/* This process is sleeping until an interrupt from the accelerometer occurs, which
    polls this process from the interrupt service routine. */

PROCESS_THREAD(accmeter_process, ev, data) {
  PROCESS_POLLHANDLER(poll_handler());
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(0);    // should do nothing in while loop.
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* XXX This interrupt vector is shared with the interrupts from CC2420, so that
  was moved here but should find a better home. XXX */

static struct timer suppressTimer1, suppressTimer2;

interrupt(PORT1_VECTOR) port1_isr (void) {
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* ADXL345_IFG.x goes high when interrupt occurs, use to check what interrupted */
  if ((ADXL345_IFG & ADXL345_INT1_PIN) && !(ADXL345_IFG & BV(CC2420_FIFOP_PIN))){
    /* Check if this should be suppressed or not */
    if(timer_expired(&suppressTimer1)) {
      timer_set(&suppressTimer1, SUPPRESS_TIME_INT1);
      ADXL345_IFG &= ~ADXL345_INT1_PIN;   // clear interrupt flag
      process_poll(&accmeter_process);
      LPM4_EXIT;
    }
  } else if ((ADXL345_IFG & ADXL345_INT2_PIN) && !(ADXL345_IFG & BV(CC2420_FIFOP_PIN))){
    /* Check if this should be suppressed or not */
    if(timer_expired(&suppressTimer2)) {
      timer_set(&suppressTimer2, SUPPRESS_TIME_INT2);
      ADXL345_IFG &= ~ADXL345_INT2_PIN;   // clear interrupt flag
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


