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
 *         Device drivers header file for adxl345 accelerometer in Zolertia Z1.
 * \author
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *         Enric Calvo, Zolertia <ecalvo@zolertia.com>
 */

#ifndef __ADXL345_H__
#define __ADXL345_H__
#include <stdio.h>
#include "dev/i2cmaster.h"

#define DEBUGLEDS 0
#if DEBUGLEDS
  #undef LEDS_ON(x)
  #undef LEDS_OFF(x)
  #define LEDS_ON(x)    (LEDS_PxOUT &= ~x)
  #define LEDS_OFF(x)   (LEDS_PxOUT |= x)
#else
  #undef LEDS_ON
  #undef LEDS_OFF
  #define LEDS_ON(x)
  #define LEDS_OFF(x)
#endif

#define LEDS_R        0x10
#define LEDS_G        0x40
#define LEDS_B        0x20
#define L_ON(x)    (LEDS_PxOUT &= ~x)
#define L_OFF(x)   (LEDS_PxOUT |= x)

/* Used in accm_read_axis(), eg accm_read_axis(X_AXIS);*/
enum ADXL345_AXIS {
  X_AXIS = 0,
  Y_AXIS = 2,
  Z_AXIS = 4,
};

/* -------------------------------------------------------------------------- */
/* Init the accelerometer: ports, pins, registers, interrupts (none enabled), I2C,
    default threshold values etc. */
void    accm_init(void);

/* Write to a register.
    args:
      reg       register to write to
      val       value to write
*/
void    accm_write_reg(uint8_t reg, uint8_t val);

/* Write several registers from a stream.
    args:
      len       number of bytes to read
      data      pointer to where the data is read from
  First byte in stream must be the register address to begin writing to.
  The data is then written from the second byte and increasing. The address byte
  is not included in length len. */
void    accm_write_stream(uint8_t len, uint8_t *data);

/* Read one register.
    args:
      reg       what register to read
    returns the value of the read register
*/
uint8_t    accm_read_reg(uint8_t reg);

/* Read several registers in a stream.
    args:
      reg       what register to start reading from
      len       number of bytes to read
      whereto   pointer to where the data is saved
*/
void    accm_read_stream(uint8_t reg, uint8_t len, uint8_t *whereto);

/* Read an axis of the accelerometer (x, y or z). Return value is a signed 10 bit int.
  The resolution of the acceleration measurement can be increased up to 13 bit, but
  will change the data format of this read out. Refer to the data sheet if so is
  wanted/needed. */
int16_t accm_read_axis(enum ADXL345_AXIS axis);

/* Sets the g-range, ie the range the accelerometer measures (ie 2g means -2 to +2 g
    on every axis). Possible values:
        ADXL345_RANGE_2G
        ADXL345_RANGE_4G
        ADXL345_RANGE_8G
        ADXL345_RANGE_16G
    Example:
        accm_set_grange(ADXL345_RANGE_4G);
    */
void    accm_set_grange(uint8_t grange);

/* Map interrupt (FF, tap, dbltap etc) to interrupt pin (IRQ_INT1, IRQ_INT2).
    This must come after accm_init() as the registers will otherwise be overwritten. */
void    accm_set_irq(uint8_t int1, uint8_t int2);

/* Macros for setting the pointers to callback functions from the interrupts.
  The function will be called with an uint8_t as parameter, containing the interrupt
  flag register from the ADXL345. That way, several interrupts can be mapped to
  the same pin and be read from the  */
#define ACCM_REGISTER_INT1_CB(ptr)   accm_int1_cb = ptr;
#define ACCM_REGISTER_INT2_CB(ptr)   accm_int2_cb = ptr;
/* -------------------------------------------------------------------------- */
/* Application definitions, change if required by application. */

/* Interrupt suppress periods */
/*
// XXX Not used yet.
#define ADXL345_INT_OVERRUN_BACKOFF     CLOCK_SECOND/8
#define ADXL345_INT_WATERMARK_BACKOFF   CLOCK_SECOND/8
#define ADXL345_INT_FREEFALL_BACKOFF    CLOCK_SECOND/8
#define ADXL345_INT_INACTIVITY_BACKOFF  CLOCK_SECOND/8
#define ADXL345_INT_ACTIVITY_BACKOFF    CLOCK_SECOND/8
#define ADXL345_INT_DOUBLETAP_BACKOFF   CLOCK_SECOND/8
#define ADXL345_INT_TAP_BACKOFF         CLOCK_SECOND/8
#define ADXL345_INT_DATAREADY_BACKOFF   CLOCK_SECOND/8
*/
/* Time after an interrupt that subsequent interrupts are suppressed. Should later
  be turned into one specific time per type of interrupt (tap, freefall etc) */
