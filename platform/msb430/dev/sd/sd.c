
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
 * @file	ScatterWeb.sd.c
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @date	Jan 2007
 * @version	0.2
 * 
 * Initialisation and basic functions for read and write access
 */

#include "contiki-msb430.h"
#include "sd_internals.h"
#include "sd.h"

volatile sd_state_t sd_state;

/******************************************************************************
 * @name	Initialization and configuration
 * @{
 */
void
sd_init()
{
  spi_init();
  P5SEL |= 0x0E;		// 00 00 11 10  -> Dout, Din, Clk = peripheral (now done in UART module)
  P5SEL &= ~0x01;		// 00 00 00 01  -> Cs = I/O
  P5OUT |= 0x01;		// 00 00 00 01  -> Cs = High
  P5DIR |= 0x0D;		// 00 00 11 01  -> Dout, Clk, Cs = output
  P5DIR &= ~0x02;		// 00 00 00 10  -> Din = Input
  P2SEL &= ~0x40;		// 11 00 00 00  -> protect, detect = I/O
  P2DIR &= ~0x40;		// 11 00 00 00  -> protect, detect = input     
}

enum sd_init_ret
sd_init_card(sd_cache_t * pCache)
{
  enum sd_init_ret ret = SD_INIT_SUCCESS;
  sd_csd_t csd;
  uint16_t ccc;
  sd_response_r3_t r3;
  uint32_t blocklen, blocknr;

  if (!sd_detected())
    return SD_INIT_FAILED;

  uart_set_mode(UART_MODE_SPI);

  // reset card
  if (!sd_reset()) {
    ret = SD_INIT_FAILED;
    goto sd_init_card_fail;
  }

  // Test for hardware compatibility
  if (!sd_send_cmd(SD_CMD_READ_OCR, SD_RESPONSE_TYPE_R3, NULL, &r3)) {
    ret = SD_INIT_FAILED;
    goto sd_init_card_fail;
  }

  if ((r3.ocr & SD_V_MASK) != SD_V_MASK) {
    ret = SD_INIT_NOTSUPP;
    goto sd_init_card_fail;
  }

  // Test for software compatibility
  if (!sd_read_register(&csd, SD_CMD_SEND_CSD, sizeof (sd_csd_t))) {
    ret = SD_INIT_FAILED;
    goto sd_init_card_fail;
  }

  blocklen = 1UL << SD_CSD_READ_BL_LEN(csd);
  blocknr = ((unsigned long)(SD_CSD_C_SIZE(csd) + 1)) * (1 << (SD_CSD_C_MULT(csd) + 2));

  printf("SD block length: %lu\n", (unsigned long)blocklen);
  printf("SD block number: %lu\n", (unsigned long)blocknr);

  ccc = SD_CSD_CCC(csd);
  if ((ccc & SD_DEFAULT_MINCCC) != SD_DEFAULT_MINCCC) {
    ret = SD_INIT_NOTSUPP;
    goto sd_init_card_fail;
  }

sd_init_card_fail:
  uart_unlock(UART_MODE_SPI);
  if (ret != SD_INIT_SUCCESS)
    return ret;

  // state
  sd_state.MinBlockLen_bit = 9;
  sd_state.MaxBlockLen_bit = SD_CSD_READ_BL_LEN(csd);
  sd_state.Flags = 0;

  if (SD_CSD_READ_PARTIAL(csd)) {
    sd_state.MinBlockLen_bit = 0;
    sd_state.Flags |= SD_READ_PARTIAL;
  }

  if (SD_CSD_WRITE_PARTIAL(csd))
    sd_state.Flags |= SD_WRITE_PARTIAL;
  sd_state.BlockLen_bit = 9;
  sd_state.BlockLen = 1 << 9;

#if SD_CACHE
  if (pCache == NULL)
    return SD_INIT_NOTSUPP;

  sd_state.Cache = pCache;
  sd_cache_init();
#endif

  return ret;
}

