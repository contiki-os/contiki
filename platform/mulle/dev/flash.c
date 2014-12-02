/* Author: Simon Aittamaa */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "K60.h"

#include "flash.h"

#include "udelay.h"

/* TODO(henrik) Clean up */

/* ************************************************************************** */
/* #define FLASH_DEBUG */
#ifdef FLASH_DEBUG
#include <stdio.h>
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...) (void)(0);
#endif

/* ************************************************************************** */

/** \todo move FLASH_CTAS to spi abstraction consolidation file. */
#define FLASH_CTAS 0

/* Spi functions */
typedef enum spi_transfer_flag {
  SPI_TRANSFER_DONE = 0,
  SPI_TRANSFER_CONT = 1
} spi_transfer_flag_t;

typedef enum spi_transfer_sync {
  SPI_TRANSFER_ASYNC = 0,
  SPI_TRANSFER_BLOCKING = 1
} spi_transfer_sync_t;

void
spi_init(void)
{
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(2); /* SPI0_PCS2 */
  PORTD->PCR[2] = PORT_PCR_MUX(2); /* SPI0_MOSI */
  PORTD->PCR[1] = PORT_PCR_MUX(2); /* SPI0_SCLK */
  PORTD->PCR[3] = PORT_PCR_MUX(2); /* SPI0_MISO */

  /* Enable clock gate for SPI0 module */
  SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;

  /* Configure SPI0 */
  /* Master mode */
  /* all peripheral chip select signals are active low */
  /* Disable TX,RX FIFO */
  SPI0->MCR = SPI_MCR_MSTR_MASK | SPI_MCR_PCSIS(0x1F) | SPI_MCR_DIS_RXF_MASK | SPI_MCR_DIS_TXF_MASK;     /* 0x803F3000; */
  /* 8 bit frame size */
  /* Set up different delays and clock scalers */
  /* TODO: These need tuning */
  /* FIXME: Coordinate SPI0 parameters between different peripheral drivers */
  /* IMPORTANT: Clock polarity is active low! */
  SPI0->CTAR[FLASH_CTAS] = SPI_CTAR_FMSZ(7) | SPI_CTAR_CSSCK(2) | SPI_CTAR_ASC(2) | SPI_CTAR_DT(2) | SPI_CTAR_BR(4); /*0x38002224; *//* TODO: Should be able to speed up */
}
/**
 * Perform a one byte transfer over SPI.
 *
 * \param ctas The CTAS register to use for timing information (0 or 1)
 * \param cs The chip select pins to assert. Bitmask, not PCS number.
 * \param cont Whether to keep asserting the chip select pin after the current transfer ends.
 * \param data The data to write to the slave.
 * \param blocking If set to SPI_TRANSFER_BLOCKING, wait until all bits have been transferred before returning.
 * \return The byte received from the slave during the same transfer.
 *
 * \note There is no need for separate read and write functions, since SPI transfers work like a shift register (one bit out, one bit in.)
 *
 * \todo Make SPI abstraction standalone.
 */
