/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 *  Coffee architecture-dependent functionality for the AVR-Raven 1284p platform.
 * \author
 *  Nicolas Tsiftes <nvt@sics.se>
 *  Frederic Thepaut <frederic.thepaut@inooi.com>
 *  David Kopf <dak664@embarqmail.com>
 */

#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "cfs-coffee-arch.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#endif

#define TESTCOFFEE 1
#define DEBUG_CFS 1
#if TESTCOFFEE
#if DEBUG_CFS
#include <stdio.h>
#define PRINTF_CFS(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF_CFS(...)
#endif

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/crc16.h"
#include "lib/random.h"
#include <stdio.h>

#define FAIL(x) error = (x); goto end;

#define FILE_SIZE 512

int
coffee_file_test(void)
{
  int error;
  int wfd, rfd, afd;
  unsigned char buf[256], buf2[11];
  int r, i, j, total_read;
  unsigned offset;

  cfs_remove("T1");
  cfs_remove("T2");
  cfs_remove("T3");
  cfs_remove("T4");
  cfs_remove("T5");

  wfd = rfd = afd = -1;

  for(r = 0; r < sizeof(buf); r++) {
    buf[r] = r;
  }

  /* Test 1: Open for writing. */
  wfd = cfs_open("T1", CFS_WRITE);
  if(wfd < 0) {
    FAIL(-1);
  }

  /* Test 2: Write buffer. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-2);
  } else if(r < sizeof(buf)) {
    FAIL(-3);
  }

  /* Test 3: Deny reading. */
  r = cfs_read(wfd, buf, sizeof(buf));
  if(r >= 0) {
    FAIL(-4);
  }

  /* Test 4: Open for reading. */
  rfd = cfs_open("T1", CFS_READ);
  if(rfd < 0) {
    FAIL(-5);
  }

  /* Test 5: Write to read-only file. */
  r = cfs_write(rfd, buf, sizeof(buf));
  if(r >= 0) {
    FAIL(-6);
  }

  /* Test 7: Read the buffer written in Test 2. */
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-8);
  } else if(r < sizeof(buf)) {
    PRINTF_CFS("r=%d\n", r);
    FAIL(-9);
  }

  /* Test 8: Verify that the buffer is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      PRINTF_CFS("r=%d. buf[r]=%d\n", r, buf[r]);
      FAIL(-10);
    }
  }

  /* Test 9: Seek to beginning. */
  if(cfs_seek(wfd, 0, CFS_SEEK_SET) != 0) {
    FAIL(-11);
  }

  /* Test 10: Write to the log. */
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-12);
  } else if(r < sizeof(buf)) {
    FAIL(-13);
  }

  /* Test 11: Read the data from the log. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-14);
  } else if(r < sizeof(buf)) {
    FAIL(-15);
  }

  /* Test 12: Verify that the data is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != r) {
      FAIL(-16);
    }
  }

  /* Test 13: Write a reversed buffer to the file. */
  for(r = 0; r < sizeof(buf); r++) {
    buf[r] = sizeof(buf) - r - 1;
  }
  if(cfs_seek(wfd, 0, CFS_SEEK_SET) != 0) {
    FAIL(-17);
  }
  r = cfs_write(wfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-18);
  } else if(r < sizeof(buf)) {
    FAIL(-19);
  }
  if(cfs_seek(rfd, 0, CFS_SEEK_SET) != 0) {
    FAIL(-20);
  }

  /* Test 14: Read the reversed buffer. */
  cfs_seek(rfd, 0, CFS_SEEK_SET);
  memset(buf, 0, sizeof(buf));
  r = cfs_read(rfd, buf, sizeof(buf));
  if(r < 0) {
    FAIL(-21);
  } else if(r < sizeof(buf)) {
    PRINTF_CFS("r = %d\n", r);
    FAIL(-22);
  }

  /* Test 15: Verify that the data is correct. */
  for(r = 0; r < sizeof(buf); r++) {
    if(buf[r] != sizeof(buf) - r - 1) {
      FAIL(-23);
    }
  }

  cfs_close(rfd);
  cfs_close(wfd);

  if(cfs_coffee_reserve("T2", FILE_SIZE) < 0) {
    FAIL(-24);
  }

  /* Test 16: Test multiple writes at random offset. */
  for(r = 0; r < 100; r++) {
    wfd = cfs_open("T2", CFS_WRITE | CFS_READ);
    if(wfd < 0) {
      FAIL(-25);
    }

    offset = random_rand() % FILE_SIZE;

    for(r = 0; r < sizeof(buf); r++) {
      buf[r] = r;
    }

    if(cfs_seek(wfd, offset, CFS_SEEK_SET) != offset) {
      FAIL(-26);
    }

    if(cfs_write(wfd, buf, sizeof(buf)) != sizeof(buf)) {
      FAIL(-27);
    }

    if(cfs_seek(wfd, offset, CFS_SEEK_SET) != offset) {
      FAIL(-28);
    }

    memset(buf, 0, sizeof(buf));
    if(cfs_read(wfd, buf, sizeof(buf)) != sizeof(buf)) {
      FAIL(-29);
    }

    for(i = 0; i < sizeof(buf); i++) {
      if(buf[i] != i) {
        PRINTF_CFS("buf[%d] != %d\n", i, buf[i]);
        FAIL(-30);
      }
    }
  }
  /* Test 17: Append data to the same file many times. */
