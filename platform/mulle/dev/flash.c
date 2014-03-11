// Author: Simon Aittamaa

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "K60.h"

#include "flash.h"

#include "udelay.h"

//TODO(henrik) Clean up

/* ************************************************************************** */
//#define FLASH_DEBUG
#ifdef FLASH_DEBUG
#include <stdio.h>
#	define DEBUG(...)	printf(__VA_ARGS__)
#else
#	define DEBUG(...)	(void)(0);
#endif

/* ************************************************************************** */

// Spi functions
enum spi_transfer_flag_t
{
  SPI_TRANSFER_DONE	= 0,
  SPI_TRANSFER_CONT	= 1
};

void spi_init(void)
{
//  // Setup PORTC
//  SIM_SCGC5  |= SIM_SCGC5_PORTC_MASK;
//
//  PORTC_PCR5 |= 0x0200; // 5 CLK
//  PORTC_PCR6 |= 0x0200; // 6 MOSI
//  PORTC_PCR7 |= 0x0200; // 7 MISO
//
//  PORTC_PCR1 |= 0x0200; // Flash CS 1
//  PORTC_PCR2 |= 0x0200; // Flash CS 2
//  PORTC_PCR3 |= 0x0200; // Flash CS 3
//  PORTC_PCR4 |= 0x0200; // Flash CS 4
//
//  // Setup SPI0
//  SIM_SCGC6 |= SIM_SCGC6_DSPI0_MASK;
//  // TODO(henrik) Check clock speeds
//  SPI0_CTAR0 = 0xb8000005;
//  SPI0_MCR   = 0x803F3000;
  SIM_SCGC5  |= SIM_SCGC5_PORTD_MASK;
  PORTD_PCR5 |= 0x0200; /* SPI0_PCS2 */
  PORTD_PCR2 |= 0x0200; /* SPI0_MOSI */
  PORTD_PCR1 |= 0x0200; /* SPI0_SCLK */
  PORTD_PCR3 |= 0x0200; /* SPI0_MISO */

  /* Enable clock gate for SPI0 module */
  SIM_SCGC6 |= SIM_SCGC6_DSPI0_MASK;

  /* Configure SPI1 */
  SPI0_MCR   = 0x803F3000;
  SPI0_CTAR0 = 0x38002224; /* TODO: Should be able to speed up */
}

void spi_write(uint32_t cs, enum spi_transfer_flag_t cont, uint32_t data)
{
  uint32_t send = SPI_PUSHR_PCS(cs) | SPI_PUSHR_TXDATA(data);
  if (cont) {
    send |= SPI_PUSHR_CONT_MASK;
  }
  SPI0_PUSHR = send;
}

uint32_t spi_read(void)
{
  uint32_t result;
  while (!(SPI0_SR & SPI_SR_TCF_MASK));
  SPI0_SR |= SPI_SR_TCF_MASK;
  result = SPI0_POPR;
  return result & 0xffff;
}

/* ************************************************************************** */
enum flash_cmd_t
{
  FLASH_CMD_WREN		= 0x06,
  FLASH_CMD_WRDI		= 0x04,
  FLASH_CMD_RDID		= 0x9f,
  FLASH_CMD_RDSR		= 0x05,
  FLASH_CMD_WRSR		= 0x01,
  FLASH_CMD_EN4B		= 0xb7,
  FLASH_CMD_EX4B		= 0xe9,
  FLASH_CMD_READ		= 0x03,
  FLASH_CMD_FAST_READ	= 0x0b,
  FLASH_CMD_RDSFDP	= 0x5a,
  FLASH_CMD_2READ		= 0xbb,
  FLASH_CMD_DREAD		= 0x3b,
  FLASH_CMD_4READ		= 0xeb,
  FLASH_CMD_QREAD		= 0x6b,
  FLASH_CMD_4PP		= 0x38,
  FLASH_CMD_SE		= 0xd8,
  //FLASH_CMD_BE		= 0xd8,
  FLASH_CMD_BE32K		= 0x52,
  FLASH_CMD_CE		= 0xc7,
  FLASH_CMD_PP		= 0x02,
  FLASH_CMD_CP		= 0xad,
  FLASH_CMD_DP		= 0xb9,
  FLASH_CMD_RDP		= 0xab,
  FLASH_CMD_RES		= 0xab,
  FLASH_CMD_REMS		= 0x90,
  FLASH_CMD_REMS2		= 0xef,
  FLASH_CMD_REMS4		= 0xdf,
  FLASH_CMD_ENSO		= 0xb1,
  FLASH_CMD_EXSO		= 0xc1,
  FLASH_CMD_RDSCUR	= 0x2b,
  FLASH_CMD_WRSCUR	= 0x2f,
  FLASH_CMD_ESRY		= 0x70,
  FLASH_CMD_DSRY		= 0x80,
  FLASH_CMD_CLSR		= 0x30,
  FLASH_CMD_HPM		= 0xa3,
  FLASH_CMD_WPSEL		= 0x68,
  FLASH_CMD_SBLK		= 0x36,
  FLASH_CMD_SBULK		= 0x39,
  FLASH_CMD_RDBLOCK	= 0x3c,
  FLASH_CMD_GBLK		= 0x7e,
  FLASH_CMD_GBULK		= 0x98
};

