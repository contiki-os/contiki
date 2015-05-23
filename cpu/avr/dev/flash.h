#ifndef FLASH_H_
#define FLASH_H_

#include <inttypes.h>

void flash_write_page(uint32_t page, uint8_t *buf);

#endif /* FLASH_H_ */
