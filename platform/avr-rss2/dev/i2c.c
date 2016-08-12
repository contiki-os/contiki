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
 */

/**
 * \file
 *         i2c core functions
 * \author
 *         Robert Olsson <robert@radio-sensors.com>
 */

#include <avr/pgmspace.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "dev/watchdog.h"
#include "contiki.h"
#include "i2c.h"
#include <compat/twi.h>
#include <stdio.h>
#include <string.h>
#include "dev/co2_sa_kxx-sensor.h"

void
i2c_init(uint32_t speed)
{
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */

  TWSR = 0;                         /* no prescaler */
  TWBR = ((F_CPU / speed) - 16) / 2;  /* must be > 10 for stable operation */
}
uint8_t
i2c_start(uint8_t addr)
{
  uint8_t twst;
  uint32_t n;

  /* Send START condition */
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

  /* Wait until transmission completed */
  for(n = 0; n < 100000 && !(TWCR & (1 << TWINT)); n++) {
  }
  if(n >= 100000) {
    return 1;
  }

  /* check value of TWI Status Register. Mask prescaler bits. */
  twst = TW_STATUS & 0xF8;
  if((twst != TW_START) && (twst != TW_REP_START)) {
    return 1;
  }

  /* send device address */
  TWDR = addr;
  TWCR = (1 << TWINT) | (1 << TWEN);

  /* wail until transmission completed and ACK/NACK has been received */
  for(n = 0; n < 100000 && !(TWCR & (1 << TWINT)); n++) {
  }
  if(n >= 100000) {
    return 1;
  }

  /* check value of TWI Status Register. Mask prescaler bits. */
  twst = TW_STATUS & 0xF8;
  if((twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK)) {
    return 1;
  }

  return 0;
}
void 
i2c_start_wait(uint8_t addr)
{
    uint8_t   twst;
    while ( 1 )
    {
            // send START condition
            TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
        // wait until transmission completed
        while(!(TWCR & (1<<TWINT)));
        // check value of TWI Status Register. Mask prescaler bits.
        twst = TW_STATUS & 0xF8;
        if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
        // send device address
        TWDR = addr;
        TWCR = (1<<TWINT) | (1<<TWEN);
        // wail until transmission completed
        while(!(TWCR & (1<<TWINT)));
        // check value of TWI Status Register. Mask prescaler bits.
        twst = TW_STATUS & 0xF8;
        if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) )
        {
            /* device busy, send stop condition to terminate write operation */
                TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
                // wait until stop condition is executed and bus released
                while(TWCR & (1<<TWSTO));
            continue;
        }
        //if( twst != TW_MT_SLA_ACK) return 1;
        break;
     }
}
void
i2c_stop(void)
{
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
  /* wait until ready */
  while(TWCR & (1<<TWSTO));
}
void
i2c_write(uint8_t u8data)
{
  TWDR = u8data;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while((TWCR & (1 << TWINT)) == 0) ;
}
uint8_t
i2c_readAck(void)
{
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
  while((TWCR & (1 << TWINT)) == 0) ;
  return TWDR;
}
uint8_t
i2c_readNak(void)
{
  TWCR = (1 << TWINT) | (1 << TWEN);
  while((TWCR & (1 << TWINT)) == 0) ;
  return TWDR;
}
static void
print_delim(int p, char *s, const char *d)
{
  if(p) {
    printf("%s", d);
  }
  printf("%s", s);
}
void
i2c_write_mem(uint8_t addr, uint8_t reg, uint8_t value)
{
  i2c_start(addr | I2C_WRITE);
  i2c_write(reg);
  i2c_write(value);
  i2c_stop();
}
void
i2c_read_mem(uint8_t addr, uint8_t reg, uint8_t buf[], uint8_t bytes)
{
  uint8_t i = 0;
  i2c_start(addr | I2C_WRITE);
  i2c_write(reg);
  i2c_start(addr | I2C_READ);
  for(i = 0; i < bytes; i++) {
    if(i == bytes - 1) {
      buf[i] = i2c_readNak();
    } else {
      buf[i] = i2c_readAck();
    }
  }
  i2c_stop();
}
void
i2c_at24mac_read(char *buf, uint8_t eui64)
{
  if(eui64) {
    i2c_read_mem(I2C_AT24MAC_ADDR, 0x98, (uint8_t *)buf, 8);
  }
  /* 128bit unique serial number */
  else {
    i2c_read_mem(I2C_AT24MAC_ADDR, 0x80, (uint8_t *)buf, 16);
  }
}

uint16_t
i2c_probe(void)
{
  int p = 0;
  const char *del = ",";
  uint16_t probed = 0;
  watchdog_periodic();
  if(!i2c_start(I2C_AT24MAC_ADDR)) {
    i2c_stop();
    probed |= I2C_AT24MAC;
    print_delim(p++, "AT24MAC", del);
  }
  watchdog_periodic();
  if(!i2c_start(I2C_SHT2X_ADDR)) {
    i2c_stop();
    probed |= I2C_SHT2X;
    print_delim(p++, "SHT2X", del);
  }
  watchdog_periodic();
  if(!i2c_start(I2C_CO2SA_ADDR)) {
    i2c_stop();
    probed |= I2C_CO2SA;
    print_delim(p++, "CO2SA", del);
  }
  return probed;
}
