
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
 * @file	ScatterWeb.Sd.c
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @version	$Revision: 1.2 $
 *
 * $Id: sd.c,v 1.2 2008/03/28 15:58:43 nvt-se Exp $
 * 
 * Initialisation and basic functions for read and write access
 */

#include "sd_internals.h"
#include "sd.h"

volatile sd_state_t sd_state;



/******************************************************************************
 * @name	Initialization and configuration
 * @{
 */
void
sd_init(void)
{
  // depending on the system global variables may not get initialised on startup
  memset((void *)&sd_state, 0, sizeof (sd_state));

  // initialize io ports
  sd_init_platform();
}


enum sd_init_ret
sd_init_card(sd_cache_t * pCache)
{
  enum sd_init_ret ret = SD_INIT_SUCCESS;
  struct sd_csd csd;
  uint16_t ccc = 0;
  int resetcnt;
  struct sd_response_r3 r3;

  if (!sd_detected()) {
    return SD_INIT_NOCARD;
  }

  if (sd_state.Flags & SD_INITIALIZED) {
    return SD_INIT_SUCCESS;
  }

  // Wait for UART and switch to SPI mode
  if (!uart_lock_wait(UART_MODE_SPI)) {
    return SD_INIT_FAILED;
  }

  // reset card
  resetcnt = _sd_reset(&r3);

  if (resetcnt >= SD_RESET_RETRY_COUNT) {
    ret = SD_INIT_FAILED;
    goto sd_init_card_fail;
  }
  // Test for hardware compatibility
  if ((r3.ocr & SD_V_MASK) != SD_V_MASK) {
    ret = SD_INIT_NOTSUPP;
    goto sd_init_card_fail;
  }
  // Test for software compatibility
  if (!_sd_read_register(&csd, SD_CMD_SEND_CSD, sizeof (struct sd_csd))) {
    ret = SD_INIT_FAILED;
    goto sd_init_card_fail;
  }

  ccc = SD_CSD_CCC(csd);

  if ((ccc & SD_DEFAULT_MINCCC) != SD_DEFAULT_MINCCC) {
    ret = SD_INIT_NOTSUPP;
    goto sd_init_card_fail;
  }


sd_init_card_fail:
  sdspi_unselect();

  uart_unlock(UART_MODE_SPI);
#ifdef LOG_VERBOSE
  LOG_VERBOSE("(sd_init) result:%u, resetcnt:%i OCR:%.8lx, CCC:%.4x",
	      ret, resetcnt, r3.ocr, ccc);
#endif
  if (ret != SD_INIT_SUCCESS) {
    return ret;
  }
  // state
  sd_state.MinBlockLen_bit = 9;
  sd_state.MaxBlockLen_bit = SD_CSD_READ_BL_LEN(csd);
  sd_state.Flags = SD_INITIALIZED;
  if (SD_CSD_READ_PARTIAL(csd)) {
    sd_state.MinBlockLen_bit = 0;
    sd_state.Flags |= SD_READ_PARTIAL;
  }

  if (SD_CSD_WRITE_PARTIAL(csd)) {
    sd_state.Flags |= SD_WRITE_PARTIAL;
  }

  sd_state.BlockLen_bit = 9;
  sd_state.BlockLen = 1 << 9;

#if SD_CACHE
  if (pCache == NULL) {
    return SD_INIT_NOTSUPP;
  }
  sd_state.Cache = pCache;
  _sd_cache_init();
#endif

  return ret;
}


void
sd_flush(void)
{
  if (uart_lock(UART_MODE_SPI)) {
#if SD_WRITE && SD_CACHE
    _sd_cache_flush();
#endif
#if SD_WRITE && SPI_DMA_WRITE
    sd_write_flush();
#endif
    uart_unlock(UART_MODE_SPI);
  }
}


void
sd_close(void)
{
  sd_flush();

  // reset state
  memset((void *)&sd_state, 0, sizeof (sd_state));
}


