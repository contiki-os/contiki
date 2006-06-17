#ifndef __FLASH_H__
#define __FLASH_H__

#include <inttypes.h>

void flash_write_page(uint32_t page, uint8_t *buf);

#endif /* __FLASH_H__ */
