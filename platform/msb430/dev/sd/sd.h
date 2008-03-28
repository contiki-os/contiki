/*
Copyright 2007, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universität Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2007
*/

/**
 * @file	ScatterWeb.Sd.h
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library, Public interface
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @version	$Revision: 1.3 $
 *
 * $Id: sd.h,v 1.3 2008/03/28 23:03:05 nvt-se Exp $
 */

/**
 * @ingroup	libsd
 * @{
 */

#ifndef __SD_H__
#define __SD_H__

#define SD_BLOCKLENGTH_INVALID		0xFF
#define	SD_WRITE_BLOCKLENGTH_BIT	9
#define SD_WRITE_BLOCKLENGTH		0x200


/******************************************************************************
 * @name	Setup, initialisation, configuration
 * @{
 */

/**
 * @brief SD Flags
 * @see sd_state_t
 * @{
 */
enum sd_state_flags {
  SD_READ_PARTIAL = 0x80,
  SD_WRITE_PARTIAL = 0x40,
  SD_INITIALIZED = 0x01
};

/** @} */

#ifdef __TI_COMPILER_VERSION__
#pragma pack(1)
#endif

/**
 * @brief	Card Identification Register
 */
#ifndef __TI_COMPILER_VERSION__
__attribute__ ((packed))
#endif
#if defined(__MSP430GCC__)
  __attribute__ ((packed))
#endif
     struct sd_cid {
       uint8_t mid;
       char oid[2];
       char pnm[5];
       uint8_t revision;
       uint32_t psn;
         uint16_t:4;
       uint16_t mdt:12;
       uint8_t crc7:7;
         uint8_t:1;
     };
#ifdef __TI_COMPILER_VERSION__
#pragma pack()
#endif

/**
 * @name	Card Specific Data register
 * @{
 */
     struct sd_csd {
       uint8_t raw[16];
     };
#define SD_CSD_READ_BL_LEN(csd)		(( csd ).raw[5] & 0x0F)	// uchar : 4
#define SD_CSD_READ_PARTIAL(csd)	(( csd ).raw[ 6] & 0x80)	// bool
#define SD_CSD_CCC(csd)				( (( csd ).raw[4]<<4) | (( csd ).raw[5]>>4) )	// uchar
#define SD_CSD_WRITE_PARTIAL(csd)	(( csd ).raw[13] & 0x04)	// bool
#define SD_CSD_C_SIZE(csd)			( ((( csd ).raw[6] & 0x03)<<10) | (( csd ).raw[7]<<2) | (( csd ).raw[8]>>6) )	// uint : 12
#define SD_CSD_C_MULT(csd)			( ((( csd ).raw[9] & 0x03)<<1) | (( csd ).raw[10]>>7) )	// uchar : 4

/** @} */

/// Card access library state
#define SD_CACHE_LOCKED			0x01
#define SD_CACHE_DIRTY			0x02

typedef struct {
  char buffer[SD_WRITE_BLOCKLENGTH];
  uint32_t address;
  uint8_t state;
} sd_cache_t;

typedef struct {
  uint16_t MinBlockLen_bit:4;	///< minimum supported blocklength
  uint16_t MaxBlockLen_bit:4;	///< maximum supported blocklength
  uint16_t Flags:8;	///< feature flags
  uint8_t BlockLen_bit;	///< currently selected blocklength as bit value (n where BlockLen is 2^n)
  uint16_t BlockLen;	///< currently selected blocklength for reading and writing
#if SD_CACHE
  sd_cache_t *Cache;
#endif
} sd_state_t;

extern volatile sd_state_t sd_state;	///< Card access library state

/**
 * @brief 	Library initialisation
 */
void sd_init(void);

/**
 * @brief	Setup ports for sd card communication
 * 
 * This function needs to be called once before any other library function.
 * It invokes ::Spi_enable to configure UART1 for SPI communication.
 * 
 * If you need to reconfigure the UART for other operations only call
 * ::Spi_enable to return to SPI mode. ::sd_setup needs to be run only once.
 */
     void sd_init_platform(void);