static uint8_t
spi_transfer(const uint8_t ctas, const uint32_t cs,
             const spi_transfer_flag_t cont, const uint32_t data,
             const spi_transfer_sync_t blocking)
{
  uint32_t spi_pushr;

  spi_pushr = SPI_PUSHR_TXDATA(data);
  spi_pushr |= SPI_PUSHR_CTAS(ctas);
  spi_pushr |= SPI_PUSHR_PCS(cs);
  if(cont == SPI_TRANSFER_CONT) {
    spi_pushr |= SPI_PUSHR_CONT_MASK;
  }

  /* Clear transfer complete flag */
  SPI0->SR |= SPI_SR_TCF_MASK;

  /* Shift a frame out/in */
  SPI0->PUSHR = spi_pushr;

  if(blocking) {
    /* Wait for transfer complete */
    while(!(SPI0->SR & SPI_SR_TCF_MASK));
  }

  /* Pop the buffer */
  return SPI0->POPR;
}
/* ************************************************************************** */
typedef enum flash_cmd {
  FLASH_CMD_WREN = 0x06,
  FLASH_CMD_WRDI = 0x04,
  FLASH_CMD_RDID = 0x9f,
  FLASH_CMD_RDSR = 0x05,
  FLASH_CMD_WRSR = 0x01,
  FLASH_CMD_EN4B = 0xb7,
  FLASH_CMD_EX4B = 0xe9,
  FLASH_CMD_READ = 0x03,
  FLASH_CMD_FAST_READ = 0x0b,
  FLASH_CMD_RDSFDP = 0x5a,
  FLASH_CMD_2READ = 0xbb,
  FLASH_CMD_DREAD = 0x3b,
  FLASH_CMD_4READ = 0xeb,
  FLASH_CMD_QREAD = 0x6b,
  FLASH_CMD_4PP = 0x38,
  FLASH_CMD_SE = 0xd8,
  /* FLASH_CMD_BE		= 0xd8, */
  FLASH_CMD_BE32K = 0x52,
  FLASH_CMD_CE = 0xc7,
  FLASH_CMD_PP = 0x02,
  FLASH_CMD_CP = 0xad,
  FLASH_CMD_DP = 0xb9,
  FLASH_CMD_RDP = 0xab,
  FLASH_CMD_RES = 0xab,
  FLASH_CMD_REMS = 0x90,
  FLASH_CMD_REMS2 = 0xef,
  FLASH_CMD_REMS4 = 0xdf,
  FLASH_CMD_ENSO = 0xb1,
  FLASH_CMD_EXSO = 0xc1,
  FLASH_CMD_RDSCUR = 0x2b,
  FLASH_CMD_WRSCUR = 0x2f,
  FLASH_CMD_ESRY = 0x70,
  FLASH_CMD_DSRY = 0x80,
  FLASH_CMD_CLSR = 0x30,
  FLASH_CMD_HPM = 0xa3,
  FLASH_CMD_WPSEL = 0x68,
  FLASH_CMD_SBLK = 0x36,
  FLASH_CMD_SBULK = 0x39,
  FLASH_CMD_RDBLOCK = 0x3c,
  FLASH_CMD_GBLK = 0x7e,
  FLASH_CMD_GBULK = 0x98
} flash_cmd_t;

typedef enum flash_status {
  FLASH_STATUS_WIP = 1 << 0,
  FLASH_STATUS_WEL = 1 << 1,
  FLASH_STATUS_SRWD = 1 << 7
} flash_status_t;

typedef enum flash_security {
  FLASH_SECURITY_CP = 1 << 4,
  FLASH_SECURITY_P_FAIL = 1 << 5,
  FLASH_SECURITY_E_FAIL = 1 << 6
} flash_security_t;

/* ************************************************************************** */

static const int FLASH_PAGE_WRITE_SIZE = 32;

/* ************************************************************************** */

static struct {
  uint32_t active;
  flash_id_t id;
  uint32_t addr;
  const uint8_t *data;
  uint32_t size;
} scheduled_write;

/* ************************************************************************** */

static void   cmd_wrdi(const flash_id_t);
static void cmd_wren(const flash_id_t);
/* static void	cmd_clsr(const flash_id_t); */
static uint32_t cmd_rdscur(const flash_id_t);
static void cmd_wrsr(const flash_id_t, const uint32_t);
static uint32_t cmd_rdsr(const flash_id_t);
static uint32_t cmd_rdid(const flash_id_t);
static uint32_t cmd_pp(const flash_id_t, const flash_addr_t, const uint8_t *, const uint32_t);
static void cmd_se(const flash_id_t, const uint32_t);
/* static void	cmd_be(const flash_id_t, const uint32_t); */
static void cmd_ce(const flash_id_t);

/* ************************************************************************** */

