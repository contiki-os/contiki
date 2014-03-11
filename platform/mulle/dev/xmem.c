/**
 * \file
 *      Xmem implementation for IRoad platform using flash driver and FLASH_ID0.
 *
 * \author
 *      Henrik Makitaavola <henrik@makitaavola.se>
 */

#include "xmem.h"
#include "flash.h"


void xmem_init(void)
{
  flash_init();
}

int xmem_pread(void *buf, int nbytes, unsigned long offset)
{
  if (flash_readi(FLASH_ID0, offset, buf, nbytes, FLASH_WAIT) == E_FLASH_OK)
  {
    return nbytes;
  }
  return 0;
}

int xmem_pwrite(const void *buf, int nbytes, unsigned long offset)
{
  if (flash_writei(FLASH_ID0, offset, (uint8_t *)buf, nbytes, FLASH_WAIT | FLASH_FINISH)
      == E_FLASH_OK)
  {
    return nbytes;
  }
  return 0;
}

int xmem_erase(long nbytes, unsigned long offset)
{
  uint16_t i;
  uint16_t first;

  // Offset must be at a start of a sector
  if (offset % FLASH_SECTOR_SIZE != 0)
  {
    return 0;
  }
  // nbytes must be a multiple of sectors
  if (nbytes % FLASH_SECTOR_SIZE != 0)
  {
    return 0;
  }
  first = offset/FLASH_SECTOR_SIZE;
  for (i = first; i < (first+nbytes/FLASH_SECTOR_SIZE); ++i)
  {
    if (flash_erase_sector(FLASH_ID0, i, FLASH_WAIT|FLASH_FINISH) != E_FLASH_OK)
    {
      // TODO(henrik) fix return code so that number of erased bytes is returned.
      return 0;
    }
  }

  return nbytes;
}
