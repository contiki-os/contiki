/*
 * Copyright (c) 2015, Copyright Markus Hidell, Robert Olsson
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
 * Authors  : Maximus Byamukama, Flavia Nshemerirwe
 * Created :
 */

#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <i2c.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include "dev/watchdog.h"
#include "lib/list.h"
#include "lib/memb.h"
#include <compat/twi.h>
#include "dev/ds1307.h"

uint8_t am_pm_flag = 0;

/*initialize DS1307 RTC*/
void
DS1307_init(void)
{
	i2c_init(F_SCL);
	i2c_start(I2C_DS1307_ADDR);
	i2c_write(I2C_DS1307_ADDR+TW_WRITE);
	i2c_write(CONTROL_REGISTER);
	i2c_write(0x00);
	i2c_stop();
}

/*Convert bcd value to decimal value*/
uint8_t
bcd_to_dec(uint8_t val)
{
	return( (val/16*10) + (val%16) );
}

/*Convert decimal value to bcd value */
uint8_t
dec_to_bcd(uint8_t val)
{
	return( (val/10*16) + (val%10) );
}

/* set time */
void
set_seconds(uint8_t sec)
{
	i2c_write_mem(I2C_DS1307_ADDR, REG_SECS, dec_to_bcd(sec));
}

void
set_minutes(uint8_t min){
	i2c_write_mem(I2C_DS1307_ADDR, REG_MINS, dec_to_bcd(min));
}

void
set_hours(uint8_t hrs)
{
	i2c_write_mem(I2C_DS1307_ADDR, REG_HRS, dec_to_bcd(hrs));
}

/* set date */
void
set_day(uint8_t day)
{
	i2c_write_mem(I2C_DS1307_ADDR, REG_DAYS, dec_to_bcd(day));
}

void
set_month(uint8_t month)
{
	i2c_write_mem(I2C_DS1307_ADDR, REG_MONTH, dec_to_bcd(month));
}

/* accepted values for year 00-99 */
void
set_year(uint8_t yrs)
{
	i2c_write_mem(I2C_DS1307_ADDR, REG_YRS, dec_to_bcd(yrs));
}

/* get time */
uint8_t
get_seconds()
{
	uint8_t sec[1];
	i2c_read_mem(I2C_DS1307_ADDR,REG_SECS, sec, 1);
	return bcd_to_dec(sec[0]);
}

uint8_t
get_minutes()
{
	uint8_t min[1];
	i2c_read_mem(I2C_DS1307_ADDR,REG_MINS, min, 1);
	return bcd_to_dec(min[0]);
}

uint8_t
get_hours()
{
	uint8_t hrs[1];
	i2c_read_mem(I2C_DS1307_ADDR,REG_HRS, hrs, 1);
	return bcd_to_dec(hrs[0]);
}

/*get date*/
uint8_t
get_day()
{
	uint8_t day[1];
	i2c_read_mem(I2C_DS1307_ADDR, REG_DAYS, day, 1);
	return bcd_to_dec(day[0]);
}

uint8_t
get_month()
{
	uint8_t month[1];
	i2c_read_mem(I2C_DS1307_ADDR, REG_MONTH, month, 1);
	return bcd_to_dec(month[0]);
}

uint16_t
get_year()
{
	uint8_t year[1];
	i2c_read_mem(I2C_DS1307_ADDR, REG_YRS, year, 1);
	return bcd_to_dec(year[0])+2000;
}

static int
status(int type)
{
  return 0;
}
static int
configure(int type, int c)
{
	switch (type){
		case SET_SEC:
			set_seconds(c);
			break;
        case SET_MIN:
			set_minutes(c);
			break;
		case SET_HOURS:
			set_hours(c);
			break;
		case SET_DAY:
			set_day(c);
			break;
		case SET_MONTH:
			set_month(c);
			break;
		case SET_YEAR:
			set_year(c);
			break;
  }
  return 0;
}
static int
value(int var)
{
	int val = 0;
	switch (var){
		case GET_SEC:
			val = get_seconds();
			break;
		case GET_MIN:
			val = get_minutes();
			break;
		case GET_HOURS:
			val = get_hours();
			break;
		case GET_DAY:
			val = get_day();
			break;
		case GET_MONTH:
			val = get_month();
			break;
		case GET_YEAR:
			val = get_year();
			break;
  }
  return val;
}
SENSORS_SENSOR(ds1307_sensor, DS1307_RTC, value, configure, status);