void
sd_flush(void)
{
  if (uart_lock(UART_MODE_SPI)) {
#if SD_WRITE && SD_CACHE
    sd_cache_flush();
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
}

uint8_t
sd_set_blocklength(const uint8_t blocklength_bit)
{
  uint8_t ret;
  uint8_t arg[4];

  // test if already set
  if (blocklength_bit == sd_state.BlockLen_bit)
    return sd_state.BlockLen_bit;

  // Wait for UART and switch to SPI mode
  if (!uart_lock(UART_MODE_SPI))
    return sd_state.BlockLen_bit;

  ((uint16_t *) arg)[1] = 0;
  ((uint16_t *) arg)[0] = 1 << blocklength_bit;

  // set blocklength command
  if (sd_send_cmd(SD_CMD_SET_BLOCKLENGTH, SD_RESPONSE_TYPE_R1, arg, NULL)) {
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
sd_align_address(uint32_t * pAddress)
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
  if (!sd_read_start(SD_CMD_READ_SINGLE_BLOCK, address))
    return FALSE;
  spi_read(pBuffer, sd_state.BlockLen, TRUE);

  // receive CRC16 and finish
  sd_read_stop(2);
  return sd_state.BlockLen;
}


#if SD_READ_BYTE
bool
sd_read_byte(void *pBuffer, const uint32_t address)
{
  uint32_t blAdr = address;
  uint16_t offset;		// bytes from aligned address to start of first byte to keep

  if (sd_set_blocklength(0) == 0)
    return sd_read_block(pBuffer, address);

  // align
  offset = sd_align_address(&blAdr);

  // start
  if (!sd_read_start(SD_CMD_READ_SINGLE_BLOCK, address))
    return FALSE;

  // read
  Spi_read(pBuffer, offset + 1, FALSE);

  // done
  sd_read_stop(sd_state.BlockLen - offset - 1);

  return TRUE;
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Public functions, Writing
///////////////////////////////////////////////////////////////////////////////
#if SD_WRITE
uint16_t
sd_write_finish(void)
{
  uint16_t r2;
  uint8_t ret;

#if SPI_DMA_WRITE
  spi_dma_wait();
  spi_dma_lock = FALSE;
#endif

  // dummy crc
  spi_idle(2);

  // receive data response (ZZS___ 3 bits crc response)
  ret = spi_rx();
  while (ret & 0x80)
    ret <<= 1;
  ret = ((ret & 0x70) == 0x20);

  // wait for data to be written
  sd_wait_standby();
  sd_unselect();

  if (ret) {
    // data transfer to sd card buffer was successful
    // query for result of actual write operation
    ret = sd_send_cmd(SD_CMD_SEND_STATUS, SD_RESPONSE_TYPE_R2, NULL, &r2);
    if (ret & (r2 == 0))
      ret = sd_state.BlockLen;
  } else {
    // data transfer to sd card buffer failed
  }

  // unlock uart (locked from every write operation)
  uart_unlock(UART_MODE_SPI);
  return ret;
}

uint16_t
sd_write_flush(void)
{
#if SPI_DMA_WRITE
  if (!spi_dma_lock)
    return 0;
  return sd_write_finish();
#else
  return 0;
#endif
}

uint16_t
sd_write_block_x(const uint32_t * pAddress, const void *pBuffer, bool incPtr)
{
  uint8_t r1, ret;

  // block write-access on write protection
  if (sd_protected())
    return 0;

  // acquire uart
  if (!uart_lock(UART_MODE_SPI))
    return 0;

  // start write
  ret = sd_send_cmd(SD_CMD_WRITE_SINGLE_BLOCK, SD_RESPONSE_TYPE_R1, 
		pAddress, &r1);
  if (!ret | r1) {
    uart_unlock(UART_MODE_SPI);
    return 0;
  }
  // write data
  sd_select();
  spi_tx(0xFF);
  SD_LED_WRITE_ON;
  spi_write(pBuffer, sd_state.BlockLen, SD_TOKEN_WRITE, incPtr);
  SD_LED_WRITE_OFF;

  // finish write
#if SPI_DMA_WRITE
  spi_dma_lock = TRUE;
  return sd_state.BlockLen;
#else
  return sd_write_finish();
#endif
}

uint16_t
sd_set_block(const uint32_t address, const char (*const pChar))
{
  return sd_write_block_x(&address, pChar, FALSE);
}

uint16_t
sd_write_block(const uint32_t address, void const (*const pBuffer))
{
  return sd_write_block_x(&address, pBuffer, TRUE);
}


#endif

///////////////////////////////////////////////////////////////////////////////
// Supporting functions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Activate SD Card on SPI Bus
 * @internal
 */
void
sd_select(void)
{
  P5OUT &= ~0x01;		// Card Select
}

/**
 * @brief	Deactivate SD Card on SPI Bus
 * @internal
 */
void
sd_unselect(void)
{
  UART_WAIT_TXDONE();
  P5OUT |= 0x01;		// Card Deselect
  spi_rx();
}

/**
 * @brief	Wait for the card to enter standby state
 * @internal
 */
bool
sd_wait_standby(void)
{
  sd_response_r1_t r1;
  bool ret;
  int i;

  spi_wait_token(0xFF, SD_TIMEOUT_READ);
  for (i = 0; i < SD_TIMEOUT_IDLE; i++) {
    ret = sd_get_op_cond(&r1);
    if ((ret) && (r1.r1.in_idle_state == 0))
      return TRUE;
  }
  return FALSE;
}


/**
 * @brief 	Resets the card and (hopefully) returns with the card in standby state
 * @internal
 */
bool
sd_reset(void)
{
  int i;
  bool ret;
  sd_response_r1_t r1;

  for (i = 0; i < 4; i++) {
    ret = sd_send_cmd(SD_CMD_GO_IDLE_STATE, SD_RESPONSE_TYPE_R1, NULL, &r1);
    if (ret && r1.r1.illegal_cmd) {
      sd_send_cmd(SD_CMD_STOP_TRANSMISSION, SD_RESPONSE_TYPE_R1, NULL, &r1);
      ret = sd_send_cmd(SD_CMD_GO_IDLE_STATE, SD_RESPONSE_TYPE_R1, NULL, &r1);
    }
    ret = sd_wait_standby();
    if (ret)
      return TRUE;
  }
  return FALSE;
}


/**
 * @brief	Reads operating condition from SD or MMC card.
 * @internal
 * @Note	Should allow to find out the card type on first run if needed.
 */
bool
sd_get_op_cond(sd_response_r1_t * pResponse)
{
  bool ret;

  // SD style
  ret =
    sd_send_cmd(SD_CMD_APP_SECIFIC_CMD, SD_RESPONSE_TYPE_R1, NULL, pResponse);
  if (ret)
    ret =
      sd_send_cmd(SD_ACMD_SEND_OP_COND, SD_RESPONSE_TYPE_R1, NULL, pResponse);

  // MMC style init
  if (!ret)
    ret =
      sd_send_cmd(SD_CMD_SEND_OP_COND, SD_RESPONSE_TYPE_R1, NULL, pResponse);
  if (*((uint8_t *) pResponse) & SD_R1_ERROR_MASK)
    return FALSE;
  return ret;
}


/**
 * @brief	Used to send all kinds of commands to the card and return the response.
 * @internal
 */
bool
sd_send_cmd(const uint8_t command, const uint8_t response_type,
	    const void *pArg, void (*const pResponse))
{
  uint8_t data;			// rx buffer
  int i;			// loop counter

#if SD_WRITE && SPI_DMA_WRITE
  sd_write_flush();
#endif
  sd_select();

  // send command (1 byte)
  spi_tx(0x40 | command);

  // send argument (4 bytes)
  if (pArg == NULL) {
    for (i = 0; i < 4; i++)
      spi_tx(0x00);
  } else {
    for (i = 3; i >= 0; i--)
      spi_tx(((uint8_t *) pArg)[i]);
  }

  // send CRC matching CMD0 (1 byte)
  spi_tx(0x95);

  // wait for start bit
  for (i = 0; i < SD_TIMEOUT_NCR; i++) {
    data = spi_rx();
    if ((data & 0x80) == 0)
      goto sd_send_cmd_response;
  }

  // timeout ( i >= SD_TIMEOUT_NCR )
  // failed
  sd_unselect();
  return FALSE;

sd_send_cmd_response:
  // start bit received, read response with size i
  i = response_type - 1;
  if (pResponse != NULL) {

    // copy response to response buffer
    do {
      ((uint8_t *) pResponse)[i] = data;
      if (i == 0)
	break;
      data = spi_rx();
      i--;
    } while (1);
  } else {
    // receive and ignore response
    spi_idle(i);
  }

  // done successfully
  sd_unselect();
  return TRUE;
}


/**
 * @brief	Read Card Register
 * @internal
 */
uint16_t
sd_read_register(void *pBuffer, uint8_t cmd, uint16_t size)
{
  if (!sd_read_start(cmd, 0)) {
    return FALSE;
  }
  spi_read(pBuffer, size, TRUE);
  sd_read_stop(2);

  return size;
}


/**
 * @brief	Begin block read operation
 * @internal
 */
bool
sd_read_start(uint8_t cmd, uint32_t address)
{
  uint8_t r1;
  uint8_t ret;

  if (!uart_lock(UART_MODE_SPI)) {
    return FALSE;
  }
  ret = sd_send_cmd(cmd, SD_RESPONSE_TYPE_R1, &address, &r1);
  if (!ret || r1) {
    goto sd_read_start_fail;
  }

  // Wait for start bit (0)
  ret = sd_read_wait();
  if (ret)
    return TRUE;

sd_read_start_fail:
  uart_unlock(UART_MODE_SPI);
  return FALSE;
}


/**
 * @brief	Wait for beginning of data
 * @internal
 */
bool
sd_read_wait(void)
{
  uint16_t i;			// loop counter
  uint8_t data;			// rx buffer

  sd_select();
  for (i = 0; i < SD_TIMEOUT_READ; i++) {
    data = spi_rx();
    if (data == SD_TOKEN_READ) {

      // token received, data bytes follow
      SD_LED_READ_ON;
      return TRUE;
    }

#if 0
    /*
     * The following code handles error tokens. Since these are currently 
     * not used in the application they can just be ignored. Anyway this 
     * is still useful when debugging.
     */

     else if ((data != 0) && (data & SD_DATA_ERROR_TOKEN_MASK) == data) {
       // data error token
       spi_rx();
       break;
     }
#endif
  }

  // error or timeout
  sd_unselect();
  return FALSE;
}


/**
 * @brief	Finished with reading, stop transfer
 * @internal
 */
void
sd_read_stop(uint16_t count)
{

  // finish block + crc
  if (count) {
    uint8_t dump;

    spi_read(&dump, count + 2, FALSE);
    sd_unselect();
  }
  SD_LED_READ_OFF;

  // wait for switch to standby mode
  if (!sd_wait_standby())
    sd_reset();

  // unlock uart (locked from sd_read_start)
  uart_unlock(UART_MODE_SPI);
}