enum flash_status_t
{
  FLASH_STATUS_WIP	= 1 << 0,
  FLASH_STATUS_WEL	= 1 << 1,
  FLASH_STATUS_SRWD	= 1 << 7
};

enum flash_security_t
{
  FLASH_SECURITY_CP	= 1 << 4,
  FLASH_SECURITY_P_FAIL	= 1 << 5,
  FLASH_SECURITY_E_FAIL	= 1 << 6
};

/* ************************************************************************** */

static const int FLASH_PAGE_WRITE_SIZE = 32;

/* ************************************************************************** */

static struct
{
  uint32_t	active;
  enum flash_id_t	id;
  uint32_t	addr;
  uint8_t		*data;
  uint32_t	size;
} scheduled_write;

/* ************************************************************************** */

static void 	cmd_wrdi(enum flash_id_t);
static void	cmd_wren(enum flash_id_t);
//static void	cmd_clsr(enum flash_id_t);
static uint32_t	cmd_rdscur(enum flash_id_t);
static void	cmd_wrsr(enum flash_id_t, uint32_t);
static uint32_t	cmd_rdsr(enum flash_id_t);
static uint32_t	cmd_rdid(enum flash_id_t);
static uint32_t	cmd_pp(enum flash_id_t, flash_addr_t, uint8_t *, uint32_t);
static void	cmd_se(enum flash_id_t, uint32_t);
//static void	cmd_be(enum flash_id_t, uint32_t);
static void	cmd_ce(enum flash_id_t);

/* ************************************************************************** */

static void spi_write_addr(enum flash_id_t id, enum spi_transfer_flag_t flag, flash_addr_t addr)
{
  spi_write(id, SPI_TRANSFER_CONT, (addr >> 16) & 0xff);
  spi_read();
  spi_write(id, SPI_TRANSFER_CONT, (addr >> 8) & 0xff);
  spi_read();
  spi_write(id, flag, addr & 0xff);
  spi_read();
}

/* ************************************************************************** */

static uint32_t cmd_rdid(enum flash_id_t id)
{
  uint32_t result;

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_RDID);
  spi_read();

  spi_write(id, SPI_TRANSFER_CONT, 0);
  result = spi_read();

  result <<= 8;
  spi_write(id, SPI_TRANSFER_CONT, 0);
  result |= spi_read();

  result <<= 8;
  spi_write(id, SPI_TRANSFER_DONE, 0);
  result |= spi_read();

  return result;
}

/* ************************************************************************** */

static uint32_t cmd_rdsr(enum flash_id_t id)
{
  uint32_t result;

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_RDSR);
  spi_read();

  spi_write(id, SPI_TRANSFER_DONE, 0);
  result = spi_read();

  return result;
}

/* ************************************************************************** */

static void cmd_wrsr(enum flash_id_t id, uint32_t status) __attribute__((unused));
static void cmd_wrsr(enum flash_id_t id, uint32_t status)
{
  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_WRSR);
  spi_read();
  spi_write(id, SPI_TRANSFER_DONE, status);
  spi_read();
}

/* ************************************************************************** */

static uint32_t cmd_rdscur(enum flash_id_t id)
{
  uint32_t result;

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_RDSCUR);
  spi_read();

  spi_write(id, SPI_TRANSFER_DONE, 0);
  result = spi_read();

  return result;
}

/* ************************************************************************** */

//static void cmd_clsr(enum flash_id_t id)
//{
//  spi_write(id, SPI_TRANSFER_DONE, FLASH_CMD_CLSR);
//  spi_read();
//}

/* ************************************************************************** */

static void cmd_wren(enum flash_id_t id)
{
  while (!(cmd_rdsr(id) & FLASH_STATUS_WEL)) {
    spi_write(id, SPI_TRANSFER_DONE, FLASH_CMD_WREN);
    spi_read();
  }
}

/* ************************************************************************** */