uint8_t
sd_set_blocklength(const uint8_t blocklength_bit)
{
  uint8_t ret;
  uint8_t arg[4];

  // test if already set
  if (blocklength_bit == sd_state.BlockLen_bit) {
    return sd_state.BlockLen_bit;
  }

  // Wait for UART and switch to SPI mode
  if (!uart_lock_wait(UART_MODE_SPI)) {
    return sd_state.BlockLen_bit;
  }

  ((uint16_t *) arg)[1] = 0;
  ((uint16_t *) arg)[0] = 1 << blocklength_bit;

  // set blocklength command
  if (_sd_send_cmd(SD_CMD_SET_BLOCKLENGTH, SD_RESPONSE_SIZE_R1, arg, NULL)) {
    sd_state.BlockLen_bit = blocklength_bit;
    sd_state.BlockLen = ((uint16_t *) arg)[0];
    ret = blocklength_bit;
  } else {
    ret = SD_BLOCKLENGTH_INVALID;
  }

  // unlock uart
  uart_unlock(UART_MODE_SPI);
  return ret;
}


//@}

///////////////////////////////////////////////////////////////////////////////
// Public functions, Reading
///////////////////////////////////////////////////////////////////////////////
uint16_t
sd_AlignAddress(uint32_t * pAddress)
{
  uint16_t blMask = sd_state.BlockLen - 1;
  uint16_t *lw = (uint16_t *) pAddress;
  uint16_t offset = *lw & blMask;

  *lw &= ~blMask;

  return offset;
}


uint16_t
sd_read_block(void (*const pBuffer), const uint32_t address)
{
  if (!_sd_read_start(SD_CMD_READ_SINGLE_BLOCK, address)) {
    return FALSE;
  }

  sdspi_read(pBuffer, sd_state.BlockLen, TRUE);

  // receive CRC16 and finish
  _sd_read_stop(2);

  return sd_state.BlockLen;
}


#if SD_READ_BYTE
bool
sd_read_byte(void *pBuffer, const uint32_t address)
{
  if (sd_set_blocklength(0) == 0) {
    return sd_read_block(pBuffer, address);
  } else {
    uint32_t blAdr = address;
    uint16_t offset;		// bytes from aligned address to start of first byte to keep
    // align
    offset = sd_AlignAddress(&blAdr);

    // start
    if (!_sd_read_start(SD_CMD_READ_SINGLE_BLOCK, address)) {
      return FALSE;
    }

    // read
    Spi_read(pBuffer, offset + 1, FALSE);

    // done
    _sd_read_stop(sd_state.BlockLen - offset - 1);
  }

  return TRUE;
}
#endif

