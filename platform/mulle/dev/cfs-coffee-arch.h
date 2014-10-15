/**
 * \file
 *	Coffee architecture-dependent header.
 * \author
 *  Henrik Makitaavola <henrik@makitaavola.se>
 */

#ifndef CFS_COFFEE_ARCH_H
#define CFS_COFFEE_ARCH_H

#include "contiki-conf.h"
#include "dev/xmem.h"
#include "xmem-arch.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Coffee configuration parameters. */
#define COFFEE_PAGE_SIZE    XMEM_PAGE_SIZE
#define COFFEE_SECTOR_SIZE              XMEM_SECTOR_SIZE
#define COFFEE_NUM_SECTORS              XMEM_NUM_SECTORS
#define COFFEE_START      0
#define COFFEE_SIZE     XMEM_NUM_SECTORS * XMEM_SECTOR_SIZE
#define COFFEE_NAME_LENGTH    64
#define COFFEE_MAX_OPEN_FILES 6
#define COFFEE_FD_SET_SIZE    8
/* TODO(henrik) check the next four settings. */
#define COFFEE_LOG_TABLE_LIMIT  256
#define COFFEE_DYN_SIZE     4 * 1024
#define COFFEE_LOG_SIZE     1024
#define COFFEE_MICRO_LOGS   1

/* Flash operations. */
#define COFFEE_WRITE(buf, size, offset) \
  xmem_pwrite((char *)(buf), (size), COFFEE_START + (offset))

#define COFFEE_READ(buf, size, offset) \
  xmem_pread((char *)(buf), (size), COFFEE_START + (offset))

#define COFFEE_ERASE(sector) \
  xmem_erase(COFFEE_SECTOR_SIZE, COFFEE_START + (sector) * COFFEE_SECTOR_SIZE)

/* Coffee types. */
typedef int16_t coffee_page_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !COFFEE_ARCH_H */