#define APPEND_BYTES 3000
#define BULK_SIZE 10
  for (i = 0; i < APPEND_BYTES; i += BULK_SIZE) {
		afd = cfs_open("T3", CFS_WRITE | CFS_APPEND);
		if (afd < 0) {
			FAIL(-31);
		}
		for (j = 0; j < BULK_SIZE; j++) {
			buf[j] = 1 + ((i + j) & 0x7f);
		}
		if ((r = cfs_write(afd, buf, BULK_SIZE)) != BULK_SIZE) {
			PRINTF_CFS("Count:%d, r=%d\n", i, r);
			FAIL(-32);
		}
		cfs_close(afd);
	}

  /* Test 18: Read back the data written in Test 17 and verify that it
     is correct. */
  afd = cfs_open("T3", CFS_READ);
  if(afd < 0) {
    FAIL(-33);
  }
  total_read = 0;
  while((r = cfs_read(afd, buf2, sizeof(buf2))) > 0) {
    for(j = 0; j < r; j++) {
      if(buf2[j] != 1 + ((total_read + j) & 0x7f)) {
  FAIL(-34);
      }
    }
    total_read += r;
  }
  if(r < 0) {
	  PRINTF_CFS("FAIL:-35 r=%d\n",r);
    FAIL(-35);
  }
  if(total_read != APPEND_BYTES) {
	  PRINTF_CFS("FAIL:-35 total_read=%d\n",total_read);
    FAIL(-35);
  }
  cfs_close(afd);

/***************T4********************/
/* file T4 and T5 writing forces to use garbage collector in greedy mode
 * this test is designed for 10kb of file system
 * */
#define APPEND_BYTES_1 2000
#define BULK_SIZE_1 10
  for (i = 0; i < APPEND_BYTES_1; i += BULK_SIZE_1) {
		afd = cfs_open("T4", CFS_WRITE | CFS_APPEND);
		if (afd < 0) {
			FAIL(-36);
		}
		for (j = 0; j < BULK_SIZE_1; j++) {
			buf[j] = 1 + ((i + j) & 0x7f);
		}


		if ((r = cfs_write(afd, buf, BULK_SIZE_1)) != BULK_SIZE_1) {
			PRINTF_CFS("Count:%d, r=%d\n", i, r);
			FAIL(-37);
		}
		cfs_close(afd);
	}

  afd = cfs_open("T4", CFS_READ);
  if(afd < 0) {
    FAIL(-38);
  }
  total_read = 0;
  while((r = cfs_read(afd, buf2, sizeof(buf2))) > 0) {
    for(j = 0; j < r; j++) {
      if(buf2[j] != 1 + ((total_read + j) & 0x7f)) {
    	  PRINTF_CFS("FAIL:-39, total_read=%d r=%d\n",total_read,r);
  FAIL(-39);
      }
    }
    total_read += r;
  }
  if(r < 0) {
	  PRINTF_CFS("FAIL:-40 r=%d\n",r);
    FAIL(-40);
  }
  if(total_read != APPEND_BYTES_1) {
	  PRINTF_CFS("FAIL:-41 total_read=%d\n",total_read);
    FAIL(-41);
  }
  cfs_close(afd);
  /***************T5********************/
