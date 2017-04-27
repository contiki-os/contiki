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
 *
 * Authors  : Maximus Byamukama, Flavia Nshemerirwe
 * Created :
 * This file is part of the Contiki operating system.
 */

#ifndef DS1307_SENSOR_H_
#define DS1307_SENSOR_H_

/*DS1307 RTC ROUTINES*/
#define REG_SECS 0x00
#define REG_MINS 0x01
#define REG_HRS 0x02
#define DAYOFWK_REGISTER 0x03
#define REG_DAYS 0x04
#define REG_MONTH 0x05
#define REG_YRS 0x06
#define CONTROL_REGISTER 0x07
#define RAM_BEGIN 0x08
#define RAM_END 0x3F

#define SET_SEC  	0
#define SET_MIN 	1
#define SET_HOURS   2
#define SET_DAY		3
#define SET_MONTH	4
#define SET_YEAR	5
#define GET_SEC		6
#define GET_MIN		7
#define GET_HOURS	8
#define GET_DAY		9
#define GET_MONTH	10
#define GET_YEAR	11

#define DS1307_RTC "rtc"
#define AM	0
#define PM	1

#include "lib/sensors.h"

extern const struct sensors_sensor ds1307_sensor;
void DS1307_init(void);
uint8_t bcd_to_dec(uint8_t val);
uint8_t dec_to_bcd(uint8_t val);
void set_seconds(uint8_t sec);
void set_minutes(uint8_t min);
void set_hours(uint8_t hrs);
void set_day(uint8_t day);
void set_month(uint8_t month);
void set_year(uint8_t yrs);
uint8_t get_seconds();
uint8_t get_minutes();
uint8_t get_hours();
uint8_t get_day();
uint8_t get_month();
uint16_t get_year();

#endif /* DS1307_SENSOR_H_ */