static void cmd_wrdi(enum flash_id_t id)
{
  spi_write(id, SPI_TRANSFER_DONE, FLASH_CMD_WRDI);
  spi_read();
}

/* ************************************************************************** */

static uint32_t cmd_pp(enum flash_id_t id, flash_addr_t addr, uint8_t *data, uint32_t size)
{
  uint32_t i;

  if (size == 0) {
    return E_FLASH_OK;
  }

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_PP);
  spi_read();

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for (i=0;i<size-1;i++) {
    spi_write(id, SPI_TRANSFER_CONT, *data++);
    spi_read();
  }
  spi_write(id, SPI_TRANSFER_DONE, *data);
  spi_read();

//  if (cmd_rdscur(id) & FLASH_SECURITY_P_FAIL) {
//    return E_FLASH_WRITE_FAILED;
//  }

  return E_FLASH_OK;
}

/* ************************************************************************** */

static uint32_t cmd_ppi(enum flash_id_t id, flash_addr_t addr, uint8_t *data, uint32_t size)
{
  uint32_t i;

  if (size == 0) {
    return E_FLASH_OK;
  }

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_PP);
  spi_read();

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for (i=0;i<size-1;i++) {
    spi_write(id, SPI_TRANSFER_CONT, ~(*data++));
    spi_read();
  }
  spi_write(id, SPI_TRANSFER_DONE, ~(*data));
  spi_read();

//  if (cmd_rdscur(id) & FLASH_SECURITY_P_FAIL) {
//    return E_FLASH_WRITE_FAILED;
//  }

  return E_FLASH_OK;
}

/* ************************************************************************** */

static uint32_t cmd_cp(enum flash_id_t id, flash_addr_t addr, uint8_t *data, uint32_t size) __attribute__((unused));
static uint32_t cmd_cp(enum flash_id_t id, flash_addr_t addr, uint8_t *data, uint32_t size)
{
  if (size == 0) {
    return 0;
  }

  if (cmd_rdscur(id) & FLASH_SECURITY_CP) {
    DEBUG("Flash already in continous programming mode.\n");
    return 1;
  }

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_CP);
  spi_read();

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  while (size > 2) {
    size -= 2;

    spi_write(id, SPI_TRANSFER_CONT, *data++);
    spi_read();
    spi_write(id, SPI_TRANSFER_DONE, *data++);
    spi_read();

    if (!(cmd_rdscur(id) & FLASH_SECURITY_CP)) {
      DEBUG("Flash unexcpectedly left continous programming mode.\n");
      return 1;
    }

    spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_CP);
    spi_read();
  }

  spi_write(id, SPI_TRANSFER_CONT, *data++);
  spi_read();
  if (size == 2) {
    spi_write(id, SPI_TRANSFER_DONE, *data++);
  } else {
    spi_write(id, SPI_TRANSFER_DONE, 0xff);
  }
  spi_read();

  cmd_wrdi(id);

  return 0;
}

/* ************************************************************************** */

static void cmd_se(enum flash_id_t id, uint32_t sector)
{
  uint32_t addr = sector * FLASH_SECTOR_SIZE;

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_SE);
  spi_read();
  spi_write_addr(id, SPI_TRANSFER_DONE, addr);
}

/* ************************************************************************** */

static void cmd_ce(enum flash_id_t id)
{
  spi_write(id, SPI_TRANSFER_DONE, FLASH_CMD_CE);
  spi_read();
}

/* ************************************************************************** */

static void cmd_dp(enum flash_id_t id)
{
  spi_write(id, SPI_TRANSFER_DONE, FLASH_CMD_DP);
  spi_read();
}

/* ************************************************************************** */


static void cmd_rdp(enum flash_id_t id)
{
  spi_write(id, SPI_TRANSFER_DONE, FLASH_CMD_RDP);
  spi_read();
}

/* ************************************************************************** */

static void cmd_read(enum flash_id_t id, flash_addr_t addr, uint8_t *dest, uint32_t size)
{
  uint32_t i;

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_READ);
  spi_read();

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for (i=0;i<size-1;i++) {
    spi_write(id, SPI_TRANSFER_CONT, 0);
    *dest++ = spi_read();
  }
  spi_write(id, SPI_TRANSFER_DONE, 0);
  *dest = spi_read();
}

/* ************************************************************************** */

static void cmd_readi(enum flash_id_t id, flash_addr_t addr, uint8_t *dest, uint32_t size)
{
  uint32_t i;

  spi_write(id, SPI_TRANSFER_CONT, FLASH_CMD_READ);
  spi_read();

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for (i=0;i<size-1;i++) {
    spi_write(id, SPI_TRANSFER_CONT, 0);
    *dest++ = ~spi_read();
  }
  spi_write(id, SPI_TRANSFER_DONE, 0);
  *dest = ~spi_read();
}