static void
spi_write_addr(const flash_id_t id, const spi_transfer_flag_t flag, const flash_addr_t addr)
{
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, (addr >> 16) & 0xff, SPI_TRANSFER_BLOCKING);
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, (addr >> 8) & 0xff, SPI_TRANSFER_BLOCKING);
  spi_transfer(FLASH_CTAS, id, flag, addr & 0xff, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

/** Read ID */
static uint32_t
cmd_rdid(const flash_id_t id)
{
  uint32_t result;

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_RDID, SPI_TRANSFER_BLOCKING);
  result = spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, 0, SPI_TRANSFER_BLOCKING);
  result <<= 8;
  result |= spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, 0, SPI_TRANSFER_BLOCKING);
  result <<= 8;
  result |= spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, 0, SPI_TRANSFER_BLOCKING);

  return result;
}
/* ************************************************************************** */

/** Read status register */
static uint32_t
cmd_rdsr(const flash_id_t id)
{
  uint32_t result;

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_RDSR, SPI_TRANSFER_BLOCKING);
  result = spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, 0, SPI_TRANSFER_BLOCKING);

  return result;
}
/* ************************************************************************** */

static void cmd_wrsr(const flash_id_t id, const uint32_t status) __attribute__((unused));

/** Write status register */
static void
cmd_wrsr(const flash_id_t id, const uint32_t status)
{
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_WRSR, SPI_TRANSFER_BLOCKING);
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, status, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

static uint32_t
cmd_rdscur(const flash_id_t id)
{
  uint32_t result;

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_RDSCUR, SPI_TRANSFER_BLOCKING);
  result = spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, 0, SPI_TRANSFER_BLOCKING);

  return result;
}
/* ************************************************************************** */

/*
   static void cmd_clsr(const flash_id_t id)
   {
   spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, FLASH_CMD_CLSR, SPI_TRANSFER_BLOCKING);
   }
 */

/* ************************************************************************** */

static void
cmd_wren(const flash_id_t id)
{
  while(!(cmd_rdsr(id) & FLASH_STATUS_WEL)) {
    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, FLASH_CMD_WREN, SPI_TRANSFER_BLOCKING);
  }
}
/* ************************************************************************** */

static void
cmd_wrdi(const flash_id_t id)
{
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, FLASH_CMD_WRDI, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

static uint32_t
cmd_pp(const flash_id_t id, const flash_addr_t addr, const uint8_t *data, const uint32_t size)
{
  uint32_t i;

  if(size == 0) {
    return E_FLASH_OK;
  }

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_PP, SPI_TRANSFER_BLOCKING);

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for(i = 0; i < size - 1; ++i) {
    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, *(data++), SPI_TRANSFER_BLOCKING);
  }
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, *data, SPI_TRANSFER_BLOCKING);

/*
   if (cmd_rdscur(id) & FLASH_SECURITY_P_FAIL) {
    return E_FLASH_WRITE_FAILED;
   }
 */

  return E_FLASH_OK;
}
/* ************************************************************************** */

static uint32_t
cmd_ppi(const flash_id_t id, const flash_addr_t addr, const uint8_t *data, const uint32_t size)
{
  uint32_t i;

  if(size == 0) {
    return E_FLASH_OK;
  }

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_PP, SPI_TRANSFER_BLOCKING);

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for(i = 0; i < size - 1; ++i) {
    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, ~(*(data++)), SPI_TRANSFER_BLOCKING);
  }
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, ~(*data), SPI_TRANSFER_BLOCKING);

/*  if (cmd_rdscur(id) & FLASH_SECURITY_P_FAIL) { */
/*    return E_FLASH_WRITE_FAILED; */
/*  } */

  return E_FLASH_OK;
}
/* ************************************************************************** */