/**
 * @brief	Return value of ::sd_init function
 */
enum sd_init_ret {
  SD_INIT_SUCCESS = 0,
  SD_INIT_NOCARD = 1,
  SD_INIT_FAILED = 2,
  SD_INIT_NOTSUPP = 3
};

/**
 * @brief	Return value of write functions
 * @see ::sd_write, ::sd_write_block
 */
enum sd_write_ret {
  SD_WRITE_SUCCESS = 0,	///< writing successfull
  SD_WRITE_PROTECTED_ERR = 1,	///< card write protected
  SD_WRITE_INTERFACE_ERR = 2,	///< error in UART SPI interface
  SD_WRITE_COMMAND_ERR = 3,	///< error in write command or command arguments (e.g. target address)
  SD_WRITE_STORE_ERR = 4,	///< storing written data to persistant memory on card failed
  SD_WRITE_DMA_ERR = 5
};

/**
 * @brief	Initialize card and state
 * @return	one of sd_init_ret
 * 
 * Initializes the memory card, checks supported voltage range and
 * functionality. Initializes the global state struct sd_state.
 * Should be invoked once immediately after ::sd_setup.
 */
enum sd_init_ret sd_init_card(sd_cache_t * pCache);

/**
 * @brief	Last operation to call when finished with using the card.
 */
void sd_close(void);

/**
 * @brief SD Card physically present?
 * @return	if no card present returns 0
 */
#define sd_detected()	((P2IN & 0x40) == 0)

/**
 * @brief SD Card locked by switch?
 * @return	if card is not locked returns 0
 */
#define	sd_protected()	((P2IN & 0x80) != 0)

/**
 * @brief	Set blocklength to 2 ^ n
 * 
 * Tries to set the card's blocklength for reading and writing to any
 * 2^n value, where n shall be between 0 and 11.
 * Be aware that a card may or may not support different blocksizes.
 * 
 * Since all read and write operations have to use blockaligned addresses
 * and need to process complete blocks always try to use the optimal blocksize
 * and let ::sd_read do the rest. If the blocklength is already set to the new
 * value nothing is done.
 * 
 * \Note
 * The default blocklength is 512 (n=9). SD Cards usually support partial blocks
 * with a blocklength of 1 to 512 (n=0 to n=9), MMC cards don't.
 * Large cards support blocklength up to 2048 (n=11).
 * The supported range of blocklengths can be read as n-value from 
 * sd_state.BlockLenMin_bit and sd_state.BlockLneMax_bit. The current value
 * is available as n-value in sd_state.Blocsd_state.BlockLen_bit and as byte-value
 * in sd_state.BlockLen.
 * 
 * @param[in]	blocklength_bit		blocklength as n-value of 2^n
 * 
 * @return	Returns blocklength_bit if set operation was successful or
 * 			SD_BLOCKLENGTH_INVALID otherwise.
 * 
 */
     uint8_t sd_set_blocklength(const uint8_t blocklength_bit);

/**
 * @brief			Align byte address to current blocklength
 * @param[in,out]	pAddress	address to align, will be modified to be block aligned
 * @return			Offset from aligned address to original address
 */
uint16_t sd_AlignAddress(uint32_t * pAddress);

/**
 * @brief	Read one complete block from a block aligned address into buffer
 * 
 * This function reads a single block of the currently set blocklength
 * from a block aligned address.
 * 
 * \Note: If address is not block aligned the card will respond with an error
 * and no bytes will be read.
 * 
 * @param[out]	pBuffer		Pointer to a buffer to which data is read. It should be least
 * 							1 byte large
 * @param[in]	address		The address of the first byte that shall be read to pBuffer
 * 
 * @return		Number of bytes read (should always be = sd_state.BlockLen)
 */
uint16_t sd_read_block(void (*const pBuffer), const uint32_t address);

