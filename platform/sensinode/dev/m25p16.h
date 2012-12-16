/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Header file for the control of the M25P16 on sensinode N740s.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef M25P16_H_
#define M25P16_H_

/* Instruction Set */
#define M25P16_I_WREN      0x06 /* Write Enable */
#define M25P16_I_WRDI      0x04 /* Write Disable */
#define M25P16_I_RDID      0x9F /* Read Identification */
#define M25P16_I_RDSR      0x05 /* Read Status Register */
#define M25P16_I_WRSR      0x01 /* Write Status Register */
#define M25P16_I_READ      0x03 /* Read Data Bytes */
#define M25P16_I_FAST_READ 0x0B /* Read Data Bytes at Higher Speed */
#define M25P16_I_PP        0x02 /* Page Program */
#define M25P16_I_SE        0xD8 /* Sector Erase */
#define M25P16_I_BE        0xC7 /* Bulk Erase */
#define M25P16_I_DP        0xB9 /* Deep Power-down */
#define M25P16_I_RES       0xAB /* Release from Deep Power-down */

/* Dummy Byte - Used in FAST_READ and RES */
#define M25P16_DUMMY_BYTE  0x00

/* Pins */
#define M25P16_PIN_CLOCK   P1_5
#define M25P16_PIN_SER_I   P1_6
#define M25P16_PIN_SER_O   P1_7

/* Status Register Bits */
#define M25P16_SR_SRWD     0x80 /* Status Register Write Disable */
#define M25P16_SR_BP2      0x10 /* Block Protect 2 */
#define M25P16_SR_BP1      0x08 /* Block Protect 1 */
#define M25P16_SR_BP0      0x04 /* Block Protect 0 */
#define M25P16_SR_BP       0x1C /* All Block Protect Bits */
#define M25P16_SR_WEL      0x02 /* Write Enable Latch */
#define M25P16_SR_WIP      0x01 /* Write in Progress */

/* Do we use READ or FAST_READ to read? Fast by default */
#ifdef M25P16_CONF_READ_FAST
#define M25P16_READ_FAST M25P16_CONF_READ_FAST
#else
#define M25P16_READ_FAST 1
#endif
/*---------------------------------------------------------------------------*/
/** \brief Device Identifier
 *
 * Holds the value of the device identifier, returned by the RDID instruction.
 *
 * After a correct RDID, this structure should hold the following values:
 * man_id = 0x20, mem_type = 0x20, mem_size = 0x15, uid_len = 0x10.
 *
 * UID holds optional Customized Factory Data (CFD) content. The CFD bytes are
 * read-only and can be programmed with customers data upon their request.
 * If the customers do not make requests, the devices are shipped with all the
 * CFD bytes programmed to 0x00.
 */
struct m25p16_rdid {
  uint8_t man_id;    /** Manufacturer ID */
  uint8_t mem_type;  /** Memory Type */
  uint8_t mem_size;  /** Memory Size */
  uint8_t uid_len;   /** Unique ID length */
  uint8_t uid[16];   /** Unique ID */
};
/*---------------------------------------------------------------------------*/
/**
 * \brief Retrieve Block Protect Bits from the status register
 *
 * This macro returns the software block protect status on the device
 * by reading the value of the BP bits ([5:3]) in the Status Register
 */
#define M25P16_BP() (m25p16_rdsr() & M25P16_SR_BP)
/**
 * \brief Check for Write in Progress
 * \retval 1 Write in progress
 * \retval 0 Write not in progress
 *
 * This macro checks if the device is currently in the middle of a write cycle
 * by reading the value of the WIP bit (bit 0) in the Status Register
 */
#define M25P16_WIP() (m25p16_rdsr() & M25P16_SR_WIP)
/**
 * \brief Check for Write-Enable
 * \retval 1 Write enabled
 * \retval 0 Write disabled
 *
 * This macro checks if the device is ready to accept a write instruction
 * by reading the value of the WEL bit (bit 1) in the Status Register
 */
#define M25P16_WEL() (m25p16_rdsr() & M25P16_SR_WEL)
/*---------------------------------------------------------------------------*/
/**
 * \brief Write Enable (WREN) instruction.
 *
 * Completing a WRDI, PP, SE, BE and WRSR
 * resets the write enable latch bit, so this instruction should be used every
 * time before trying to write.
 */
void m25p16_wren();

/**
 * \brief Write Disable (WRDI) instruction
 */
void m25p16_wrdi();

/**
 * \brief Read Identifier (RDID)instruction
 *
 * \param rdid Pointer to a struct which will hold the information returned
 *             by the RDID instruction
 */
void m25p16_rdid(struct m25p16_rdid *rdid);

/**
 * \brief Read Status Register (RDSR) instruction
 *
 * \return Value of the status register
 *
 * Reads and returns the value of the status register on the Flash Chip
 */
uint8_t m25p16_rdsr();

/**
 * \brief Write Status Register (WRSR) instruction
 * \param val Value to be written to the status register
 *
 * This instruction does not afect bits 6, 5, 1 and 0 of the SR.
 */