/* ************************************************************************** */

static void flash_busy_wait(enum flash_id_t id)
{
  while (cmd_rdsr(id) & FLASH_STATUS_WIP);
}

/* ************************************************************************** */

static void flash_write_prepare(enum flash_id_t id)
{
  flash_busy_wait(id);
  //cmd_clsr(id);
  cmd_wren(id);
}

/* ************************************************************************** */

enum flash_error_t flash_init(void)
{
  uint32_t status;
  uint32_t jedec_id;
  int i;

  SIM_SCGC5  |= SIM_SCGC5_PORTD_MASK;

  PORTD_PCR7 |= 0x0100;     // Vp
  GPIOD_PDDR |= 0x0080; /* Setup PTD7 (Vp) as output */
  GPIOD_PSOR = (1<<7);

  spi_init();
  {
    // Wait a while for memories to start
    // TODO(henrik) Change this to more exact times.
    for(i = 0; i < 200; ++i)
    {
      udelay(1000);
    }
  }
  for (i = FLASH_ID0; i <= FLASH_ID0; ++i)
  {
    status = cmd_rdsr(i);
    if (status) {
      DEBUG("Error: Status of flash 0 is non-zero (0x%02x).\n", status);
      return E_FLASH_INVALID_STATUS;
    }
    jedec_id = cmd_rdid(i);
    if (jedec_id != 0x202015) {
      DEBUG("Flash0: Invalid JEDEC-ID: 0x%08x\n", jedec_id);
      return E_FLASH_UNKNOWN;
    }
  }

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_erase_chip(enum flash_id_t id, enum flash_flags_t flags)
{
  DEBUG("Starting chip erase...");

  if ((flags & FLASH_WAIT) == 0) {
    if (cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  flash_write_prepare(id);

  cmd_ce(id);

  if (flags & FLASH_FINISH) {
    flash_busy_wait(id);
//    if (cmd_rdscur(id) & FLASH_SECURITY_E_FAIL) {
//      DEBUG("failed!\n");
//    }
    cmd_wrdi(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_erase_sector(enum flash_id_t id, uint32_t sector, enum flash_flags_t flags)
{
  if ((flags & FLASH_WAIT) == 0) {
    if (cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  DEBUG("Starting sector erase...");

  flash_write_prepare(id);

  cmd_se(id, sector);

  if (flags & FLASH_FINISH) {
    flash_busy_wait(id);
//    if (cmd_rdscur(id) & FLASH_SECURITY_E_FAIL) {
//      DEBUG("failed! (0x%02x)\n");
//      return E_FLASH_ERASE_FAILED;
//    }
    cmd_wrdi(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_write_queue(enum flash_id_t id, flash_addr_t addr, uint8_t *data, uint32_t size, enum flash_flags_t flags)
{
  if (scheduled_write.active) {
    DEBUG("We already have a scheduled write.\n");
    return E_FLASH_QUEUE_FULL;
  }

  if (size == 0) {
    return E_FLASH_OK;
  }

  scheduled_write.active = 1;
  scheduled_write.id = id;
  scheduled_write.addr = addr;
  scheduled_write.data = data;
  scheduled_write.size = size;

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_write_process(enum flash_flags_t flags)
{
  uint32_t count;

  if (scheduled_write.active == 0) {
    return E_FLASH_OK;
  }

  if (flags & FLASH_WAIT) {
    if (cmd_rdsr(scheduled_write.id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  flash_write_prepare(scheduled_write.id);

  /* NOTE: FLASH_PAGE_WRITE_SIZE controls how much we write each time... */
  if (FLASH_PAGE_WRITE_SIZE >= FLASH_PAGE_SIZE ) {
    DEBUG("Sanity-check failed! We are trying to write too large chunks!\n");
  }
  count = FLASH_PAGE_WRITE_SIZE - (scheduled_write.addr % FLASH_PAGE_WRITE_SIZE);
  if (scheduled_write.size < count) {
    count = scheduled_write.size;
  }

  if (cmd_pp(scheduled_write.id, scheduled_write.addr, scheduled_write.data, count) != E_FLASH_OK) {
    DEBUG("flash_write_process(): Page-programming failed.\n");
    return E_FLASH_WRITE_FAILED;
  }

  scheduled_write.size -= count;
  scheduled_write.addr += count;
  scheduled_write.data += count;

  if (flags & FLASH_FINISH) {
    flash_busy_wait(scheduled_write.id);
    if (cmd_rdscur(scheduled_write.id) & FLASH_SECURITY_P_FAIL) {
      DEBUG("flash_write_process(): Page-programming failed.\n");
      return E_FLASH_WRITE_FAILED;
    }
  }

  if (scheduled_write.size == 0) {
    scheduled_write.active = 0;
    scheduled_write.id = 0;
    scheduled_write.addr = 0;
    scheduled_write.data = 0;
    return E_FLASH_OK;
  }

  return E_FLASH_BUSY;

}

/* ************************************************************************** */

enum flash_error_t flash_write(enum flash_id_t id, flash_addr_t addr, uint8_t *data, uint32_t size, enum flash_flags_t flags)
{
  uint32_t count;
  uint32_t offset = 0;

  DEBUG("Starting flash write operation...");

  if ((flags & FLASH_WAIT) == 0) {
    if (cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if (size == 0) {
    return E_FLASH_OK;
  }

  count = FLASH_PAGE_SIZE - (addr % FLASH_PAGE_SIZE);
  if (size > count) {
    flash_write_prepare(id);
    if (cmd_pp(id, addr+offset, data+offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  while (offset < size) {
    count = size - offset;
    if (count > FLASH_PAGE_SIZE) {
      count = FLASH_PAGE_SIZE;
    }

    flash_write_prepare(id);
    if (cmd_pp(id, addr+offset, data+offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  if (flags & FLASH_FINISH) {
    flash_busy_wait(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_writei(enum flash_id_t id, flash_addr_t addr, uint8_t *data, uint32_t size, enum flash_flags_t flags)
{
  uint32_t count;
  uint32_t offset = 0;

  DEBUG("Starting flash write operation...");

  if ((flags & FLASH_WAIT) == 0) {
    if (cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if (size == 0) {
    return E_FLASH_OK;
  }

  count = FLASH_PAGE_SIZE - (addr % FLASH_PAGE_SIZE);
  if (size > count) {
    flash_write_prepare(id);
    if (cmd_ppi(id, addr+offset, data+offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  while (offset < size) {
    count = size - offset;
    if (count > FLASH_PAGE_SIZE) {
      count = FLASH_PAGE_SIZE;
    }

    flash_write_prepare(id);
    if (cmd_ppi(id, addr+offset, data+offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  if (flags & FLASH_FINISH) {
    flash_busy_wait(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_read(enum flash_id_t id, flash_addr_t addr, uint8_t *dest, uint32_t size, enum flash_flags_t flags)
{
  if (flags & FLASH_WAIT) {
    flash_busy_wait(id);
  } else {
    if (cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if (size == 0) {
    return E_FLASH_OK;
  }

  DEBUG("Starting flash read operation...");
  cmd_read(id, addr, dest, size);
  DEBUG("done.\n");

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_readi(enum flash_id_t id, flash_addr_t addr, uint8_t *dest, uint32_t size, enum flash_flags_t flags)
{
  if (flags & FLASH_WAIT) {
    flash_busy_wait(id);
  } else {
    if (cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if (size == 0) {
    return E_FLASH_OK;
  }

  DEBUG("Starting flash read operation...");
  cmd_readi(id, addr, dest, size);
  DEBUG("done.\n");

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_status(enum flash_id_t id)
{
  if (cmd_rdsr(id) & FLASH_STATUS_WIP) {
    return E_FLASH_BUSY;
  } else if (scheduled_write.active) {
    return E_FLASH_BUSY;
  }

  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_sleep(enum flash_id_t id, enum flash_flags_t flags)
{
  cmd_dp(id);
  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_wakeup(enum flash_id_t id, enum flash_flags_t flags)
{
  cmd_rdp(id);
  return E_FLASH_OK;
}

/* ************************************************************************** */

enum flash_error_t flash_dump(enum flash_id_t id, void (*dump)(uint8_t))
{
  flash_addr_t offset = 0;
  uint8_t buf[FLASH_PAGE_SIZE];

  while (offset < FLASH_SIZE) {
    int i;
    enum flash_error_t err;

    err = flash_read(id, offset, buf, sizeof(buf), FLASH_BLOCKING);
    if (err != E_FLASH_OK) {
      DEBUG("Reading from flash failed while dumping (0x%02x).\r\n", err);
      return E_FLASH_WRITE_FAILED;
    }

    for (i = 0;i<sizeof(buf);i++) {
      if (dump) {
        dump(buf[i]);
      }
    }

    offset += sizeof(buf);
  }

  return E_FLASH_OK;
}