#if SD_READ_ANY && !SD_CACHE
unsigned int
sd_read(void *pBuffer, unsigned long address, unsigned int size)
{
  unsigned char *p;		// pointer to current pos in receive buffer
  unsigned int offset;		// bytes from aligned address to start of first byte to keep
  unsigned int read_count;	// num bytes to read in one iteration
  bool dump_flag;		// number of bytes to dump in last iteration
  unsigned int num_bytes_read;	// number of bytes read into receive buffer
  unsigned char ret;

  //
  // parameter processing
  //
  if (size == 0) {
    return FALSE;
  }

  // align to block
  offset = sd_AlignAddress(&address);

  if ((offset == 0) && (sd_state.BlockLen == size)) {
    // best case: perfectly block aligned, no chunking
    // -> do shortcut
    return sd_read_block(pBuffer, address);
  }
  // calculate first block
  if (size > sd_state.BlockLen) {
    read_count = sd_state.BlockLen;
  } else {
    read_count = size;
  }
  //
  // Data transfer
  //

  // request data transfer
  ret = _sd_read_start(SD_CMD_READ_SINGLE_BLOCK, address);

  RETF(ret);

  // run to offset
  if (offset) {
    sdspi_read(pBuffer, offset, FALSE);	// dump till offset
    dump_flag = ((read_count + offset) < sd_state.BlockLen);
    if (!dump_flag) {
      read_count = sd_state.BlockLen - offset;	// max bytes to read from first block
    }
  } else {
    dump_flag = (read_count < sd_state.BlockLen);
  }

  //
  // block read loop
  //
  num_bytes_read = 0;
  p = pBuffer;

  do {
    // whole block will be processed
    size -= read_count;		// global counter

    // read to receive buffer
    sdspi_read(p, read_count, TRUE);

    p += read_count;		// increment buffer pointer
    num_bytes_read += read_count;

    // finish block         
    if (dump_flag) {
      // cancel remaining bytes (last iteration)
      _sd_read_stop(sd_state.BlockLen - read_count - offset);
      break;
      // unselect is included in send_cmd
    } else {
      sdspi_idle(2);		// receive CRC16
      if (size != 0) {
	// address calculation for next block
	offset = 0;
	address += sd_state.BlockLen;
	if (size > sd_state.BlockLen) {
	  read_count = sd_state.BlockLen;
	  dump_flag = FALSE;
	} else {
	  read_count = size;
	  dump_flag = TRUE;
	}

	sdspi_unselect();
	ret = _sd_read_start(SD_CMD_READ_SINGLE_BLOCK, address);
	RETF(ret);
      } else {
	// finished
	_sd_read_stop(0);
	break;
      }
    }
  } while (1);

  return num_bytes_read;
}

#endif // SD_READ_ANY

///////////////////////////////////////////////////////////////////////////////
// Public functions, Writing
///////////////////////////////////////////////////////////////////////////////
#if SD_WRITE
enum sd_write_ret
_sd_write_finish(void)
{
  uint16_t r2;
  uint8_t ret;
  enum sd_write_ret result = SD_WRITE_STORE_ERR;
  uint16_t i;

#if SPI_DMA_WRITE
  sdspi_dma_wait();
  sdspi_dma_lock = FALSE;
#endif

  // dummy crc
  sdspi_idle(2);

  // receive data response (ZZS___ 3 bits crc response)
  for (i = 0; i < SD_TIMEOUT_NCR; i++) {
    ret = sdspi_rx();
    if ((ret > 0) && (ret < 0xFF)) {
      while (ret & 0x80) {
	ret <<= 1;
      }
      ret = ((ret & 0x70) == 0x20);
      break;
    }
  }

  // wait for data to be written
  _sd_wait_standby(NULL);
  sdspi_unselect();

  if (ret) {
    // data transfer to sd card buffer was successful
    // query for result of actual write operation
    ret = _sd_send_cmd(SD_CMD_SEND_STATUS, SD_RESPONSE_SIZE_R2, NULL, &r2);
    if (ret && (r2 == 0)) {
      result = SD_WRITE_SUCCESS;
    }
  } else {
    // data transfer to sd card buffer failed
  }

  // unlock uart (locked from every write operation)
  uart_unlock(UART_MODE_SPI);

  return result;

}

enum sd_write_ret
sd_write_flush(void)
{
#if SPI_DMA_WRITE
  if (!sdspi_dma_lock) {
    return SD_WRITE_DMA_ERR;
  } else {
    return _sd_write_finish();
  }
#else
  return SD_WRITE_SUCCESS;
#endif
}


