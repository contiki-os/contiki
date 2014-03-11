// Author: Simon Aittamaa

#ifndef FLASH_H_
#define FLASH_H_ 1

#include <stddef.h>
#include <stdint.h>

enum flash_id_t
{
  FLASH_ID0 = 0x4,
};

enum flash_metrics_t
{
  FLASH_PAGE_SIZE	= 256,
  FLASH_SECTOR_SIZE	= 256*FLASH_PAGE_SIZE,
  FLASH_SECTORS         = 32,
  FLASH_SIZE		= FLASH_SECTOR_SIZE*FLASH_SECTORS,
};

enum flash_flags_t
{
  FLASH_WAIT		= (1 << 0),
  FLASH_FINISH		= (1 << 1),
  FLASH_BLOCKING		= FLASH_WAIT | FLASH_FINISH,
};

enum flash_error_t
{
  E_FLASH_OK		= 0,
  E_FLASH_INVALID_STATUS,
  E_FLASH_UNKNOWN,
  E_FLASH_BUSY,
  E_FLASH_QUEUE_FULL,
  E_FLASH_QUEUE_EMPTY,
  E_FLASH_ERASE_FAILED,
  E_FLASH_WRITE_FAILED,
};

typedef uint32_t flash_addr_t;

enum flash_error_t	flash_init(void);

enum flash_error_t	flash_status(enum flash_id_t);

enum flash_error_t	flash_read(enum flash_id_t, flash_addr_t, uint8_t *, uint32_t, enum flash_flags_t);
// Read data inverted (~data)
enum flash_error_t	flash_readi(enum flash_id_t, flash_addr_t, uint8_t *, uint32_t, enum flash_flags_t);
enum flash_error_t	flash_write(enum flash_id_t, flash_addr_t, uint8_t *, uint32_t, enum flash_flags_t);
// Write data inverted (~data)
enum flash_error_t	flash_writei(enum flash_id_t, flash_addr_t, uint8_t *, uint32_t, enum flash_flags_t);

enum flash_error_t	flash_write_queue(enum flash_id_t, flash_addr_t, uint8_t *, uint32_t, enum flash_flags_t);
enum flash_error_t	flash_write_process(enum flash_flags_t);

enum flash_error_t	flash_erase_sector(enum flash_id_t, uint32_t, enum flash_flags_t);
enum flash_error_t	flash_erase_chip(enum flash_id_t, enum flash_flags_t);

enum flash_error_t	flash_sleep(enum flash_id_t, enum flash_flags_t);
enum flash_error_t	flash_wakeup(enum flash_id_t, enum flash_flags_t);

/* Indended for debugging purposes _ONLY_  -- Simon */
enum flash_error_t	flash_dump(enum flash_id_t, void (*)(uint8_t));

#endif
