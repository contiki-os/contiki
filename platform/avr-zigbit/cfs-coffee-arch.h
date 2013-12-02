/*
 * Copyright (c) 2008, Swedish Institute of Computer Science
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
 * \file Modified to Atmel 1281
 *  Coffee architecture-dependent header for the Zigbit platform.
 *  The 1281 PROGMEM has 512 pages of 256 bytes each = 128KB
 * \author
 *  Original
 *  Frederic Thepaut <frederic.thepaut@inooi.com>
 *  David Kopf <dak664@embarqmail.com>
 *  Modified by 
 *  Juan Pablo Leal Licudis jplicudis@insus.com.ar
 */

#ifndef CFS_COFFEE_ARCH_H
#define CFS_COFFEE_ARCH_H

#include "contiki-conf.h"

//Currently you may choose just one of the following for the coffee file sytem
//A static file sysstem allows file rewrites but no extensions or new files
//This allows a static linked list to index into the file system
#if COFFEE_FILES==1             //1=eeprom for static file system
#define COFFEE_AVR_EEPROM 1
#define COFFEE_STATIC     1
#elif COFFEE_FILES==2           //2=eeprom for full file system
#define COFFEE_AVR_EEPROM 1
#elif COFFEE_FILES==3           //3=program flash for static file system
#define COFFEE_AVR_FLASH  1
#define COFFEE_STATIC     1
#else                           //4=program flash with full file system
#define COFFEE_AVR_FLASH  1
#endif

#ifdef COFFEE_AVR_EEPROM
#include "dev/eeprom.h"
#if COFFEE_ADDRESS==DEFAULT       //Make can pass starting address with COFFEE_ADDRESS=0xnnnnnnnn
#undef COFFEE_ADDRESS
#ifdef CFS_EEPROM_CONF_OFFSET     //Else use the platform default
#define COFFEE_ADDRESS            CFS_EEPROM_CONF_OFFSET
#else                             //Use zero if no default defined
#define COFFEE_ADDRESS            0
#endif
#endif

typedef int16_t coffee_page_t;
typedef uint16_t coffee_offset_t;

#define COFFEE_ERASE(sector) avr_eeprom_erase(sector)
void avr_eeprom_erase(uint16_t sector);

#define COFFEE_WRITE(buf, size, offset) \
        eeprom_write(COFFEE_START + (offset), (unsigned char *)(buf), (size))

#define COFFEE_READ(buf, size, offset) \
        eeprom_read (COFFEE_START + (offset), (unsigned char *)(buf), (size))

#endif /* COFFEE_AVR_EEPROM */

#ifdef COFFEE_AVR_FLASH

/* 1281 PROGMEM has 512 pages of 256 bytes each = 128KB
 * Writing to the last 32 NRRW pages will halt the CPU.
 * Take care not to overwrite the .bootloader section...
 */
#define COFFEE_PAGE_SIZE          256UL                           //Byte per program flash page 256 for 1281
#define COFFEE_PAGE_START         65536UL/COFFEE_PAGE_SIZE        //Starting page at 64KB for webserver
#define COFFEE_PAGES              512UL-COFFEE_PAGE_START-32UL    //Number of pages to use (reserve NWWR pages)
#define COFFEE_START              (COFFEE_ADDRESS & ~(COFFEE_PAGE_SIZE-1))
#define FLASH_WORD_READS          1                               //1=Faster, but 130 bytes more PROGMEM
#define FLASH_COMPLEMENT_DATA     0                               //1=Slower reads, but no page writes after erase and 18 bytes less PROGMEM

#define COFFEE_SIZE               (COFFEE_PAGES)*COFFEE_PAGE_SIZE //Bytes in filesystem
#define COFFEE_SECTOR_SIZE        (COFFEE_PAGE_SIZE*1)            //Each page a sector
#define COFFEE_NAME_LENGTH        16                              //processes.shtml is longest file name
#define COFFEE_MAX_OPEN_FILES     4
#define COFFEE_FD_SET_SIZE        8                                //Size of file descriptor
#define COFFEE_LOG_TABLE_LIMIT    16
#define COFFEE_DIR_CACHE_ENTRIES  1
#define COFFEE_DYN_SIZE           (COFFEE_PAGE_SIZE*1)             //Allocation block size
#define COFFEE_MICRO_LOGS         0                                //1 to enable, 0 best for single page sectors
#define COFFEE_LOG_SIZE           128                              //Microlog size, when used

/* coffee_page_t is used for page and sector numbering
 * uint8_t can handle 511 pages.
 * cfs_offset_t is used for full byte addresses
 * If CFS_CONF_OFFSET_TYPE is not defined it defaults to int.
 * uint16_t can handle up to a 65535 byte file system.
 */
#define coffee_page_t uint8_t
#define CFS_CONF_OFFSET_TYPE uint16_t


#define COFFEE_WRITE(buf, size, offset) \
        avr_flash_write(offset, (uint8_t *) buf, size)

#define COFFEE_READ(buf, size, offset) \
        avr_flash_read(offset, (uint8_t *) buf, size)

#define COFFEE_ERASE(sector) avr_flash_erase(sector)

void avr_flash_erase(coffee_page_t sector);
void avr_flash_read (CFS_CONF_OFFSET_TYPE addr, uint8_t *buf, CFS_CONF_OFFSET_TYPE size);
void avr_flash_write(CFS_CONF_OFFSET_TYPE addr, uint8_t *buf, CFS_CONF_OFFSET_TYPE size);

#define avr_httpd_fs_cpy(dest,addr,size) avr_flash_read((CFS_CONF_OFFSET_TYPE) addr, (uint8_t *)dest, (CFS_CONF_OFFSET_TYPE) size)
char    avr_httpd_fs_getchar(char *addr);
char *  avr_httpd_fs_strchr (char *ram, int character);
int     avr_httpd_fs_strcmp (char *addr,char *ram);


#endif /* COFFEE_AVR_FLASH */

#endif /* !COFFEE_ARCH_H */