static uint32_t cmd_cp(const flash_id_t id, const flash_addr_t addr, const uint8_t *data, const uint32_t size) __attribute__((unused));
static uint32_t
cmd_cp(const flash_id_t id, const flash_addr_t addr, const uint8_t *data, uint32_t size)
{
  if(size == 0) {
    return 0;
  }

  if(cmd_rdscur(id) & FLASH_SECURITY_CP) {
    DEBUG("Flash already in continous programming mode.\n");
    return 1;
  }

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_CP, SPI_TRANSFER_BLOCKING);

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  while(size > 2) {
    size -= 2;

    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, *(data++), SPI_TRANSFER_BLOCKING);
    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, *(data++), SPI_TRANSFER_BLOCKING);

    if(!(cmd_rdscur(id) & FLASH_SECURITY_CP)) {
      DEBUG("Flash unexcpectedly left continous programming mode.\n");
      return 1;
    }

    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_CP, SPI_TRANSFER_BLOCKING);
  }

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, *(data++), SPI_TRANSFER_BLOCKING);
  if(size == 2) {
    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, *(data++), SPI_TRANSFER_BLOCKING);
  } else {
    spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, 0xff, SPI_TRANSFER_BLOCKING);
  }

  cmd_wrdi(id);

  return 0;
}
/* ************************************************************************** */

static void
cmd_se(const flash_id_t id, const uint32_t sector)
{
  uint32_t addr = sector * FLASH_SECTOR_SIZE;

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_SE, SPI_TRANSFER_BLOCKING);
  spi_write_addr(id, SPI_TRANSFER_DONE, addr);
}
/* ************************************************************************** */

static void
cmd_ce(const flash_id_t id)
{
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, FLASH_CMD_CE, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

static void
cmd_dp(const flash_id_t id)
{
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, FLASH_CMD_DP, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

static void
cmd_rdp(const flash_id_t id)
{
  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, FLASH_CMD_RDP, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

static void
cmd_read(const flash_id_t id, const flash_addr_t addr, uint8_t *dest, const uint32_t size)
{
  uint32_t i;

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_READ, SPI_TRANSFER_BLOCKING);

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for(i = 0; i < size - 1; ++i) {
    *(dest++) = spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, 0, SPI_TRANSFER_BLOCKING);
  }
  *dest = spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, 0, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

static void
cmd_readi(const flash_id_t id, const flash_addr_t addr, uint8_t *dest, const uint32_t size)
{
  uint32_t i;

  spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, FLASH_CMD_READ, SPI_TRANSFER_BLOCKING);

  spi_write_addr(id, SPI_TRANSFER_CONT, addr);

  for(i = 0; i < size - 1; ++i) {
    *(dest++) = ~spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_CONT, 0, SPI_TRANSFER_BLOCKING);
  }
  *dest = ~spi_transfer(FLASH_CTAS, id, SPI_TRANSFER_DONE, 0, SPI_TRANSFER_BLOCKING);
}
/* ************************************************************************** */

static void
flash_busy_wait(const flash_id_t id)
{
  while(cmd_rdsr(id) & FLASH_STATUS_WIP) {
    /* XXX: Attempt to fix slow erase times on some chips by not flooding the device with status requests */
    udelay(1000);
  }
}
/* ************************************************************************** */

static void
flash_write_prepare(const flash_id_t id)
{
  flash_busy_wait(id);
  /* cmd_clsr(id); */
  cmd_wren(id);
}
/* ************************************************************************** */

