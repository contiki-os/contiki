/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit@wsystem.com>
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
 * \addtogroup dev
 * @{
 *
 * \defgroup disk Disk device drivers
 *
 * Documentation for all the disk device drivers.
 * @{
 *
 * \file
 * Header file defining the disk device driver API.
 */
#ifndef DISK_H_
#define DISK_H_

#include <stdint.h>

/** Disk status flags. */
typedef enum {
  DISK_STATUS_INIT     = 0x01, /**< Device initialized and ready to work */
  DISK_STATUS_DISK     = 0x02, /**< Medium present in the drive */
  DISK_STATUS_WRITABLE = 0x04  /**< Writable medium */
} disk_status_t;

/** Generic disk I/O control commands. */
typedef enum {
  DISK_IOCTL_CTRL_SYNC,        /**< Synchronize the cached writes to persistent storage */
  DISK_IOCTL_GET_SECTOR_COUNT, /**< Get the sector count through the \c uint32_t pointed to by \c buff */
  DISK_IOCTL_GET_SECTOR_SIZE,  /**< Get the sector size through the \c uint16_t pointed to by \c buff */
  DISK_IOCTL_GET_BLOCK_SIZE,   /**< Get the erase block size (in sectors) through the \c uint32_t pointed to by \c buff */
  DISK_IOCTL_CTRL_TRIM         /**< Trim the sector range within the \c uint32_t boundaries pointed to by \c buff */
} disk_ioctl_t;

/** Disk access result codes. */
typedef enum {
  DISK_RESULT_OK,           /**< Success */
  DISK_RESULT_IO_ERROR,     /**< Unrecoverable I/O error */
  DISK_RESULT_WR_PROTECTED, /**< Write-protected medium */
  DISK_RESULT_NO_INIT,      /**< Device not initialized */
  DISK_RESULT_INVALID_ARG   /**< Invalid argument */
} disk_result_t;

/** Disk driver API structure. */
struct disk_driver {
  /** Get device status. */
  disk_status_t (*status)(uint8_t dev);

  /** Initialize device. */
  disk_status_t (*initialize)(uint8_t dev);

  /** Read sector(s). */
  disk_result_t (*read)(uint8_t dev, void *buff, uint32_t sector,
                        uint32_t count);

  /** Write sector(s). */
  disk_result_t (*write)(uint8_t dev, const void *buff, uint32_t sector,
                         uint32_t count);

  /** Control device-specific features. */
  disk_result_t (*ioctl)(uint8_t dev, uint8_t cmd, void *buff);
};

#endif /* DISK_H_ */

/**
 * @}
 * @}
 */
