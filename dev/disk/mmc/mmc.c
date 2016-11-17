/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit@wsystem.com>
 * All rights reserved.
 *
 * Based on the FatFs Module STM32 Sample Project,
 * Copyright (c) 2014, ChaN
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup mmc
 * @{
 *
 * \file
 * Implementation of the SD/MMC device driver.
 */
#include <stddef.h>
#include <stdint.h>
#include "contiki.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/watchdog.h"
#include "mmc-arch.h"
#include "mmc.h"

/* Data read/write block length */
#define BLOCK_LEN       512
/*
 * Logical sector size exposed to the disk API, not to be confused with the SDSC
 * sector size, which is the size of an erasable sector
 */
#define SECTOR_SIZE     BLOCK_LEN

/* Clock frequency in card identification mode: fOD <= 400 kHz */
#define CLOCK_FREQ_CARD_ID_MODE         400000
/*
 * Clock frequency in data transfer mode: fPP <= 20 MHz, limited by the
 * backward-compatible MMC interface timings
 */
#define CLOCK_FREQ_DATA_XFER_MODE       20000000

/* SPI-mode command list */
#define CMD0    0               /* GO_IDLE_STATE */
#define CMD1    1               /* SEND_OP_COND */
#define CMD8    8               /* SEND_IF_COND */
#define CMD8_VHS_2_7_3_6        0x1
#define CMD8_ARG(vhs, check_pattern)    ((vhs) << 8 | (check_pattern))
#define CMD8_ECHO_MASK          0x00000fff
#define CMD9    9               /* SEND_CSD */
#define CMD10   10              /* SEND_CID */
#define CMD12   12              /* STOP_TRANSMISSION */
#define CMD13   13              /* SEND_STATUS */
#define CMD16   16              /* SET_BLOCKLEN */
#define CMD17   17              /* READ_SINGLE_BLOCK */
#define CMD18   18              /* READ_MULTIPLE_BLOCK */
#define CMD23   23              /* SET_BLOCK_COUNT */
#define CMD24   24              /* WRITE_BLOCK */
#define CMD25   25              /* WRITE_MULTIPLE_BLOCK */
#define CMD32   32              /* ERASE_WR_BLK_START */
#define CMD33   33              /* ERASE_WR_BLK_END */
#define CMD38   38              /* ERASE */
#define CMD55   55              /* APP_CMD */
#define CMD58   58              /* READ_OCR */
#define ACMD    0x80            /* Application-specific command */
#define ACMD13  (ACMD | 13)     /* SD_STATUS */
#define ACMD23  (ACMD | 23)     /* SET_WR_BLK_ERASE_COUNT */
#define ACMD41  (ACMD | 41)     /* SD_APP_OP_COND */
#define ACMD41_HCS              (1 << 30)

#define CMD_TX  0x40            /* Command transmission bit */

#define R1_MSB                  0x00
#define R1_SUCCESS              0x00
#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

#define TOK_DATA_RESP_MASK              0x1f
#define TOK_DATA_RESP_ACCEPTED          0x05
#define TOK_DATA_RESP_CRC_ERROR         0x0b
#define TOK_DATA_RESP_WR_ERROR          0x0d
#define TOK_RD_SINGLE_WR_START_BLOCK    0xfe
#define TOK_MULTI_WR_START_BLOCK        0xfc
#define TOK_MULTI_WR_STOP_TRAN          0xfd

/* The SD Status is one data block of 512 bits. */
#define SD_STATUS_SIZE                  (512 / 8)
#define SD_STATUS_AU_SIZE(sd_status)    ((sd_status)[10] >> 4)

#define OCR_CCS         (1 << 30)

#define CSD_SIZE                        16
#define CSD_STRUCTURE(csd)              ((csd)[0] >> 6)
#define CSD_STRUCTURE_SD_V1_0           0
#define CSD_STRUCTURE_SD_V2_0           1
#define CSD_SD_V1_0_READ_BL_LEN(csd)    ((csd)[5] & 0x0f)
#define CSD_SD_V1_0_BLOCK_LEN(csd)      (1ull << CSD_SD_V1_0_READ_BL_LEN(csd))
#define CSD_SD_V1_0_C_SIZE(csd)         \
  (((csd)[6] & 0x03) << 10 | (csd)[7] << 2 | (csd)[8] >> 6)
#define CSD_SD_V1_0_C_SIZE_MULT(csd)    \
  (((csd)[9] & 0x03) << 1 | (csd)[10] >> 7)
#define CSD_SD_V1_0_MULT(csd)           \
  (1 << (CSD_SD_V1_0_C_SIZE_MULT(csd) + 2))
#define CSD_SD_V1_0_BLOCKNR(csd)        \
  (((uint32_t)CSD_SD_V1_0_C_SIZE(csd) + 1) * CSD_SD_V1_0_MULT(csd))
#define CSD_SD_V1_0_CAPACITY(csd)       \
  (CSD_SD_V1_0_BLOCKNR(csd) * CSD_SD_V1_0_BLOCK_LEN(csd))
#define CSD_SD_V1_0_SECTOR_SIZE(csd)    \
  (((csd)[10] & 0x3f) << 1 | (csd)[11] >> 7)
#define CSD_SD_V1_0_WRITE_BL_LEN(csd)   \
  (((csd)[12] & 0x03) << 2 | (csd)[13] >> 6)
#define CSD_SD_V2_0_C_SIZE(csd)         \
  (((csd)[7] & 0x3f) << 16 | (csd)[8] << 8 | (csd)[9])
#define CSD_SD_V2_0_CAPACITY(csd)       \
  (((uint64_t)CSD_SD_V2_0_C_SIZE(csd) + 1) << 19)
#define CSD_MMC_ERASE_GRP_SIZE(csd)     (((csd)[10] & 0x7c) >> 2)
#define CSD_MMC_ERASE_GRP_MULT(csd)     \
  (((csd)[10] & 0x03) << 3 | (csd)[11] >> 5)
#define CSD_MMC_WRITE_BL_LEN(csd)       \
  (((csd)[12] & 0x03) << 2 | (csd)[13] >> 6)

typedef enum {
  CARD_TYPE_MMC   = 0x01,                               /* MMC v3 */
  CARD_TYPE_SD1   = 0x02,                               /* SD v1 */
  CARD_TYPE_SD2   = 0x04,                               /* SD v2 */
  CARD_TYPE_SD    = CARD_TYPE_SD1 | CARD_TYPE_SD2,      /* SD */
  CARD_TYPE_BLOCK = 0x08                                /* Block addressing */
} card_type_t;

static struct mmc_priv {
  uint8_t status;
  uint8_t card_type;
} mmc_priv[MMC_CONF_DEV_COUNT];

/*----------------------------------------------------------------------------*/
static uint8_t
mmc_spi_xchg(uint8_t dev, uint8_t tx_byte)
{
  uint8_t rx_byte;

  mmc_arch_spi_xfer(dev, &tx_byte, 1, &rx_byte, 1);
  return rx_byte;
}
/*----------------------------------------------------------------------------*/
static void
mmc_spi_tx(uint8_t dev, const void *buf, size_t cnt)
{
  mmc_arch_spi_xfer(dev, buf, cnt, NULL, 0);
}
/*----------------------------------------------------------------------------*/
static void
mmc_spi_rx(uint8_t dev, void *buf, size_t cnt)
{
  mmc_arch_spi_xfer(dev, NULL, 0, buf, cnt);
}
/*----------------------------------------------------------------------------*/
static bool
mmc_wait_ready(uint8_t dev, uint16_t timeout_ms)
{
  rtimer_clock_t timeout_end =
    RTIMER_NOW() + ((uint32_t)timeout_ms * RTIMER_SECOND + 999) / 1000;
  uint8_t rx_byte;

  do {
    rx_byte = mmc_spi_xchg(dev, 0xff);
    watchdog_periodic();
  } while(rx_byte != 0xff && RTIMER_CLOCK_LT(RTIMER_NOW(), timeout_end));
  return rx_byte == 0xff;
}
/*----------------------------------------------------------------------------*/
static bool
mmc_select(uint8_t dev, bool sel)
{
  mmc_arch_spi_select(dev, sel);
  mmc_spi_xchg(dev, 0xff); /* Dummy clock (force D0) */
  if(sel && !mmc_wait_ready(dev, 500)) {
    mmc_select(dev, false);
    return false;
  }
  return true;
}
/*----------------------------------------------------------------------------*/
static uint8_t
mmc_send_cmd(uint8_t dev, uint8_t cmd, uint32_t arg)
{
  uint8_t resp, n;

  /* Send a CMD55 prior to a ACMD<n>. */
  if(cmd & ACMD) {
    cmd &= ~ACMD;
    resp = mmc_send_cmd(dev, CMD55, 0);
    if(resp != R1_SUCCESS && resp != R1_IDLE_STATE) {
      return resp;
    }
  }

  /*
   * Select the card and wait for ready, except to stop a multiple-block read.
   */
  if(cmd != CMD12) {
    mmc_select(dev, false);
    if(!mmc_select(dev, true)) {
      return 0xff;
    }
  }

  /* Send the command packet. */
  mmc_spi_xchg(dev, CMD_TX | cmd);      /* Start & tx bits, cmd index */
  mmc_spi_xchg(dev, arg >> 24);         /* Argument[31..24] */
  mmc_spi_xchg(dev, arg >> 16);         /* Argument[23..16] */
  mmc_spi_xchg(dev, arg >> 8);          /* Argument[15..8] */
  mmc_spi_xchg(dev, arg);               /* Argument[7..0] */
  switch(cmd) {
  case CMD0:
    n = 0x95;                           /* CMD0(0) CRC7, end bit */
    break;
  case CMD8:
    n = 0x87;                           /* CMD8(0x1aa) CRC7, end bit */
    break;
  default:
    n = 0x01;                           /* Dummy CRC7, end bit */
    break;
  }
  mmc_spi_xchg(dev, n);

  /* Receive the command response. */
  if(cmd == CMD12) {
    mmc_spi_xchg(dev, 0xff); /* Discard following byte if CMD12. */
  }
  /* Wait for the response (max. 10 bytes). */
  n = 10;
  do {
    resp = mmc_spi_xchg(dev, 0xff);
  } while((resp & 0x80) != R1_MSB && --n);
  return resp;
}
/*----------------------------------------------------------------------------*/
static bool
mmc_tx_block(uint8_t dev, const void *buf, uint8_t token)
{
  uint8_t resp;

  if(!mmc_wait_ready(dev, 500)) {
    return false;
  }

  mmc_spi_xchg(dev, token);
  if(token != TOK_MULTI_WR_STOP_TRAN) {
    mmc_spi_tx(dev, buf, BLOCK_LEN);
    mmc_spi_xchg(dev, 0xff); /* Dummy CRC */
    mmc_spi_xchg(dev, 0xff);

    resp = mmc_spi_xchg(dev, 0xff);
    if((resp & TOK_DATA_RESP_MASK) != TOK_DATA_RESP_ACCEPTED) {
      return false;
    }
  }
  return true;
}
/*----------------------------------------------------------------------------*/
static bool
mmc_rx(uint8_t dev, void *buf, size_t cnt)
{
  rtimer_clock_t timeout_end =
    RTIMER_NOW() + (200ul * RTIMER_SECOND + 999) / 1000;
  uint8_t token;

  do {
    token = mmc_spi_xchg(dev, 0xff);
    watchdog_periodic();
  } while(token == 0xff && RTIMER_CLOCK_LT(RTIMER_NOW(), timeout_end));
  if(token != TOK_RD_SINGLE_WR_START_BLOCK) {
    return false;
  }

  mmc_spi_rx(dev, buf, cnt);
  mmc_spi_xchg(dev, 0xff); /* Discard CRC. */
  mmc_spi_xchg(dev, 0xff);
  return true;
}
/*----------------------------------------------------------------------------*/
void
mmc_arch_cd_changed_callback(uint8_t dev, bool cd)
{
  uint8_t status;

  if(dev >= MMC_CONF_DEV_COUNT) {
    return;
  }

  if(cd) {
    status = DISK_STATUS_DISK;
    if(!mmc_arch_get_wp(dev)) {
      status |= DISK_STATUS_WRITABLE;
    }
  } else {
    status = 0;
  }
  mmc_priv[dev].status = status;
}
/*----------------------------------------------------------------------------*/
static disk_status_t
mmc_status(uint8_t dev)
{
  bool cd;
  struct mmc_priv *priv;

  if(dev >= MMC_CONF_DEV_COUNT) {
    return DISK_RESULT_INVALID_ARG;
  }

  cd = mmc_arch_get_cd(dev);
  priv = &mmc_priv[dev];
  if(cd == !(priv->status & DISK_STATUS_DISK)) {
    mmc_arch_cd_changed_callback(dev, cd);
  }
  return priv->status;
}
/*----------------------------------------------------------------------------*/
static disk_status_t
mmc_initialize(uint8_t dev)
{
  disk_status_t status;
  uint8_t n, cmd;
  card_type_t card_type;
  rtimer_clock_t timeout_end;
  uint32_t arg, resp, ocr;
  struct mmc_priv *priv;

  if(dev >= MMC_CONF_DEV_COUNT) {
    return DISK_RESULT_INVALID_ARG;
  }
  status = mmc_status(dev);
  if(!(status & DISK_STATUS_DISK)) {
    return status;
  }

  mmc_arch_spi_select(dev, false);
  clock_delay_usec(10000);

  mmc_arch_spi_set_clock_freq(dev, CLOCK_FREQ_CARD_ID_MODE);
  for(n = 10; n; n--) {
    mmc_spi_xchg(dev, 0xff); /* Generate 80 dummy clock cycles. */
  }

  card_type = 0;
  if(mmc_send_cmd(dev, CMD0, 0) == R1_IDLE_STATE) {
    timeout_end = RTIMER_NOW() + RTIMER_SECOND;
    arg = CMD8_ARG(CMD8_VHS_2_7_3_6, 0xaa); /* Arbitrary check pattern */
    if(mmc_send_cmd(dev, CMD8, arg) == R1_IDLE_STATE) { /* SD v2? */
      resp = 0;
      for(n = 4; n; n--) {
        resp = resp << 8 | mmc_spi_xchg(dev, 0xff);
      }
      /* Does the card support 2.7 V - 3.6 V? */
      if((arg & CMD8_ECHO_MASK) == (resp & CMD8_ECHO_MASK)) {
        /* Wait for end of initialization. */
        while(RTIMER_CLOCK_LT(RTIMER_NOW(), timeout_end) &&
              mmc_send_cmd(dev, ACMD41, ACMD41_HCS) != R1_SUCCESS) {
          watchdog_periodic();
        }
        if(RTIMER_CLOCK_LT(RTIMER_NOW(), timeout_end) &&
           mmc_send_cmd(dev, CMD58, 0) == R1_SUCCESS) { /* Read OCR. */
          ocr = 0;
          for(n = 4; n; n--) {
            ocr = ocr << 8 | mmc_spi_xchg(dev, 0xff);
          }
          card_type = CARD_TYPE_SD2;
          if(ocr & OCR_CCS) {
            card_type |= CARD_TYPE_BLOCK;
          }
        }
      }
    } else { /* Not SD v2 */
      resp = mmc_send_cmd(dev, ACMD41, 0);
      if(resp == R1_SUCCESS || resp == R1_IDLE_STATE) { /* SD v1 or MMC? */
        card_type = CARD_TYPE_SD1;
        cmd = ACMD41;
      } else {
        card_type = CARD_TYPE_MMC;
        cmd = CMD1;
      }
      /* Wait for end of initialization. */
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), timeout_end) &&
            mmc_send_cmd(dev, cmd, 0) != R1_SUCCESS) {
        watchdog_periodic();
      }
      /* Set block length. */
      if(!RTIMER_CLOCK_LT(RTIMER_NOW(), timeout_end) ||
         mmc_send_cmd(dev, CMD16, BLOCK_LEN) != R1_SUCCESS) {
        card_type = 0;
      }
    }
  }
  priv = &mmc_priv[dev];
  priv->card_type = card_type;
  mmc_select(dev, false);

  status = priv->status;
  if(status & DISK_STATUS_DISK && card_type) { /* OK */
    mmc_arch_spi_set_clock_freq(dev, CLOCK_FREQ_DATA_XFER_MODE);
    status |= DISK_STATUS_INIT;
  } else { /* Failed */
    status &= ~DISK_STATUS_INIT;
  }
  priv->status = status;
  return status;
}
/*----------------------------------------------------------------------------*/
static disk_result_t
mmc_read(uint8_t dev, void *buff, uint32_t sector, uint32_t count)
{
  if(dev >= MMC_CONF_DEV_COUNT || !count) {
    return DISK_RESULT_INVALID_ARG;
  }
  if(!(mmc_status(dev) & DISK_STATUS_INIT)) {
    return DISK_RESULT_NO_INIT;
  }

  if(!(mmc_priv[dev].card_type & CARD_TYPE_BLOCK)) {
    sector *= SECTOR_SIZE;
  }

  if(count == 1) {
    if(mmc_send_cmd(dev, CMD17, sector) == R1_SUCCESS &&
       mmc_rx(dev, buff, SECTOR_SIZE)) {
      count = 0;
    }
  } else if(mmc_send_cmd(dev, CMD18, sector) == R1_SUCCESS) {
    do {
      if(!mmc_rx(dev, buff, SECTOR_SIZE)) {
        break;
      }
      buff = (uint8_t *)buff + SECTOR_SIZE;
      watchdog_periodic();
    } while(--count);
    mmc_send_cmd(dev, CMD12, 0); /* Stop transmission. */
  }
  mmc_select(dev, false);
  return count ? DISK_RESULT_IO_ERROR : DISK_RESULT_OK;
}
/*----------------------------------------------------------------------------*/
static disk_result_t
mmc_write(uint8_t dev, const void *buff, uint32_t sector, uint32_t count)
{
  disk_status_t status;
  card_type_t card_type;

  if(dev >= MMC_CONF_DEV_COUNT || !count) {
    return DISK_RESULT_INVALID_ARG;
  }
  status = mmc_status(dev);
  if(!(status & DISK_STATUS_INIT)) {
    return DISK_RESULT_NO_INIT;
  }
  if(!(status & DISK_STATUS_WRITABLE)) {
    return DISK_RESULT_WR_PROTECTED;
  }

  card_type = mmc_priv[dev].card_type;
  if(!(card_type & CARD_TYPE_BLOCK)) {
    sector *= SECTOR_SIZE;
  }

  if(count == 1) {
    if(mmc_send_cmd(dev, CMD24, sector) == R1_SUCCESS &&
       mmc_tx_block(dev, buff, TOK_RD_SINGLE_WR_START_BLOCK)) {
      count = 0;
    }
  } else {
    if(card_type & CARD_TYPE_SD) {
      mmc_send_cmd(dev, ACMD23, count);
    }
    if(mmc_send_cmd(dev, CMD25, sector) == R1_SUCCESS) {
      do {
        if(!mmc_tx_block(dev, buff, TOK_MULTI_WR_START_BLOCK)) {
          break;
        }
        buff = (uint8_t *)buff + BLOCK_LEN;
        watchdog_periodic();
      } while(--count);
      if(!mmc_tx_block(dev, NULL, TOK_MULTI_WR_STOP_TRAN)) {
        count = 1;
      }
    }
  }
  mmc_select(dev, false);
  return count ? DISK_RESULT_IO_ERROR : DISK_RESULT_OK;
}
/*----------------------------------------------------------------------------*/
static disk_result_t
mmc_ioctl(uint8_t dev, uint8_t cmd, void *buff)
{
  card_type_t card_type;
  disk_result_t res;
  uint8_t csd[CSD_SIZE], sd_status[SD_STATUS_SIZE], au_size;
  uint64_t capacity;
  uint32_t block_size;

  static const uint8_t AU_TO_BLOCK_SIZE[] = {12, 16, 24, 32, 64};

  if(dev >= MMC_CONF_DEV_COUNT) {
    return DISK_RESULT_INVALID_ARG;
  }
  if(!(mmc_status(dev) & DISK_STATUS_INIT)) {
    return DISK_RESULT_NO_INIT;
  }

  card_type = mmc_priv[dev].card_type;
  res = DISK_RESULT_IO_ERROR;

  switch(cmd) {
  case DISK_IOCTL_CTRL_SYNC:
    if(mmc_select(dev, true)) {
      res = DISK_RESULT_OK;
    }
    break;

  case DISK_IOCTL_GET_SECTOR_COUNT:
    if(mmc_send_cmd(dev, CMD9, 0) == R1_SUCCESS && mmc_rx(dev, csd, CSD_SIZE)) {
      capacity = CSD_STRUCTURE(csd) == CSD_STRUCTURE_SD_V2_0 ?
                 CSD_SD_V2_0_CAPACITY(csd) : CSD_SD_V1_0_CAPACITY(csd);
      *(uint32_t *)buff = capacity / SECTOR_SIZE;
      res = DISK_RESULT_OK;
    }
    break;

  case DISK_IOCTL_GET_SECTOR_SIZE:
    *(uint16_t *)buff = SECTOR_SIZE;
    res = DISK_RESULT_OK;
    break;

  case DISK_IOCTL_GET_BLOCK_SIZE:
    if(card_type & CARD_TYPE_SD2) {
      if(mmc_send_cmd(dev, ACMD13, 0) == R1_SUCCESS) { /* Read SD status. */
        mmc_spi_xchg(dev, 0xff);
        if(mmc_rx(dev, sd_status, SD_STATUS_SIZE)) {
          au_size = SD_STATUS_AU_SIZE(sd_status);
          if(au_size) {
            block_size = au_size <= 0xa ? 8192ull << au_size :
                         (uint32_t)AU_TO_BLOCK_SIZE[au_size - 0xb] << 20;
            *(uint32_t *)buff = block_size / SECTOR_SIZE;
            res = DISK_RESULT_OK;
          }
        }
      }
    } else if(mmc_send_cmd(dev, CMD9, 0) == R1_SUCCESS &&
              mmc_rx(dev, csd, CSD_SIZE)) {
      if(card_type & CARD_TYPE_SD1) {
        block_size = (uint32_t)(CSD_SD_V1_0_SECTOR_SIZE(csd) + 1) <<
                     CSD_SD_V1_0_WRITE_BL_LEN(csd);
      } else { /* MMC */
        block_size = (uint32_t)(CSD_MMC_ERASE_GRP_SIZE(csd) + 1) *
                     (CSD_MMC_ERASE_GRP_MULT(csd) + 1) <<
                     CSD_MMC_WRITE_BL_LEN(csd);
      }
      *(uint32_t *)buff = block_size / SECTOR_SIZE;
      res = DISK_RESULT_OK;
    }
    break;

  default:
    res = DISK_RESULT_INVALID_ARG;
    break;
  }
  mmc_select(dev, false);
  return res;
}
/*----------------------------------------------------------------------------*/
const struct disk_driver mmc_driver = {
  .status     = mmc_status,
  .initialize = mmc_initialize,
  .read       = mmc_read,
  .write      = mmc_write,
  .ioctl      = mmc_ioctl
};
/*----------------------------------------------------------------------------*/

/** @} */
