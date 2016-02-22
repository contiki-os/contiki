/*
 * Copyright (c) 2015, Copyright Per Lindgren <per.o.lindgren@gmail.com>
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
 *
 * Author   : Per Lindgren <per.o.lindgren@gmail.com>
 * Hacked by: Robert Olsson robert@radio-sensors.com
 * Created : 2015-11-22
 */

#include "contiki.h"
#include "dev/temp-sensor.h"
#include <util/delay_basic.h>
#define delay_us(us)   (_delay_loop_2(1 + (us * F_CPU) / 4000000UL))

const struct sensors_sensor temp_mcu_sensor;

/* probe_for_ds18b20 probes for the sensor. Returns 0 on failure, 1 on success
 * Assumptions: only one sensor on the "1-wire bus", on port WSN_DS18B20_PORT
 * BUG: THIS CODE DOES NOT WORK AS INTENDED! IT RETURNS "1" EVEN WHEN THERE
 * IS NO SENSOR CONNECTED.
 */

uint8_t
ds18b20_probe(void)
{
  uint8_t result = 0;

  /* Reset 1W-bus */

  /* Pull PIN low for 480 microseconds (us)
   * Start with setting bit DS18B20_1_PIN to 0 */
  OW_SET_PIN_LOW();
  /* then set direction to OUT by setting DS18B20_1_DDR bit to 1 */
  OW_SET_OUTPUT();
  /* Delay 480 us */
  clock_delay_usec(480);
  /* See if sensor responds. First release the bus and switch to INput mode
   * by setting DS18B20_1_DDR bit to 0 */
  OW_SET_INPUT();
  /* Activate internal pull-up by  setting pin to HIGH (when in INput mode)
   * OW_SET_PIN_HIGH();
   * Wait for the pin to go HIGH for 64 us */
  clock_delay_usec(64);
  /* Now the sensor, if present, pulls the pin LOW for 60-240 us
   * Detect 0 on PIND bit DS18B20_1_PIN. Invert the result so a presence
   * (aka * a 0)  sets "result" to 1 (for success) */
  result = !OW_GET_PIN_STATE();

  /* The sensor releases the pin so it goes HIGH after 240 us, add some
     for the signal to stabilize, say 300 usecs to be on the safe side? */
  if(result) {
    clock_delay_usec(300);
    /* Now the bus should be HIGH again */
    result = OW_GET_PIN_STATE();
  }
  return result;
}
/* Write 1 or 0 on the bus */

void
write_bit(uint8_t bit)
{
  /* Set pin to 0 */
  OW_SET_OUTPUT();
  OW_SET_PIN_LOW();

  /* Pin should be 0 for at least 1 us */
  clock_delay_usec(2);

  /* If we're writing a 1, let interna pull-up pull the bus high
   * within 15 us of setting the bus to low */
  if(bit) {
    /* Internal pull-up is activated by setting direction to IN and the
     * setting the pin to HIGH */
    OW_SET_INPUT();
    OW_SET_PIN_HIGH();
  }
  /* OK, now the bus is either LOW, or pulled HIGH by the internal pull-up
   * Let this state remain for 60 us, then release the bus */
  clock_delay_usec(60);

  /* Release the bus */
  OW_SET_PIN_HIGH();
  OW_SET_INPUT();

  /* Allow > 1 us between read/write operations */
  clock_delay_usec(2);
}
/* Read one bit of information from the bus, and return it as 1 or 0 */

uint8_t
read_bit(void)
{
  uint8_t bit = 0;

  /* Set pin to 0 */
  OW_SET_OUTPUT();
  OW_SET_PIN_LOW();

  /* Pin should be 0 for at least 1 us */
  clock_delay_usec(2);

  /* Now read the bus, start by setting in/out direction and activating
   * internal pull-up resistor */
  OW_SET_INPUT();
  OW_SET_PIN_HIGH();

  /* ds18b20 either keeps the pin down or releases the bus and the
   * bus then goes high because of the interna pull-up resistor
   * Check whichever happens before 15 us has passed */
  clock_delay_usec(15 - 2 - 1);
  bit = OW_GET_PIN_STATE();

  /* The complete read cycle must last at least 60 us. We have now spent
   * about 14-15 us in delays, so add another delay to reach >= 60 us */
  clock_delay_usec(50);

  /* Release bus */
  OW_SET_PIN_HIGH();
  OW_SET_INPUT();

  /* Allow > 1 us between read/write operations */
  clock_delay_usec(2);

  return bit ? 1 : 0;
}
/* Read one byte of information. A byte is read least significant bit first */