enum sd_write_ret
_sd_write_block(const uint32_t * pAddress, const void *pBuffer, int increment)
{
  uint8_t r1, ret;

  // block write-access on write protection
  if (sd_protected()) {
    return SD_WRITE_PROTECTED_ERR;
  }

  // acquire uart
  if (!uart_lock_wait(UART_MODE_SPI)) {
    return SD_WRITE_INTERFACE_ERR;
  }

  // start write
  SD_LED_WRITE_ON;
  r1 = 0;
  ret = _sd_send_cmd(SD_CMD_WRITE_SINGLE_BLOCK, SD_RESPONSE_SIZE_R1, 
		pAddress, &r1);
  if (!ret | r1) {
    uart_unlock(UART_MODE_SPI);
printf("r1 = %u\n", r1);
    SD_LED_WRITE_OFF;
    return SD_WRITE_COMMAND_ERR;
  }
  // write data
  sdspi_select();
  sdspi_tx(0xFF);
  sdspi_tx(SD_TOKEN_WRITE);
  sdspi_write(pBuffer, sd_state.BlockLen, increment);

  SD_LED_WRITE_OFF;

  // finish write
#if SPI_DMA_WRITE
  sdspi_dma_lock = TRUE;
  return SD_WRITE_SUCCESS;
#else
  return _sd_write_finish();
#endif
}

enum sd_write_ret
sd_set_block(const uint32_t address, const char (*const pChar))
{
  return _sd_write_block(&address, pChar, FALSE);
}


enum sd_write_ret
sd_write_block(const uint32_t address, void const (*const pBuffer))
{
  return _sd_write_block(&address, pBuffer, TRUE);
}


#endif

///////////////////////////////////////////////////////////////////////////////
// Supporting functions
///////////////////////////////////////////////////////////////////////////////


/**
 * @brief	Reads operating condition from SD or MMC card.
 * \internal
 * \Note	Should allow to find out the card type on first run if needed.
 */
inline bool _sd_get_op_cond(struct sd_response_r1 * pResponse)
{
  bool ret;

  // SD style
  ret = _sd_send_cmd(SD_CMD_APP_SECIFIC_CMD, SD_RESPONSE_SIZE_R1, NULL,
		pResponse);

  if (ret) {
    uint32_t arg = SD_V_MASK;
    ret = _sd_send_cmd(SD_ACMD_SEND_OP_COND, SD_RESPONSE_SIZE_R1, &arg,
		   pResponse);
  } else {
    // MMC style init
    ret = _sd_send_cmd(SD_CMD_SEND_OP_COND, SD_RESPONSE_SIZE_R1, NULL, 
		pResponse);
    if (*((uint8_t *) pResponse) & SD_R1_ERROR_MASK) {
      ret = FALSE;
    }
  }

  return ret;
}

/**
 * @brief	Wait for the card to enter standby state
 * \internal
 */
bool
_sd_wait_standby(struct sd_response_r3 * pOpCond)
{
  bool ret;
  int i = SD_TIMEOUT_IDLE;
  struct sd_response_r3 opCond;
  struct sd_response_r3 *pR3 = pOpCond;

  if (pR3 == NULL) {
    pR3 = &opCond;
  }

  sdspi_wait_token(0xFF, 0xFF, 0xFF, SD_TIMEOUT_NCR);

  do {
    ret = _sd_get_op_cond((struct sd_response_r1 *)pR3);
    if (ret && (pR3->r1.in_idle_state == 0)) {
      ret = _sd_send_cmd(SD_CMD_READ_OCR, SD_RESPONSE_SIZE_R3, NULL, pR3);
      if (ret && !SD_OCR_BUSY(pR3->ocr)) {
	return TRUE;
      }
    }
    i--;
  } while (i);

  return FALSE;
}

/**
 * @brief 	Resets the card and (hopefully) returns with the card in standby state
 * \internal
 */
int
_sd_reset(struct sd_response_r3 *pOpCond)
{
  int i;
  bool ret;
  struct sd_response_r1 r1;

  for (i = 0; i < SD_RESET_RETRY_COUNT; i++) {
    ret = _sd_send_cmd(SD_CMD_GO_IDLE_STATE, SD_RESPONSE_SIZE_R1, NULL, &r1);
    if (ret == 0 || r1.illegal_cmd) {
      _sd_send_cmd(SD_CMD_STOP_TRANSMISSION, SD_RESPONSE_SIZE_R1, NULL, &r1);
    } else {
      ret = _sd_wait_standby(pOpCond);
      if (ret) {
	break;
      }
    }
  }

  return i;
}