#define APPEND_BYTES_2 1000
#define BULK_SIZE_2 10
    for (i = 0; i < APPEND_BYTES_2; i += BULK_SIZE_2) {
  		afd = cfs_open("T5", CFS_WRITE | CFS_APPEND);
  		if (afd < 0) {
  			FAIL(-42);
  		}
  		for (j = 0; j < BULK_SIZE_2; j++) {
  			buf[j] = 1 + ((i + j) & 0x7f);
  		}

  		if ((r = cfs_write(afd, buf, BULK_SIZE_2)) != BULK_SIZE_2) {
  			PRINTF_CFS("Count:%d, r=%d\n", i, r);
  			FAIL(-43);
  		}

  		cfs_close(afd);
  	}

    afd = cfs_open("T5", CFS_READ);
    if(afd < 0) {
      FAIL(-44);
    }
    total_read = 0;
    while((r = cfs_read(afd, buf2, sizeof(buf2))) > 0) {
      for(j = 0; j < r; j++) {
        if(buf2[j] != 1 + ((total_read + j) & 0x7f)) {
      	  PRINTF_CFS("FAIL:-45, total_read=%d r=%d\n",total_read,r);
    FAIL(-45);
        }
      }
      total_read += r;
    }
    if(r < 0) {
  	  PRINTF_CFS("FAIL:-46 r=%d\n",r);
      FAIL(-46);
    }
    if(total_read != APPEND_BYTES_2) {
  	  PRINTF_CFS("FAIL:-47 total_read=%d\n",total_read);
      FAIL(-47);
    }
    cfs_close(afd);

  error = 0;
end:
  cfs_close(wfd); cfs_close(rfd); cfs_close(afd);
  return error;
}
#endif /* TESTCOFFEE */

/*---------------------------------------------------------------------------*/
/*---------------------------EEPROM ROUTINES---------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef COFFEE_AVR_EEPROM

/* Letting .bss initialize nullb to zero saves COFFEE_SECTOR_SIZE of flash */
//static const unsigned char nullb[COFFEE_SECTOR_SIZE] = {0};
static const unsigned char nullb[COFFEE_SECTOR_SIZE];

/*---------------------------------------------------------------------------*/
/* Erase EEPROM sector
 */
void
avr_eeprom_erase(uint16_t sector)
{
  eeprom_write(COFFEE_START + sector * COFFEE_SECTOR_SIZE,
                 (unsigned char *)nullb, sizeof(nullb));
}
#endif /* COFFEE_AVR_EEPROM */

#ifdef COFFEE_AVR_FLASH
/*---------------------------------------------------------------------------*/
/*---------------------------FLASH ROUTINES----------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * Read from flash info buf. addr contains starting flash byte address
 */
void
avr_flash_read(CFS_CONF_OFFSET_TYPE addr, uint8_t *buf, CFS_CONF_OFFSET_TYPE size)
{
  uint32_t addr32=COFFEE_START+addr;
  uint16_t isize=size;
#if DEBUG
  unsigned char *bufo=(unsigned char *)buf;
  uint8_t i;
  uint16_t w=addr32>>1;   //Show progmem word address for debug
  PRINTF("r0x%04x(%u) ",w,size);
#endif
#ifndef FLASH_WORD_READS
  for (;isize>0;isize--) {
#if FLASH_COMPLEMENT_DATA
    *buf++=~(uint8_t)pgm_read_byte_far(addr32++);
#else
    *buf++=(uint8_t)pgm_read_byte_far(addr32++);
#endif /*FLASH_COMPLEMENT_DATA*/
  }
#else
/* 130 bytes more PROGMEM, but faster */
  if (isize&0x01) {       //handle first odd byte
#if FLASH_COMPLEMENT_DATA
    *buf++=~(uint8_t)pgm_read_byte_far(addr32++);
#else
    *buf++=(uint8_t)pgm_read_byte_far(addr32++);
#endif /*FLASH_COMPLEMENT_DATA*/
     isize--;
  }
  for (;isize>1;isize-=2) {//read words from flash
#if FLASH_COMPLEMENT_DATA
   *(uint16_t *)buf=~(uint16_t)pgm_read_word_far(addr32);
#else
   *(uint16_t *)buf=(uint16_t)pgm_read_word_far(addr32);
#endif /*FLASH_COMPLEMENT_DATA*/
    buf+=2;
    addr32+=2;
  }
  if (isize) {            //handle last odd byte
#if FLASH_COMPLEMENT_DATA
    *buf++=~(uint8_t)pgm_read_byte_far(addr32);
#else
    *buf++=(uint8_t)pgm_read_byte_far(addr32);
#endif /*FLASH_COMPLEMENT_DATA*/
  }
#endif /* FLASH_WORD_READS */

#if DEBUG>1
  PRINTF("\nbuf=");
//  PRINTF("%s",bufo);
// for (i=0;i<16;i++) PRINTF("%2x ",*bufo++);
#endif
}
/*---------------------------------------------------------------------------*/
/*
 Erase the flash page(s) corresponding to the coffee sector.
 This is done by calling the write routine with a null buffer and any address
 within each page of the sector (we choose the first byte).
 */