void m25p16_wrsr(uint8_t val);

/**
 * \brief Read Data Bytes (READ) instruction
 * \param addr 3 byte array holding the read start address. MSB stored in
 *        addr[0] and LSB in addr[2]
 * \param buff Pointer to a buffer to hold the read bytes.
 * \param buff_len Number of bytes to read. buff must be long enough to hold
 *        buff_len bytes
 *
 * The bytes will be inverted after being read, so that a value of 0xFF (empty)
 * in the flash will read as 0x00
 */
void m25p16_read(uint8_t * addr, uint8_t * buff, uint8_t buff_len);

/**
 * \brief Program Page (PP) instruction
 * \param addr 3 byte array holding the write start address. MSB stored in
 *        addr[0] and LSB in addr[2]
 * \param buff Pointer to a buffer with the data to be written
 * \param buff_len Number of bytes to write, Maximum 256 bytes.
 *
 * Write BUFF_LEN bytes stored in BUFF to flash, starting from location
 * ADDR. BUFF_LEN may not exceed 256. ADDR should point to a 3 byte array,
 * with the address MSB stored in position 0 and LSB in position 2
 *
 * If the start address + buff_len exceed page boundaries, the write will
 * wrap to the start of the same page (the page at addr[2:1]).
 *
 * The bytes will be inverted before being written, so that a value of 0xFF
 * will be written as 0x00 (and subsequently correctly read as 0xFF by READ)
 *
 * This function will set the WEL bit on the SR before attempting to write,
 * so the calling function doesn't need to worry about this.
 *
 * This call is asynchronous. It will return before the write cycle has
 * completed. Thus, user software must check the WIP bit Write In Progress)
 * before sending further instructions. This can take up to 5 msecs (typical
 * duration for a 256 byte write is 640 usec)
 */
void m25p16_pp(uint8_t * addr, uint8_t * buff, uint8_t buff_len);

/**
 * \brief Sector Erase (SE) instruction
 * \param s The number of the sector to be erased
 *
 * Delete the entire sector number s, by setting it's contents to all 0xFF
 * (which will read as 0x00 by READ). The flash is broken down into 32 sectors,
 * 64 KBytes each.
 *
 * This function will set the WEL bit on the SR before attempting to write,
 * so the calling function doesn't need to worry about this.
 *
 * This call is asynchronous. It will return before the write cycle has
 * completed. Thus, user software must check the WIP bit Write In Progress)
 * before sending further instructions. This can take up to 3 secs (typical
 * duration 600 msec)
 */
void m25p16_se(uint8_t s); /* Sector Erase */


/**
 * \brief Bulk Erase (SE) instruction
 *
 * Delete the entire memory, by setting it's contents to all 0xFF
 * (which will read as 0x00 by READ).
 *
 * This function will set the WEL bit on the SR before attempting to write,
 * so the calling function doesn't need to worry about this.
 *
 * This call is asynchronous. It will return before the write cycle has
 * completed. Thus, user software must check the WIP bit Write In Progress)
 * before sending further instructions.
 *
 * This instructions takes a very long time to complete and must be used with
 * care. It can take up to 40 secs (yes, secs). A typical duration is 13 secs
 */
void m25p16_be();

/**
 * \brief Deep Power Down (DP) instruction
 *
 * Puts the device into its lowers power consumption mode (This is not the same
 * as the stand-by mode caused by de-selecting the device). While the device
 * is in DP, it will accept no instruction except a RES (Release from DP).
 *
 * This call is asynchronous and will return as soon as the instruction
 * sequence has been written but before the device has actually entered DP
 *
 * Dropping to DP takes 3usec and Resuming from DP takes at least 1.8usec, so
 * this sequence should not be used when the sleep interval is estimated to be
 * short (read as: don't DP then RES then DP repeatedly)
 */
void m25p16_dp();          /* Deep Power down */

/**
 * \brief Release from Deep Power Down (RES) instruction
 *
 * Take the device out of the Deep Power Down mode and bring it to standby.
 * Does not read the electronic signature.
 *
 * This call is synchronous. When it returns the device will be in standby
 * mode.
 *
 * Dropping to DP takes 3usec and Resuming from DP takes at least 1.8usec, so
 * this sequence should not be used when the sleep interval is estimated to be
 * short (read as: don't DP then RES then DP repeatedly)
 */
void m25p16_res();

/**
 * \brief Release from Deep Power Down (RES) and Read Electronic
 * Signature instruction
 *
 * \return The value of the electronic signature. This is provided for backward
 * compatibility and must always be 0x14
 *
 * Take the device out of the Deep Power Down mode and bring it to standby.
 * Does not read the electronic signature.
 *
 * This call is synchronous. When it returns the device will be in standby
 * mode.
 *
 * Dropping to DP takes 3usec and Resuming from DP takes at least 1.8usec, so
 * this sequence should not be used when the sleep interval is estimated to be
 * short (read as: don't DP then RES then DP repeatedly)
 */
uint8_t m25p16_res_res();

#endif /* M25P16_H_ */