#define SUPPRESS_TIME_INT1    CLOCK_SECOND/4
#define SUPPRESS_TIME_INT2    CLOCK_SECOND/4

/* Suggested defaults according to the data sheet etc */
#define ADXL345_THRESH_TAP_DEFAULT      0x48    // 4.5g (0x30 == 3.0g) (datasheet: 3g++)
#define ADXL345_OFSX_DEFAULT            0x00    // for individual units calibration purposes
#define ADXL345_OFSY_DEFAULT            0x00
#define ADXL345_OFSZ_DEFAULT            0x00
#define ADXL345_DUR_DEFAULT             0x20    // 20 ms (datasheet: 10ms++)
#define ADXL345_LATENT_DEFAULT          0x50    // 100 ms (datasheet: 20ms++)
#define ADXL345_WINDOW_DEFAULT          0xFF    // 320 ms (datasheet: 80ms++)
#define ADXL345_THRESH_ACT_DEFAULT      0x15    // 1.3g (62.5 mg/LSB)
#define ADXL345_THRESH_INACT_DEFAULT    0x08    // 0.5g (62.5 mg/LSB)
#define ADXL345_TIME_INACT_DEFAULT      0x02    // 2 s (1 s/LSB)
#define ADXL345_ACT_INACT_CTL_DEFAULT   0xFF    // all axis involved, ac-coupled
#define ADXL345_THRESH_FF_DEFAULT       0x09    // 563 mg
#define ADXL345_TIME_FF_DEFAULT         0x20    // 160 ms
#define ADXL345_TAP_AXES_DEFAULT        0x07    // all axis, no suppression

#define ADXL345_BW_RATE_DEFAULT         (0x00|ADXL345_SRATE_100)   // 100 Hz, normal operation
#define ADXL345_POWER_CTL_DEFAULT       0x28	  // link bit set, no autosleep, start normal measuring
#define ADXL345_INT_ENABLE_DEFAULT      0x00    // no interrupts enabled
#define ADXL345_INT_MAP_DEFAULT         0x00    // all mapped to int_1

/* XXX NB: In the data format register, data format of axis readings is chosen
  between left or right justify. This affects the position of the MSB/LSB and is
  different depending on g-range and resolution. If changed, make sure this is
  reflected in the _read_axis() function. Also, the resolution can be increased
  from 10 bit to at most 13 bit, but this also changes position of MSB etc on data
  format so check this in read_axis() too. */
#define ADXL345_DATA_FORMAT_DEFAULT     (0x00|ADXL345_RANGE_2G)    // right-justify, 2g, 10-bit mode, int is active high
#define ADXL345_FIFO_CTL_DEFAULT        0x00    // FIFO bypass mode

/* -------------------------------------------------------------------------- */
/* Reference definitions, should not be changed */
/* adxl345 slave address */
#define ADXL345_ADDR            0x53

/* ADXL345 registers */
#define ADXL345_DEVID           0x00    // read only
/* registers 0x01 to 0x1C are reserved, do not access */
#define ADXL345_THRESH_TAP      0x1D
#define ADXL345_OFSX            0x1E
#define ADXL345_OFSY            0x1F
#define ADXL345_OFSZ            0x20
#define ADXL345_DUR             0x21
#define ADXL345_LATENT          0x22
#define ADXL345_WINDOW          0x23
#define ADXL345_THRESH_ACT      0x24
#define ADXL345_THRESH_INACT    0x25
#define ADXL345_TIME_INACT      0x26
#define ADXL345_ACT_INACT_CTL   0x27
#define ADXL345_THRESH_FF       0x28
#define ADXL345_TIME_FF         0x29
#define ADXL345_TAP_AXES        0x2A
#define ADXL345_ACT_TAP_STATUS  0x2B    // read only
#define ADXL345_BW_RATE         0x2C
#define ADXL345_POWER_CTL       0x2D
#define ADXL345_INT_ENABLE      0x2E
#define ADXL345_INT_MAP         0x2F
#define ADXL345_INT_SOURCE      0x30    // read only
#define ADXL345_DATA_FORMAT     0x31
#define ADXL345_DATAX0          0x32    // read only, LSByte X, two's complement
#define ADXL345_DATAX1          0x33    // read only, MSByte X
#define ADXL345_DATAY0          0x34    // read only, LSByte Y
#define ADXL345_DATAY1          0x35    // read only, MSByte X
#define ADXL345_DATAZ0          0x36    // read only, LSByte Z
#define ADXL345_DATAZ1          0x37    // read only, MSByte X
#define ADXL345_FIFO_CTL        0x38
#define ADXL345_FIFO_STATUS     0x39    // read only