BOOTLOADER_SECTION
void avr_flash_erase(coffee_page_t sector) {
	coffee_page_t i;

#if FLASH_COMPLEMENT_DATA
	uint32_t addr32;
	volatile uint8_t sreg;

	// Disable interrupts.
	sreg = SREG;
	cli();

	for (i = 0; i < COFFEE_SECTOR_SIZE / COFFEE_PAGE_SIZE; i++) {
		for (addr32 = COFFEE_START + (((sector + i) * COFFEE_PAGE_SIZE)
				& ~(COFFEE_PAGE_SIZE - 1)); addr32 < (COFFEE_START + (((sector
				+ i + 1) * COFFEE_PAGE_SIZE) & ~(COFFEE_PAGE_SIZE - 1))); addr32
				+= SPM_PAGESIZE) {
			boot_page_erase(addr32);
			boot_spm_busy_wait();

		}
	}
	//RE-enable interrupts
	boot_rww_enable();
	SREG = sreg;
#else
	for (i=0;i<COFFEE_SECTOR_SIZE/COFFEE_PAGE_SIZE;i++) {
		avr_flash_write((sector+i)*COFFEE_PAGE_SIZE,0,0);
	}
#endif

#if 0
#if TESTCOFFEE
/* Defining TESTCOFFEE is a convenient way of testing a new configuration.
 * It is triggered by an erase of the last sector.
 * Note this routine will be reentered during the test!                     */

  if ((sector+i)==COFFEE_PAGES-1) {
    int j=(int)(COFFEE_START>>1),k=(int)((COFFEE_START>>1)+(COFFEE_SIZE>>1)),l=(int)(COFFEE_SIZE/1024UL);
    printf_P(PSTR("\nTesting coffee filesystem [0x%08x -> 0x%08x (%uKb)] ..."),j,k,l);
    int r= coffee_file_test();
    if (r<0) {
      printf_P(PSTR("\nFailed with return %d! :-(\n"),r);
    } else {
      printf_P(PSTR("Passed! :-)\n"));
    }
  }
#endif /* TESTCOFFEE */
#endif
}

/*httpd-fs routines
  getchar is straigtforward.
  strcmp only needs to handle file names for fs_open. Note filename in buf will not be zero terminated
    if it fills the coffee name field, so a pseudo strcmp is done here.
  strchr searches for script starts so must handle arbitrarily large strings
 */
char avr_httpd_fs_getchar(char *addr) {
  char r;
  avr_flash_read((CFS_CONF_OFFSET_TYPE) addr, (uint8_t*) &r, 1);
  return r;
}
int avr_httpd_fs_strcmp (char *ram, char *addr) {
  uint8_t i,*in,buf[32];
  avr_flash_read((CFS_CONF_OFFSET_TYPE)addr, buf, sizeof(buf));
//return strcmp(ram, (char *)buf);
  in=(uint8_t *)ram;
  for (i=0;i<32;i++) {
    if (buf[i]==0) return(0);
    if (buf[i]!=*in) break;
    in++;
  }
/* A proper strcmp would return a + or minus number based on the last comparison*/
//if (buf[i]>*in) return(i); else return(-i);
  return(i);
}
char * avr_httpd_fs_strchr (char *addr, int character) {
  char buf[129],*pptr;
  buf[128]=character;
  while (1) {
    avr_flash_read((CFS_CONF_OFFSET_TYPE)addr, (uint8_t *) buf, 128);
    pptr=strchr(buf, character);
    if (pptr!=&buf[128]) {
      if (pptr==0) return 0;
      return (addr+(pptr-buf));
   }
    addr+=128;
  }

}

/*---------------------------------------------------------------------------*/
/*
 * Transfer buf[size] from RAM to flash, starting at addr.
 * If buf is null, just erase the flash page
 * Note this routine has to be in the bootloader NRWW part of program memory,
 * and that writing to NRWW (last 32 pages on the 1284p) will halt the CPU.
 */
