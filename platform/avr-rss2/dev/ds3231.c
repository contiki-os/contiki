// ### BOILERPLATE ###
// Meter Clock Firmware
// Copyright (C) 2007 Peter Todd <pete@petertodd.org>
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// ### BOILERPLATE ###

#include "ds3231.h"
#include "i2c.h"

#define RTC_WADDR   0xD0
#define RTC_RADDR   0xD1


void ds3231_init()
{
  i2c_init(100000);
}
uint8_t bcd_to_dec(uint8_t val)
{
  return( (val/16*10) + (val%16) );
}
uint8_t dec_to_bcd(uint8_t val)
{
  return( (val/10*16) + (val%10) );
}


void ds3231_get_datetime(datetime_t *datetime)
{
  i2c_start(RTC_WADDR);
  i2c_write(0);
  i2c_stop();
  i2c_start(RTC_RADDR);
  datetime->secs = bcd_to_dec(i2c_readAck()); 
  datetime->mins = bcd_to_dec(i2c_readAck());
  datetime->hours= bcd_to_dec(i2c_readAck());
  datetime->wday= bcd_to_dec(i2c_readAck());
  datetime->day= bcd_to_dec(i2c_readAck());
  datetime->month= bcd_to_dec(i2c_readAck());
  datetime->year= bcd_to_dec(i2c_readNak());
  i2c_stop();
}

void ds3231_set_datetime(datetime_t *datetime)
{
  i2c_start(RTC_WADDR);
  i2c_write(0x00);
  i2c_write(dec_to_bcd(datetime->secs));
  i2c_write(dec_to_bcd(datetime->mins));
  i2c_write(dec_to_bcd(datetime->hours));
  i2c_write(dec_to_bcd(datetime->wday));
  i2c_write(dec_to_bcd(datetime->day));
  i2c_write(dec_to_bcd(datetime->month));
  i2c_write(dec_to_bcd(datetime->year));
  i2c_stop();
}

float ds3231_get_temp()
{
    uint8_t msb, lsb;
	i2c_start(RTC_WADDR);
    i2c_write(0x11);
    i2c_stop();
    i2c_start(RTC_RADDR);
	msb = i2c_readAck(); //0x11
    lsb = i2c_readNak();
	i2c_stop();   
    return (float) msb + ((lsb >> 6) & 0x03) * 0.25f;          
}

