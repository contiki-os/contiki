/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)

#define DEBUG 1
#if DEBUG
#define PRINTD(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTD(...)
#endif

#include "contiki.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

#if AVR_WEBSERVER
//#include "httpd-fs.h"
//#include "httpd-cgi.h"
#endif

#include "contiki-net.h"
#include "params.h"

#if WITH_NODE_ID
uint16_t node_id;
#endif

#if CONTIKI_CONF_RANDOM_MAC
extern uint8_t rng_get_uint8(void);
static void
generate_new_eui64(uint8_t eui64[8]) {
  eui64[0] = 0x02;
  eui64[1] = rng_get_uint8();
  eui64[2] = rng_get_uint8();
  eui64[3] = 0xFF;
  eui64[4] = 0xFE;
  eui64[5] = rng_get_uint8();
  eui64[6] = rng_get_uint8();
  eui64[7] = rng_get_uint8();
}
#endif

#if AVR_WEBSERVER
/* Webserver builds can set these in httpd-fsdata.c via makefsdata.h */
extern uint8_t default_mac_address[8];
extern uint8_t default_server_name[16];
extern uint8_t default_domain_name[30];
#else
const uint8_t default_mac_address[8] PROGMEM = PARAMS_EUI64ADDR;
const uint8_t default_server_name[] PROGMEM = PARAMS_SERVERNAME;
const uint8_t default_domain_name[] PROGMEM = PARAMS_DOMAINNAME;
#endif

#if PARAMETER_STORAGE==0
/* 0 Hard coded, minmal program and eeprom usage. */
uint8_t
params_get_eui64(uint8_t *eui64) {
#if CONTIKI_CONF_RANDOM_MAC
    PRINTD("Generating random EUI64 MAC\n");
    generate_new_eui64(eui64);
    return 1;
#else
    uint8_t i;
    for (i=0;i<sizeof(default_mac_address);i++) eui64[i] = pgm_read_byte_near(default_mac_address+i);
    return 0;
#endif
}
#elif PARAMETER_STORAGE==1
/* 1 Stored in fixed eeprom locations, rewritten from flash if corrupt.
 * They can be manually changed and kept over program reflash.
 * The channel and bit complement are used to check EEMEM integrity,
 * If corrupt all values will be rewritten with the default flash values.
 * To make this work, get the channel before anything else.
 */
#if !AVR_WEBSERVER
uint8_t eemem_mac_address[] EEMEM = PARAMS_EUI64ADDR;
uint8_t eemem_server_name[] EEMEM = PARAMS_SERVERNAME;
uint8_t eemem_domain_name[] EEMEM = PARAMS_DOMAINNAME;
#endif /*AVR_WEBSERVER */

uint16_t eemem_nodeid EEMEM = PARAMS_NODEID;
uint8_t eemem_channel[2] EEMEM = {PARAMS_CHANNEL, ~PARAMS_CHANNEL};
uint16_t eemem_panid EEMEM = PARAMS_PANID;
uint16_t eemem_panaddr EEMEM = PARAMS_PANADDR;
uint8_t eemem_txpower EEMEM = PARAMS_TXPOWER;

#if CONTIKI_CONF_RANDOM_MAC
static uint8_t randomeui64;
#endif

