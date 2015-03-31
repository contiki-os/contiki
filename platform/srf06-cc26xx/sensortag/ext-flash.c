/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-ext-flash
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26xx WinBond W25X20CL Flash
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "ext-flash.h"
#include "ti-lib.h"
#include "board-spi.h"
/*---------------------------------------------------------------------------*/
/* Instruction codes */

#define BLS_CODE_PROGRAM          0x02 /**< Page Program */
#define BLS_CODE_READ             0x03 /**< Read Data */
#define BLS_CODE_READ_STATUS      0x05 /**< Read Status Register */
#define BLS_CODE_WRITE_ENABLE     0x06 /**< Write Enable */
#define BLS_CODE_SECTOR_ERASE     0x20 /**< Sector Erase */
#define BLS_CODE_MDID             0x90 /**< Manufacturer Device ID */

#define BLS_CODE_DP               0xB9 /**< Power down */
#define BLS_CODE_RDP              0xAB /**< Power standby */
/*---------------------------------------------------------------------------*/
/* Erase instructions */

#define BLS_CODE_ERASE_4K         0x20 /**< Sector Erase */
#define BLS_CODE_ERASE_32K        0x52
#define BLS_CODE_ERASE_64K        0xD8
#define BLS_CODE_ERASE_ALL        0xC7 /**< Mass Erase */
/*---------------------------------------------------------------------------*/
/* Bitmasks of the status register */

#define BLS_STATUS_SRWD_BM        0x80
#define BLS_STATUS_BP_BM          0x0C
#define BLS_STATUS_WEL_BM         0x02
#define BLS_STATUS_WIP_BM         0x01

#define BLS_STATUS_BIT_BUSY       0x01 /**< Busy bit of the status register */
/*---------------------------------------------------------------------------*/
/* Part specific constants */

#define BLS_MANUFACTURER_ID       0xEF
#define BLS_DEVICE_ID             0x11

#define BLS_PROGRAM_PAGE_SIZE      256
#define BLS_ERASE_SECTOR_SIZE     4096
/*---------------------------------------------------------------------------*/
/**
 * Clear external flash CSN line
 */
static void
select(void)
{
  ti_lib_gpio_pin_write(BOARD_FLASH_CS, 0);
}
/*---------------------------------------------------------------------------*/
/**
 * Set external flash CSN line
 */