/**
 * @brief	Used to send all kinds of commands to the card and return the response.
 * \internal
 */
bool
_sd_send_cmd(const uint8_t command,
	     const int response_size,
	     const void *pArg, void (*const pResponse))
{
  uint8_t cmd[6] = {
    0x40, 0, 0, 0, 0, 0x95
  };
  uint8_t data;			// rx buffer
  int i;			// loop counter

#if SD_WRITE && SPI_DMA_WRITE
  sd_write_flush();
#endif

  sdspi_select();
  cmd[0] |= command;
  if (pArg != NULL) {
    cmd[1] = ((uint8_t *) pArg)[3];
    cmd[2] = ((uint8_t *) pArg)[2];
    cmd[3] = ((uint8_t *) pArg)[1];
    cmd[4] = ((uint8_t *) pArg)[0];
  }

  sdspi_write(cmd, 6, 1);

  // wait for start bit
  i = SD_TIMEOUT_NCR;
  do {
    data = sdspi_rx();
    if ((data & 0x80) == 0) {
      goto _sd_send_cmd_response;
    }
  } while (i);

  goto sd_send_cmd_fail;

_sd_send_cmd_response:
  // start bit received, read response with size i
  i = response_size - 1;
  if (pResponse != NULL) {
    // copy response to response buffer
    do {
      ((uint8_t *) pResponse)[i] = data;
      if (i == 0) {
	break;
      }

      data = sdspi_rx();
      i--;
    } while (1);
  } else {
    // receive and ignore response
    sdspi_read(&data, i, 0);
  }

  // done successfully
  sdspi_unselect();

  return TRUE;

sd_send_cmd_fail:
  // failed
  //sdspi_unselect();
  return FALSE;

}

/**
 * @brief	Read Card Register
 * \internal
 */
uint16_t
_sd_read_register(void *pBuffer, uint8_t cmd, uint16_t size)
{
  if (!_sd_read_start(cmd, 0)) {
    return FALSE;
  }

  sdspi_read(pBuffer, size, TRUE);
  _sd_read_stop(2);

  return size;
}

/**
 * @brief	Begin block read operation
 * \internal
 */
bool
_sd_read_start(uint8_t cmd, uint32_t address)
{
  uint8_t r1;
  uint8_t ret;
  uint16_t i;

  // aquire uart
  if (!uart_lock_wait(UART_MODE_SPI)) {
    return FALSE;
  }

  ret = _sd_send_cmd(cmd, SD_RESPONSE_SIZE_R1, &address, &r1);
  if (!ret || r1) {
    goto _sd_read_start_fail;
  }

  // Wait for start bit (0)
  sdspi_select();

  i = sdspi_wait_token(0xFF, 0xFF, SD_TOKEN_READ, SD_TIMEOUT_READ);

  if (i < SD_TIMEOUT_READ) {
    // token received, data bytes follow
    SD_LED_READ_ON;

    /*
       Following code handles error tokens. Since these are currently not used in the
       application they can just be ignored. Anyway this is still useful when debugging.
       else if( (data != 0) && (data & SD_DATA_ERROR_TOKEN_MASK) == data ) {
       // data error token
       sdspi_rx();
       break;
       } */
    return TRUE;
  } else {
    // error or timeout
  }

_sd_read_start_fail:
  sdspi_unselect();
  uart_unlock(UART_MODE_SPI);
  return FALSE;
}

/**
 * @brief	Finished with reading, stop transfer
 * \internal
 */
void
_sd_read_stop(uint16_t count)
{
  // finish block + crc
  if (count) {
    uint8_t dump;

    sdspi_read(&dump, count + 2, FALSE);
    sdspi_unselect();
  }

  SD_LED_READ_OFF;

  // wait for switch to standby mode
  if (!_sd_wait_standby(NULL)) {
    _sd_reset(NULL);
  }

  // unlock uart (locked from _sd_read_start)
  uart_unlock(UART_MODE_SPI);
}