BOOTLOADER_SECTION
void
avr_flash_write(CFS_CONF_OFFSET_TYPE addr, uint8_t *buf, CFS_CONF_OFFSET_TYPE size)
{
  uint32_t addr32;
  uint16_t w;
  uint8_t  bb,ba,sreg;
 
  /* Disable interrupts, make sure no eeprom write in progress */
  sreg = SREG;
  cli();
  eeprom_busy_wait();

  /* Calculate the starting address of the first flash page being
    modified (will be on a page boundary) and the number of
    unaltered bytes before and after the data to be written.          */
#if 0    //this is 8 bytes longer
  uint16_t startpage=addr/COFFEE_PAGE_SIZE;
  addr32=COFFEE_START+startpage*COFFEE_PAGE_SIZE;
#else
  addr32=(COFFEE_ADDRESS&~(SPM_PAGESIZE-1))+(addr&~(SPM_PAGESIZE-1));
#endif
  bb=addr & (SPM_PAGESIZE-1);
  ba=COFFEE_PAGE_SIZE-((addr+size)&0xff);

#if DEBUG
  uint16_t startpage=addr/COFFEE_PAGE_SIZE;
  w=addr32>>1;   //Show progmem word address for debug
  if (buf) {
    PRINTF("w0x%04x %u %u %u",w,size,bb,ba);
  } else {
    PRINTF("e0x%04x %u ",w,startpage);
  }
#endif

  /* If buf not null, modify the page(s) */
  if (buf) {
    if (size==0) return;            //nothing to write
    /*Copy the first part of the existing page into the write buffer */
    while (bb>1) {
      w=pgm_read_word_far(addr32);
      boot_page_fill(addr32,w);
      addr32+=2;
      bb-=2;
    }
    /* Transfer the bytes to be modified */
    while (size>1) {
      if (bb) {                     //handle odd byte boundary
        w=pgm_read_word_far(addr32);
#if FLASH_COMPLEMENT_DATA
        w  = ~w;
#endif /*FLASH_COMPLEMENT_DATA*/
        w &= 0xff;
        bb=0;
        size++;
      } else {
        w  = *buf++;
      }
      w += (*buf++) << 8;
#if FLASH_COMPLEMENT_DATA
      w  = ~w;
#endif /*FLASH_COMPLEMENT_DATA*/
      boot_page_fill(addr32, w);
      size-=2;
/* Below ought to work but writing to 0xnnnnnnfe modifies the NEXT flash page
   for some reason, at least in the AVR Studio simulator.
      if ((addr32&0x000000ff)==0x000000fe) { //handle page boundary
        if (size) {
          boot_page_erase(addr32);
          boot_spm_busy_wait();
          boot_page_write(addr32);
          boot_spm_busy_wait();
        }
      }
       addr32+=2;
*/
       
/* This works...*/
      addr32+=2;
      if ((addr32&0x000000ff)==0) {    //handle page boundary
        if (size) {
          addr32-=0x42;                //get an address within the page
          boot_page_erase(addr32);
          boot_spm_busy_wait();
          boot_page_write(addr32);
          boot_spm_busy_wait();
          addr32+=0x42;
        }
      }
    }
    /* Copy the remainder of the existing page */
    while (ba>1) {
      w=pgm_read_word_far(addr32);
      if (size) {                     //handle odd byte boundary
        w &= 0xff00;
#if FLASH_COMPLEMENT_DATA
        w +=~(*buf);
#else
        w +=*buf;
#endif /*FLASH_COMPLEMENT_DATA*/
        size=0;
      }
      boot_page_fill(addr32,w);
      addr32+=2;
      ba-=2;
    }
  /* If buf is null, erase the page to zero */
  } else {
#if FLASH_COMPLEMENT_DATA
    addr32+=2*SPM_PAGESIZE;
#else
    for (w=0;w<SPM_PAGESIZE;w++) {
      boot_page_fill(addr32, 0);
      addr32+=2;
    }
#endif /*FLASH_COMPLEMENT_DATA*/
  }
/* Write the last (or only) page */
  addr32-=0x42; //get an address within the page
  boot_page_erase(addr32);
  boot_spm_busy_wait();
#if FLASH_COMPLEMENT_DATA
  if (buf) {                      //don't write zeroes to erased page
    boot_page_write(addr32);
    boot_spm_busy_wait();
  }
#else
  boot_page_write(addr32);
  boot_spm_busy_wait();
#endif /*FLASH_COMPLEMENT_DATA*/
  /* Reenable RWW-section again. We need this if we want to jump back
   * to the application after bootloading. */
  boot_rww_enable();

  /* Re-enable interrupts (if they were ever enabled). */
  SREG = sreg;
}

#endif /* COFFEE_AVR_FLASH */
