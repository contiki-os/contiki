/**
 * \file
 *      Xmem settings.
 *
 * \author
 *      Henrik Makitaavola <henrik@makitaavola.se>
 */

#ifndef XMEM_ARCH_H_
#define XMEM_ARCH_H_

#include "flash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XMEM_PAGE_SIZE        FLASH_PAGE_SIZE
#define XMEM_SECTOR_SIZE      FLASH_SECTOR_SIZE
#define XMEM_NUM_SECTORS      FLASH_SECTORS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XMEM_ARCH_H_ */