#if SD_READ_BYTE
/**
  * @brief	Read one byte from any address
  * This function reads a single byte from any address. It is optimized for best speed
  * at any blocklength.
  * \Note: blocklength is modified
  * 
  * @param[out]	pBuffer		Pointer to a buffer to which data is read. It should be least
  * 							1 byte large
  * @param[in]	address		The address of the byte that shall be read to pBuffer
  * 
  * @return	Number of bytes read (usually 1)
  */
bool sd_read_byte(void *pBuffer, const uint32_t address);
#endif

#if SD_WRITE
 /**
  * @brief Write one complete block at a block aligned address from buffer to card
  * 
  * @param[in]	address		block aligned address to write to
  * @param[in]	pBuffer		pointer to buffer with a block of data to write
  * @return		result code (see enum #sd_write_ret)
  * 
  * \Note
  * Only supported block size for writing is usually 512 bytes.
  */
enum sd_write_ret sd_write_block(const uint32_t address,
				 void const (*const pBuffer));

 /**
  * @brief		Fill one complete block at a block aligned address with
  * 				a single character.
  * 
  * @param[in]	address		block aligned address to write to
  * @param[in]	pChar		pointer to buffer with a character to write
  * @return		result code (see enum #sd_write_ret)
  * 
  * @note		Use this for settings blocks to 0x00. 
  *				Only supported block size for writing is usually 512 bytes.
  */
enum sd_write_ret sd_set_block(const uint32_t address,
			       const char (*const pChar));

 /**
  * @brief		Flush the DMA write buffer
  * 
  * Wait for a running DMA write operation to finish
  */
enum sd_write_ret sd_write_flush(void);
#endif

#if SD_CACHE
#define	SD_WAIT_LOCK(x)	while( x ->state & SD_CACHE_LOCKED ) { _NOP(); }
#define SD_GET_LOCK(x)	do { while( x ->state & SD_CACHE_LOCKED ) { _NOP(); }; x ->state |= SD_CACHE_LOCKED; } while(0)
#define SD_FREE_LOCK(x)	do { x ->state &= ~SD_CACHE_LOCKED; } while(0)

 /**
  * @brief	Flush the sd cache
  * 
  * Writes back the cache buffer, if it has been modified. Call this if
  * a high level operation has finished and you want to store all data
  * persistantly. The write back operation does not use timers.
  */
void sd_cache_flush(void);

 /**
  * @brief	Read a block into the cache buffer
  * @internal
  * 
  * You won't usually need this operation.
  */
sd_cache_t *sd_cache_read_block(const uint32_t * blAdr);
#endif

/**
 * @brief		Erase (clear) blocks of data
 * @param[in]	address		Starting address of first block to erase
 * @param[in]	numBlocks	Number of blocks to erase (starting at address)
 * @return		true if successful
 * @note		Data can only be erased in whole blocks. All bytes will be set
 *				predifined character (see CSD). Common cards use 0xFF.
 */
     bool sd_erase_blocks(const uint32_t address, const uint16_t numBlocks);

/**
 * @brief		Read card identification register (CID)
 * @param[out]	pCID	Pointer to buffer for CID
 * @return		Number of bytes read (= size of CID) or 0 if failed
 *
 * @see			::sd_cid_t
 */
     uint16_t sd_read_cid(struct sd_cid *pCID);

/**
 * @brief	Read size of card
 * @return	Number of bytes available
 *
 * Reads the number of bytes in the card memory from the CSD register.
 */
     uint32_t sd_get_size(void);

#if SD_READ_ANY
 /**
  * @brief	Read any number of bytes from any address into buffer
  * 
  * @param[out]	pBuffer		Pointer to a buffer to which data is read. It should be least
  * 							size bytes large
  * @param[in]	address		The address of the first byte that shall be read to pBuffer
  * @param[in]	size		Number of bytes which shall be read starting at address	 */
     uint16_t sd_read(void *pBuffer, uint32_t address, uint16_t size);
#endif

#endif /*__SD_H__*/

/** @} */