/* ADXL345 interrupts */
#define ADXL345_INT_DISABLE     0X00    // used for disabling interrupts
#define ADXL345_INT_OVERRUN     0X01
#define ADXL345_INT_WATERMARK   0X02
#define ADXL345_INT_FREEFALL    0X04
#define ADXL345_INT_INACTIVITY  0X08
#define ADXL345_INT_ACTIVITY    0X10
#define ADXL345_INT_DOUBLETAP   0X20
#define ADXL345_INT_TAP         0X40
#define ADXL345_INT_DATAREADY   0X80

/* Accelerometer hardware ports, pins and registers on the msp430 µC */
#define ADXL345_DIR        P1DIR
#define ADXL345_PIN        P1PIN
#define ADXL345_REN        P1REN
#define ADXL345_SEL        P1SEL
#define ADXL345_SEL2       P1SEL2
#define ADXL345_INT1_PIN   (1<<6)            // P1.6
#define ADXL345_INT2_PIN   (1<<7)            // P1.7
#define ADXL345_IES        P1IES
#define ADXL345_IE         P1IE
#define ADXL345_IFG        P1IFG
#define ADXL345_VECTOR     PORT1_VECTOR

/* g-range for DATA_FORMAT register */
#define ADXL345_RANGE_2G    0x00
#define ADXL345_RANGE_4G    0x01
#define ADXL345_RANGE_8G    0x02
#define ADXL345_RANGE_16G   0x03


/* The adxl345 has programmable sample rates, but unexpected results may occur if the wrong 
  rate and I2C bus speed is used (see datasheet p 17). Sample rates in Hz. This
  setting does not change the internal sampling rate, just how often it is piped
  to the output registers (ie the interrupt features use the full sample rate
  internally).

  Example use:
    adxl345_set_reg(ADXL345_BW_RATE, ((_ADXL345_STATUS & LOW_POWER) | ADXL345_SRATE_50));
  */
#define ADXL345_SRATE_3200      0x0F    // XXX NB don't use at all as I2C data rate<= 400kHz (see datasheet)
#define ADXL345_SRATE_1600      0x0E    // XXX NB don't use at all as I2C data rate<= 400kHz (see datasheet)
#define ADXL345_SRATE_800       0x0D    // when I2C data rate == 400 kHz
#define ADXL345_SRATE_400       0x0C    // when I2C data rate == 400 kHz
#define ADXL345_SRATE_200       0x0B    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_100       0x0A    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_50        0x09    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_25        0x08    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_12_5      0x07    // 12.5 Hz, when I2C data rate >= 100 kHz
#define ADXL345_SRATE_6_25      0x06    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_3_13      0x05    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_1_56      0x04    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_78      0x03    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_39      0x02    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_20      0x01    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_10      0x00    // 0.10 Hz, when I2C data rate >= 100 kHz

/* Callback pointers for the interrupts */
extern void (*accm_int1_cb)(uint8_t reg);
extern void (*accm_int2_cb)(uint8_t reg);

/* Interrupt 1 and 2 events; ADXL345 signals interrupt on INT1 or INT2 pins,
  ISR is invoked and polls the accelerometer process which invokes the callbacks. */
extern process_event_t int1_event, int2_event;   // static ?

#define ACCM_INT1    0x01
#define ACCM_INT2    0x02


/* -------------------------------------------------------------------------- */
#endif /* ifndef __ADXL345_H__ */
