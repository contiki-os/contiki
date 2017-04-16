#ifndef __DSPICFLASH_H_
#define __DSPICFLASH_H_ 1

#include <stdint.h>

#define PAGE_NOPACK 0
#define PAGE_PACK 1
void erase_flash_page(uint32_t wordAddr);
void write_flash_page(uint32_t wordAddr, uint8_t *buf, uint16_t len, int pack);
void read_flash_page(uint32_t wordAddr, uint8_t *buf, uint16_t len, int pack);
uint16_t do_crc(uint32_t saddr, uint16_t len);
void psv_puts(const char __psv__ *msg);
int check_AIVT(uint32_t mivt_base, uint32_t aivt_base, uint16_t aivt_len);
int reprogram_MIVT(uint32_t mivt_loc, uint32_t aivt_loc, uint16_t aivt_len);
void dump_prog_mem(uint32_t far_addr, uint16_t len);
void dump_buf(uint32_t far_addr, uint16_t len, uint8_t *mem);

#endif

