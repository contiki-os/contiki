#include <p33Fxxxx.h>

#include <stdlib.h>
#include <stdio.h>

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
void
check_AIVT(uint32_t mivt_base, uint32_t aivt_base, uint16_t aivt_len)
{
  if(do_crc(aivt_base << 1, aivt_len << 1) != do_crc(mivt_base << 1, aivt_len << 1)) {
    psv_puts("CRCs differ, installing new AIVT");
    reprogram_MIVT(mivt_base, aivt_base, aivt_len);
  } else {
    psv_puts("CRCs same, not installing");
  }
}
/* Note - word addressing for this pair */
#define ERASEPAGESZ 0x0400
#define WRITEPAGESZ 0x0080

/* Note - all of these are using word based addressing */
void
reprogram_MIVT(uint32_t mivt_base, uint32_t aivt_base, uint16_t aivt_len)
{
  psv_puts("reprogram_mivt");
  uint32_t erasePage = aivt_base & ~(ERASEPAGESZ - 1);
  uint8_t *mem = (uint8_t *)malloc(0x600);
  if(!mem) {
    psv_puts("no memory");
    return;
  }

  /* Read in the entire base page before we modify it */
  read_flash_page(erasePage, mem, ERASEPAGESZ, PAGE_PACK);
  /* dump_buf(saddr, 0x400, mem); */

  read_flash_page(mivt_base, aivt_base - erasePage + mem, aivt_len, PAGE_PACK);
  /* dump_buf(saddr, 0x400, mem); */

  uint16_t osr = disable_int();
  erase_flash_page(erasePage);
  write_flash_page(erasePage, mem, ERASEPAGESZ, PAGE_PACK);
  enable_int(osr);

  /* dump_prog_mem(saddr, 0x400); */
  free(mem);
}
static char fmt3[] = "%08lx %04x %04x\n";

void
dump_prog_mem(uint32_t far_addr, uint16_t len)
{
  uint16_t addr;
  uint16_t l, h;
  while(len) {
    TBLPAG = far_addr >> 16;
    addr = far_addr & 0xffff;
    l = __builtin_tblrdl(addr);
    h = __builtin_tblrdh(addr);
    printf(fmt3, far_addr, h, l);
    len -= 2;
    far_addr += 2;
  }
}
static char fmt_dump_buf[] = "%08lx %04x %04x\n";

void
dump_buf(uint32_t far_addr, uint16_t len, uint8_t *mem)
{
  uint16_t vl, vh;
  while(len) {
    vl = *mem++;
    vl |= (uint16_t)*mem++ << 8;
    vh = *mem++;
    printf(fmt_dump_buf, far_addr, vh, vl);
    far_addr += 2;
    len -= 2;
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