static void
deselect(void)
{
  ti_lib_gpio_pin_write(BOARD_FLASH_CS, 1);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Wait till previous erase/program operation completes.
 * \return Zero when successful.
 */
static int
wait_ready(void)
{
  const uint8_t wbuf[1] = { BLS_CODE_READ_STATUS };

  select();

  /* Throw away all garbages */
  board_spi_flush();

  int ret = board_spi_write(wbuf, sizeof(wbuf));

  if(ret) {
    deselect();
    return -2;
  }

  for(;;) {
    uint8_t buf;
    /* Note that this temporary implementation is not
     * energy efficient.
     * Thread could have yielded while waiting for flash
     * erase/program to complete.
     */
    ret = board_spi_read(&buf, sizeof(buf));

    if(ret) {
      /* Error */
      deselect();
      return -2;
    }
    if(!(buf & BLS_STATUS_BIT_BUSY)) {
      /* Now ready */
      break;
    }
  }
  deselect();
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Put the device in power save mode. No access to data; only
 *        the status register is accessible.
 * \return True when SPI transactions succeed
 */
static bool
power_down(void)
{
  uint8_t cmd;
  bool success;

  cmd = BLS_CODE_DP;
  select();
  success = board_spi_write(&cmd, sizeof(cmd));
  deselect();

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief    Take device out of power save mode and prepare it for normal operation
 * \return   True if the command was written successfully
 */
static bool
power_standby(void)
{
  uint8_t cmd;
  bool success;

  cmd = BLS_CODE_RDP;
  select();
  success = board_spi_write(&cmd, sizeof(cmd));

  if(success) {
    success = wait_ready() == 0;
  }

  deselect();

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * Verify the flash part.
 * @return True when successful.
 */
static bool
verify_part(void)
{
  const uint8_t wbuf[] = { BLS_CODE_MDID, 0xFF, 0xFF, 0x00 };
  uint8_t rbuf[2];
  int ret;

  select();

  ret = board_spi_write(wbuf, sizeof(wbuf));

  if(ret) {
    deselect();
    return false;
  }

  ret = board_spi_read(rbuf, sizeof(rbuf));
  deselect();

  if(ret || rbuf[0] != BLS_MANUFACTURER_ID || rbuf[1] != BLS_DEVICE_ID) {
    return false;
  }
  return true;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Enable write.
 * \return Zero when successful.
 */
static int
write_enable(void)
{
  const uint8_t wbuf[] = { BLS_CODE_WRITE_ENABLE };

  select();
  int ret = board_spi_write(wbuf, sizeof(wbuf));
  deselect();

  if(ret) {
    return -3;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_open()
{
  board_spi_open(4000000, BOARD_SPI_CLK_FLASH);

  /* GPIO pin configuration */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_FLASH_CS);

  /* Default output to clear chip select */
  deselect();

  /* Put the part is standby mode */
  power_standby();

  return verify_part();
}
/*---------------------------------------------------------------------------*/
void
ext_flash_close()
{
  /* Put the part in low power mode */
  power_down();

  board_spi_close();
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_read(size_t offset, size_t length, uint8_t *buf)
{
  uint8_t wbuf[4];

  /* Wait till previous erase/program operation completes */
  int ret = wait_ready();
  if(ret) {
    return false;
  }

  /*
   * SPI is driven with very low frequency (1MHz < 33MHz fR spec)
   * in this implementation, hence it is not necessary to use fast read.
   */
  wbuf[0] = BLS_CODE_READ;
  wbuf[1] = (offset >> 16) & 0xff;
  wbuf[2] = (offset >> 8) & 0xff;
  wbuf[3] = offset & 0xff;

  select();

  if(board_spi_write(wbuf, sizeof(wbuf))) {
    /* failure */
    deselect();
    return false;
  }

  ret = board_spi_read(buf, length);

  deselect();

  return ret == 0;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_write(size_t offset, size_t length, const uint8_t *buf)
{
  uint8_t wbuf[4];
  int ret;
  size_t ilen; /* interim length per instruction */

  while(length > 0) {
    /* Wait till previous erase/program operation completes */
    ret = wait_ready();
    if(ret) {
      return false;
    }

    ret = write_enable();
    if(ret) {
      return false;
    }

    ilen = BLS_PROGRAM_PAGE_SIZE - (offset % BLS_PROGRAM_PAGE_SIZE);
    if(length < ilen) {
      ilen = length;
    }

    wbuf[0] = BLS_CODE_PROGRAM;
    wbuf[1] = (offset >> 16) & 0xff;
    wbuf[2] = (offset >> 8) & 0xff;
    wbuf[3] = offset & 0xff;

    offset += ilen;
    length -= ilen;

    /* Upto 100ns CS hold time (which is not clear
     * whether it's application only inbetween reads)
     * is not imposed here since above instructions
     * should be enough to delay
     * as much. */
    select();

    if(board_spi_write(wbuf, sizeof(wbuf))) {
      /* failure */
      deselect();
      return false;
    }

    if(board_spi_write(buf, ilen)) {
      /* failure */
      deselect();
      return false;
    }
    buf += ilen;
    deselect();
  }

  return true;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_erase(size_t offset, size_t length)
{
  /*
   * Note that Block erase might be more efficient when the floor map
   * is well planned for OTA, but to simplify this implementation,
   * sector erase is used blindly.
   */
  uint8_t wbuf[4];
  size_t i, numsectors;
  size_t endoffset = offset + length - 1;

  offset = (offset / BLS_ERASE_SECTOR_SIZE) * BLS_ERASE_SECTOR_SIZE;
  numsectors = (endoffset - offset + BLS_ERASE_SECTOR_SIZE - 1) / BLS_ERASE_SECTOR_SIZE;

  wbuf[0] = BLS_CODE_SECTOR_ERASE;

  for(i = 0; i < numsectors; i++) {
    /* Wait till previous erase/program operation completes */
    int ret = wait_ready();
    if(ret) {
      return false;
    }

    ret = write_enable();
    if(ret) {
      return false;
    }

    wbuf[1] = (offset >> 16) & 0xff;
    wbuf[2] = (offset >> 8) & 0xff;
    wbuf[3] = offset & 0xff;

    select();

    if(board_spi_write(wbuf, sizeof(wbuf))) {
      /* failure */
      deselect();
      return false;
    }
    deselect();

    offset += BLS_ERASE_SECTOR_SIZE;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_test(void)
{
  bool ret;

  ret = ext_flash_open();
  ext_flash_close();

  return ret;
}
/*---------------------------------------------------------------------------*/
/** @} */