uint8_t
read_byte(void)
{
  uint8_t result = 0;
  uint8_t bit;
  int i;

  for(i = 0; i < 8; i++) {
    bit = read_bit();
    result += (bit << i);
  }
  return result;
}
/* Write one byte of information. A byte is written least significant bit first */

void
write_byte(uint8_t byte)
{
  int i;

  for(i = 0; i < 8; i++) {
    write_bit((byte >> i) & 1);
  }
}
/* ds18b20_get_temp returns the temperature in "temp" (in degrees celsius)
 * Returns 0 on failure (and then "temp" is left unchanged
 * Returns 1 on success, and sets temp */

uint8_t
ds18b20_get_temp(double *temp)
{
  uint8_t result = 0;

  /* Reset bus by probing. Probe returns 1 on success/presence of sensor */
  if(ds18b20_probe()) {
    /* write command "skip rom" since we only have one sensor on the wire! */
    write_byte(DS18B20_COMMAND_SKIP_ROM);

    /* write command to start measurement */
    write_byte(DS18B20_COMMAND_START_CONVERSION);

    /* Wait for conversion to complete. Conversion is 12-bit by default.
     * Since we have external power to the sensor (ie not in "parasitic power"
     * mode) the bus is held LOW by the sensor while the conversion is going
     * on, and then HIGH when conversion is finished. */
    OW_SET_INPUT();
    int count = 0;
    while(!OW_GET_PIN_STATE()) {
      clock_delay_msec(10);
      count++;
      /* Longest conversion time is 750 ms (12-bit resolution)
       * So if count > 80 (for a little margin!), we return -274.0
       * which indicates failure to read the temperature. */
      if(count > 80) {
        return 0;
      }
    }

    /* The result is stored in the "scratch pad", a 9 byte memory block.
     * The first two bytes are the conversion result. Reading the scratch pad
     * can be terminated by sending a reset signal (but we read all 9 bytes) */
    (void)ds18b20_probe();
    write_byte(DS18B20_COMMAND_SKIP_ROM);
    write_byte(DS18B20_COMMAND_READ_SCRATCH_PAD);
    uint8_t i, sp_arr[9];
    for(i = 0; i < 9; i++) {
      sp_arr[i] = read_byte();
    }

    /* Check CRC, if mismatch, return 0 (failure to read temperature) */
    uint8_t crc_cal = crc8_ds18b20(sp_arr, 8);

    if(crc_cal != sp_arr[8]) {
      return 0;
    }

    /* OK, now decode what the temperature reading is. This code assumes
     * 12-bit resolution, so this must be modified if the code is modified
     * to use any other resolution! */
    int16_t temp_res;
    uint8_t temp_lsb = sp_arr[0];
    uint8_t temp_msb = sp_arr[1];

    temp_res = (int16_t)temp_msb << 8 | temp_lsb;
    *temp = (double)temp_res * 0.0625;

    result = 1;
  }
  return result;
}
/* crc8 algorithm for ds18b20 */
/* http://www.miscel.dk/MiscEl/CRCcalculations.html */

uint8_t
crc8_ds18b20(uint8_t *buf, uint8_t buf_len)
{
  uint8_t result = 0;
  uint8_t i, b;

  for(i = 0; i < buf_len; i++) {
    result = result ^ buf[i];
    for(b = 1; b < 9; b++) {
      if(result & 0x1) {
        result = (result >> 1) ^ 0x8C;
      } else {
        result = result >> 1;
      }
    }
  }
  return result;
}
static int
value(int type)
{
  double t;
  int ret;
  ret = ds18b20_get_temp(&t);
  
  /* Return temp multiplied by 100 for two decimals */
  if(ret) 
    return (int) (t * 100);

  /* Error return largest negative value */
  return 0x8000;
}
static int
configure(int type, int c)
{
  ds18b20_probe();
  return 0;
}
static int
status(int type)
{
  return 1;
}
SENSORS_SENSOR(temp_sensor, TEMP_SENSOR, value, configure, status);