uint8_t
params_get_channel(void) {
  uint8_t x[2];
  *(uint16_t *)x = eeprom_read_word ((uint16_t *)&eemem_channel);
/* Don't return an invalid channel number */
  if( (x[0]<11) || (x[0] > 26)) x[1]=x[0];
/* Do exclusive or test on the two values read */
  if((uint8_t)x[0]!=(uint8_t)~x[1]) {//~x[1] can promote comparison to 16 bit
/* Verification fails, rewrite everything */
    uint8_t i,buffer[32];
    PRINTD("EEPROM is corrupt, rewriting with defaults.\n");
#if CONTIKI_CONF_RANDOM_MAC
    PRINTD("Generating random EUI64 MAC\n");
    generate_new_eui64(&buffer);
    randomeui64=1;
#else
    for (i=0;i<sizeof(default_mac_address);i++) buffer[i] = pgm_read_byte_near(default_mac_address+i);
#endif
/* eeprom_write_block should not be interrupted */
    cli();
    eeprom_write_block(&buffer,  &eemem_mac_address, sizeof(eemem_mac_address));
    for (i=0;i<sizeof(default_server_name);i++) buffer[i] = pgm_read_byte_near(default_server_name+i);
    eeprom_write_block(&buffer,  &eemem_server_name, sizeof(eemem_server_name));
    for (i=0;i<sizeof(default_domain_name);i++) buffer[i] = pgm_read_byte_near(default_domain_name+i);
    eeprom_write_block(&buffer,  &eemem_domain_name, sizeof(eemem_domain_name));
    eeprom_write_word(&eemem_panid  , PARAMS_PANID);
    eeprom_write_word(&eemem_panaddr, PARAMS_PANADDR);
    eeprom_write_byte(&eemem_txpower, PARAMS_TXPOWER);
    eeprom_write_word(&eemem_nodeid, PARAMS_NODEID);
    x[0] = PARAMS_CHANNEL;
    x[1]= ~x[0];
    eeprom_write_word((uint16_t *)&eemem_channel, *(uint16_t *)x);
    sei();
  }
/* Always returns a valid channel */
  return x[0];
}
uint8_t
params_get_eui64(uint8_t *eui64) {
  cli();
  eeprom_read_block ((void *)eui64, &eemem_mac_address, sizeof(linkaddr_t));
  sei();
#if CONTIKI_CONF_RANDOM_MAC
  return randomeui64;
#else
  return 0;
#endif
}
uint16_t
params_get_panid(void) {
  return eeprom_read_word(&eemem_panid);
}
uint16_t
params_get_panaddr(void) {
  return eeprom_read_word (&eemem_panaddr);
}
uint8_t
params_get_txpower(void)
{
  return eeprom_read_byte(&eemem_txpower);
}

#else /* CONTIKI_CONF_SETTINGS_MANAGER */

uint8_t
params_get_channel() {
  uint8_t x;
  size_t  size = 1;
  if (settings_get(SETTINGS_KEY_CHANNEL, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get RF channel %u\n",x);
  } else {
    x = PARAMS_CHANNEL;
    if (settings_add_uint8(SETTINGS_KEY_CHANNEL,x ) == SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM RF channel to %d\n",x);
    }
  }
  return x;
}
uint8_t
params_get_eui64(uint8_t *eui64) {
  size_t size = sizeof(linkaddr_t); 
  if(settings_get(SETTINGS_KEY_EUI64, 0, (unsigned char*)eui64, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get EUI64 MAC\n");
    return 0;		
  }
#if CONTIKI_CONF_RANDOM_MAC
  PRINTD("Generating random EUI64 MAC\n");
  generate_new_eui64(eui64);
#else
  {uint8_t i;for (i=0;i<8;i++) eui64[i] = pgm_read_byte_near(default_mac_address+i);} //test this
#endif
  if (settings_add(SETTINGS_KEY_EUI64,(unsigned char*)eui64,8) == SETTINGS_STATUS_OK) {
    PRINTD("->Set EEPROM MAC address\n");
  }
#if CONTIKI_CONF_RANDOM_MAC
  return 1;
#else
  return 0;
#endif
}
uint16_t
params_get_panid(void) {
  uint16_t x;
  size_t  size = 2;
  if (settings_get(SETTINGS_KEY_PAN_ID, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get PAN ID of %04x\n",x);
  } else {
    x=PARAMS_PANID;
    if (settings_add_uint16(SETTINGS_KEY_PAN_ID,x)==SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM PAN ID to %04x\n",x);
    }
  }
  return x;
}
uint16_t
params_get_panaddr(void) {
  uint16_t x;
  size_t  size = 2;
  if (settings_get(SETTINGS_KEY_PAN_ADDR, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get PAN address of %04x\n",x);
  } else {
    x=PARAMS_PANADDR;
    if (settings_add_uint16(SETTINGS_KEY_PAN_ADDR,x)==SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM PAN address to %04x\n",x);
    }
  }        
  return x;
}
uint8_t
params_get_txpower(void) {
  uint8_t x;
  size_t  size = 1;
  if (settings_get(SETTINGS_KEY_TXPOWER, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get tx power of %d (0=max)\n",x);
  } else {
    x=PARAMS_TXPOWER;
    if (settings_add_uint8(SETTINGS_KEY_TXPOWER,x)==SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM tx power of %d (0=max)\n",x);
    }
  }
  return x;
}
#endif /* CONTIKI_CONF_SETTINGS_MANAGER */