flash_error_t
flash_init(void)
{
  uint32_t status;
  uint32_t jedec_id;
  int i;

  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

  spi_init();
  {
    /* Wait a while for memories to start */
    /* TODO(henrik) Change this to more exact times. */
    for(i = 0; i < 200; ++i) {
      udelay(1000);
    }
  }
  for(i = FLASH_ID0; i <= FLASH_ID0; ++i) {
    status = cmd_rdsr(i);
    if(status) {
      DEBUG("Error: Status of flash 0 is non-zero (0x%02x).\n", status);
      return E_FLASH_INVALID_STATUS;
    }
    jedec_id = cmd_rdid(i);
    if(jedec_id != 0x202015) {
      DEBUG("Flash0: Invalid JEDEC-ID: 0x%08x\n", jedec_id);
      return E_FLASH_UNKNOWN;
    }
  }

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_erase_chip(const flash_id_t id, const flash_flags_t flags)
{
  DEBUG("Starting chip erase...");

  if((flags & FLASH_WAIT) == 0) {
    if(cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  flash_write_prepare(id);

  cmd_ce(id);

  if(flags & FLASH_FINISH) {
    flash_busy_wait(id);
/*    if (cmd_rdscur(id) & FLASH_SECURITY_E_FAIL) { */
/*      DEBUG("failed!\n"); */
/*    } */
    cmd_wrdi(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_erase_sector(const flash_id_t id, const uint32_t sector, const flash_flags_t flags)
{
  if((flags & FLASH_WAIT) == 0) {
    if(cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  DEBUG("Starting sector erase...");

  flash_write_prepare(id);

  cmd_se(id, sector);

  if(flags & FLASH_FINISH) {
    flash_busy_wait(id);
/*    if (cmd_rdscur(id) & FLASH_SECURITY_E_FAIL) { */
/*      DEBUG("failed! (0x%02x)\n"); */
/*      return E_FLASH_ERASE_FAILED; */
/*    } */
    cmd_wrdi(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_write_queue(const flash_id_t id, const flash_addr_t addr, const uint8_t *data, const uint32_t size, const flash_flags_t flags)
{
  if(scheduled_write.active) {
    DEBUG("We already have a scheduled write.\n");
    return E_FLASH_QUEUE_FULL;
  }

  if(size == 0) {
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

flash_error_t
flash_write_process(const flash_flags_t flags)
{
  uint32_t count;

  if(scheduled_write.active == 0) {
    return E_FLASH_OK;
  }

  if(flags & FLASH_WAIT) {
    if(cmd_rdsr(scheduled_write.id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  flash_write_prepare(scheduled_write.id);

  /* NOTE: FLASH_PAGE_WRITE_SIZE controls how much we write each time... */
  if(FLASH_PAGE_WRITE_SIZE >= FLASH_PAGE_SIZE) {
    DEBUG("Sanity-check failed! We are trying to write too large chunks!\n");
  }
  count = FLASH_PAGE_WRITE_SIZE - (scheduled_write.addr % FLASH_PAGE_WRITE_SIZE);
  if(scheduled_write.size < count) {
    count = scheduled_write.size;
  }

  if(cmd_pp(scheduled_write.id, scheduled_write.addr, scheduled_write.data, count) != E_FLASH_OK) {
    DEBUG("flash_write_process(): Page-programming failed.\n");
    return E_FLASH_WRITE_FAILED;
  }

  scheduled_write.size -= count;
  scheduled_write.addr += count;
  scheduled_write.data += count;

  if(flags & FLASH_FINISH) {
    flash_busy_wait(scheduled_write.id);
    if(cmd_rdscur(scheduled_write.id) & FLASH_SECURITY_P_FAIL) {
      DEBUG("flash_write_process(): Page-programming failed.\n");
      return E_FLASH_WRITE_FAILED;
    }
  }

  if(scheduled_write.size == 0) {
    scheduled_write.active = 0;
    scheduled_write.id = 0;
    scheduled_write.addr = 0;
    scheduled_write.data = 0;
    return E_FLASH_OK;
  }

  return E_FLASH_BUSY;
}
/* ************************************************************************** */

flash_error_t
flash_write(const flash_id_t id, const flash_addr_t addr, const uint8_t *data, const uint32_t size, const flash_flags_t flags)
{
  uint32_t count;
  uint32_t offset = 0;

  DEBUG("Starting flash write operation...");

  if((flags & FLASH_WAIT) == 0) {
    if(cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if(size == 0) {
    return E_FLASH_OK;
  }

  count = FLASH_PAGE_SIZE - (addr % FLASH_PAGE_SIZE);
  if(size > count) {
    flash_write_prepare(id);
    if(cmd_pp(id, addr + offset, data + offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  while(offset < size) {
    count = size - offset;
    if(count > FLASH_PAGE_SIZE) {
      count = FLASH_PAGE_SIZE;
    }

    flash_write_prepare(id);
    if(cmd_pp(id, addr + offset, data + offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  if(flags & FLASH_FINISH) {
    flash_busy_wait(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_writei(const flash_id_t id, const flash_addr_t addr, const uint8_t *data, const uint32_t size, const flash_flags_t flags)
{
  uint32_t count;
  uint32_t offset = 0;

  DEBUG("Starting flash write operation...");

  if((flags & FLASH_WAIT) == 0) {
    if(cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if(size == 0) {
    return E_FLASH_OK;
  }

  count = FLASH_PAGE_SIZE - (addr % FLASH_PAGE_SIZE);
  if(size > count) {
    flash_write_prepare(id);
    if(cmd_ppi(id, addr + offset, data + offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  while(offset < size) {
    count = size - offset;
    if(count > FLASH_PAGE_SIZE) {
      count = FLASH_PAGE_SIZE;
    }

    flash_write_prepare(id);
    if(cmd_ppi(id, addr + offset, data + offset, count)) {
      DEBUG("failed!\n");
      return E_FLASH_WRITE_FAILED;
    }

    offset += count;
  }

  if(flags & FLASH_FINISH) {
    flash_busy_wait(id);
  }

  DEBUG("done.\n");

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_read(const flash_id_t id, const flash_addr_t addr, uint8_t *dest, const uint32_t size, const flash_flags_t flags)
{
  if(flags & FLASH_WAIT) {
    flash_busy_wait(id);
  } else {
    if(cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if(size == 0) {
    return E_FLASH_OK;
  }

  DEBUG("Starting flash read operation...");
  cmd_read(id, addr, dest, size);
  DEBUG("done.\n");

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_readi(const flash_id_t id, const flash_addr_t addr, uint8_t *dest, const uint32_t size, const flash_flags_t flags)
{
  if(flags & FLASH_WAIT) {
    flash_busy_wait(id);
  } else {
    if(cmd_rdsr(id) & FLASH_STATUS_WIP) {
      return E_FLASH_BUSY;
    }
  }

  if(size == 0) {
    return E_FLASH_OK;
  }

  DEBUG("Starting flash read operation...");
  cmd_readi(id, addr, dest, size);
  DEBUG("done.\n");

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_status(const flash_id_t id)
{
  if(cmd_rdsr(id) & FLASH_STATUS_WIP) {
    return E_FLASH_BUSY;
  } else if(scheduled_write.active) {
    return E_FLASH_BUSY;
  }

  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_sleep(const flash_id_t id, const flash_flags_t flags)
{
  cmd_dp(id);
  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_wakeup(const flash_id_t id, const flash_flags_t flags)
{
  cmd_rdp(id);
  return E_FLASH_OK;
}
/* ************************************************************************** */

flash_error_t
flash_dump(const flash_id_t id, void (*dump)(const uint8_t))
{
  flash_addr_t offset = 0;
  uint8_t buf[FLASH_PAGE_SIZE];

  while(offset < FLASH_SIZE) {
    int i;
    flash_error_t err;

    err = flash_read(id, offset, buf, sizeof(buf), FLASH_BLOCKING);
    if(err != E_FLASH_OK) {
      DEBUG("Reading from flash failed while dumping (0x%02x).\r\n", err);
      return E_FLASH_WRITE_FAILED;
    }

    for(i = 0; i < sizeof(buf); i++) {
      if(dump) {
        dump(buf[i]);
      }
    }

    offset += sizeof(buf);
  }

  return E_FLASH_OK;
}
