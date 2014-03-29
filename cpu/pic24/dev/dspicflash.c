#include <p33Fxxxx.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rs232.h"
#include "crc16.h"
#include "dspicflash.h"
#include "intsafe.h"

void
erase_flash_page(uint32_t wordAddr)
{
  uint16_t osr;
  uint16_t loc;

  osr = disable_int();
  NVMCON = 0x4042; /* Erase Page */
  TBLPAG = (wordAddr >> 16) & 0xff;
  loc = wordAddr & 0xffff;
  __builtin_tblwtl(loc, loc);
  __builtin_write_NVM();

  enable_int(osr);
}
void
write_flash_page(uint32_t wordAddr, uint8_t *buf, uint16_t len, int pack)
{
  uint16_t osr;
  uint16_t loc;
  uint16_t v;

  /*  dump_buf(wordAddr, len<<1, buf); */

  osr = disable_int();
  NVMCON = 0x4001; /* Single row program */
  TBLPAG = (wordAddr >> 16) & 0xff;
  loc = wordAddr & 0xffff;
  /* Outer loop handles entire write, inner loop handles a single write page */
  while(len) {
    uint16_t pgsz = 0x0040; /* Num of instr (24bits) per write page, not uint16_s */
    while(len && pgsz--) {
      v = *buf++;
      v |= *buf++ << 8;
      __builtin_tblwtl(loc, v);
      v = *buf++;
      v |= 0xff00; /* Program upper octet with 0xff per Microchip dox */
      __builtin_tblwth(loc, v);

      if(!pack) {
        ++buf;          /* No data here */
      }
      len -= 2;
      loc += 2;
    }

    /* Now do the program op */
    __builtin_write_NVM();
  }

  enable_int(osr);
}
/*
   If pack != 0 then don't waste the 4th octet with flash contents because
   it's not implemented on pic24/dspic
 */
void
read_flash_page(uint32_t wordAddr, uint8_t *buf, uint16_t len, int pack)
{
  TBLPAG = (wordAddr >> 16) & 0xff;
  uint16_t loc = wordAddr & 0xffff;
  uint16_t limit = loc + len;
  while(loc < limit) {
    uint16_t v = __builtin_tblrdl(loc);
    *buf++ = v & 0xff;
    *buf++ = (v >> 8) & 0xff;
    v = __builtin_tblrdh(loc);
    *buf++ = v & 0xff;
    if(!pack) {
      *buf++ = 0xff;          /* Not actually any data there */
    }
    loc += 2;
  }
}
/* Note - all of these are using word based addressing */
int
check_AIVT(uint32_t mivt_base, uint32_t aivt_base, uint16_t aivt_len)
{
  psv_puts("Checking AIVT");
  if(do_crc(aivt_base, aivt_len) != do_crc(mivt_base, aivt_len)) {
    return reprogram_MIVT(mivt_base, aivt_base, aivt_len);
  }
  psv_puts("CRCs same, not installing");

  return 0;
}
/* Note - word addressing for this pair */
#define ERASEPAGESZ 0x0400
#define WRITEPAGESZ 0x0080

/* Note - all of these are using word based addressing */
int
reprogram_MIVT(uint32_t mivt_base, uint32_t aivt_base, uint16_t aivt_len)
{
  uint32_t erasePage = aivt_base & ~(ERASEPAGESZ - 1);
  uint8_t *mem = (uint8_t *)malloc(0x600);
  if(!mem) {
    psv_puts("no memory");
    return -1;
  }

  psv_puts("Reprogramming AIVT");
  memset(mem, 0xff, 0x600);

  /* Read in the entire base page before we modify it */
  read_flash_page(erasePage, mem, ERASEPAGESZ, PAGE_PACK);

  /* odd shift is because this is a packed struct 4 bytes flash -> 3 bytes in RAM */
  read_flash_page(mivt_base, ((aivt_base - erasePage) >> 1) * 3 + mem, aivt_len, PAGE_PACK);

  uint16_t osr = disable_int();
  erase_flash_page(erasePage);
  write_flash_page(erasePage, mem, ERASEPAGESZ, PAGE_PACK);
  enable_int(osr);

  free(mem);
  return 0;
}
static const char addr_fmt[] = "%08lx : ";
static const char word_fmt[] = "%04x ";

void
dump_prog_mem(uint32_t far_addr, uint16_t len)
{
  uint16_t addr;
  uint16_t l, h;
  int cnt = 8;
  while(len) {
    TBLPAG = far_addr >> 16;
    addr = far_addr & 0xffff;
    l = __builtin_tblrdl(addr);
    h = __builtin_tblrdh(addr);
    if(cnt == 8) {
      printf(addr_fmt, far_addr);
    }
    printf(word_fmt, l);
    printf(word_fmt, h);
    cnt -= 2;
    if(cnt <= 0) {
      putchar('\n');
      cnt = 8;
    }
    len -= 2;
    far_addr += 2;
  }
}
/*
   far_addr is a word based location - printing only
   len is number of bytes
   mem is byte location
 */
void
dump_buf(uint32_t far_addr, uint16_t len, uint8_t *mem)
{
  uint16_t v;
  int cnt = 8;
  while(len) {
    if(cnt == 8) {
      printf(addr_fmt, far_addr);
    }
    v = *mem++;
    v |= (uint16_t)*mem++ << 8;
    printf(word_fmt, v);
    far_addr++;
    len -= 2;
    if(!--cnt) {
      putchar('\n');
      cnt = 8;
    }
  }
}
uint16_t
do_crc(uint32_t saddr, uint16_t len)
{
  uint16_t crc = 0;
  uint32_t addr;
  uint16_t x;

  while(len -= 2) {
    TBLPAG = saddr >> 16;
    addr = saddr & 0xffff;
    x = __builtin_tblrdl(addr);
    crc = crc16_add(x & 0xff, crc);
    crc = crc16_add(x >> 8, crc);
    x = __builtin_tblrdh(addr);
    crc = crc16_add(x & 0xff, crc);
    saddr += 2;
  }
  return crc;
}
void
psv_puts(const char __psv__ *msg)
{
  uint8_t opsv;
  char *m;
  opsv = PSVPAG;
  asm ("	mov	  %[msg], %[m]\n"
       "	btsts.c   %[m], #0xf\n"
       "	rlc.w     %d[msg], %d[msg]\n"
       "	mov       %d[msg], PSVPAG"
       :[m] "=&r" (m)
       :[msg] "r" (msg)
       );

  puts(m);

  PSVPAG = opsv;
}
