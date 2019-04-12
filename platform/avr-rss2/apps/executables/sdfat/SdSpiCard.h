/* Arduino SdSpiCard Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SdSpiCard Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdSpiCard Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SpiCard_h
#define SpiCard_h
/**
 * \file
 * \brief SdSpiCard class for V2 SD/SDHC cards
 */
#include "SystemInclude.h"
#include "SdFatConfig.h"
#include "SdInfo.h"
#include "SdSpi.h"
//==============================================================================
/**
 * \class SdSpiCard
 * \brief Raw access to SD and SDHC flash memory cards via SPI protocol.
 */
class SdSpiCard {
 public:
  /** typedef for SPI class.  */
#if SD_SPI_CONFIGURATION < 3
  typedef SpiDefault_t m_spi_t;
#else  // SD_SPI_CONFIGURATION < 3
  typedef SdSpiBase m_spi_t;
#endif  // SD_SPI_CONFIGURATION < 3
  /** Construct an instance of SdSpiCard. */
  SdSpiCard() : m_selected(false),
                m_errorCode(SD_CARD_ERROR_INIT_NOT_CALLED), m_type(0) {}
  /** Initialize the SD card.
   * \param[in] spi SPI object.
   * \param[in] chipSelectPin SD chip select pin.
   * \param[in] sckDivisor SPI clock divisor.
   * \return true for success else false.
   */
  bool begin(m_spi_t* spi, uint8_t chipSelectPin = SS,
             uint8_t sckDivisor = SPI_FULL_SPEED);
  /**
   * Determine the size of an SD flash memory card.
   *
   * \return The number of 512 byte data blocks in the card
   *         or zero if an error occurs.
   */
  uint32_t cardSize();
  /** Set the SD chip select pin high, send a dummy byte, and call SPI endTransaction.
   *
   * This function should only be called by programs doing raw I/O to the SD.
   */
  void chipSelectHigh();
  /** Set the SD chip select pin low and call SPI beginTransaction.
   *
   * This function should only be called by programs doing raw I/O to the SD.
   */  
  void chipSelectLow();
  /** Erase a range of blocks.
   *
   * \param[in] firstBlock The address of the first block in the range.
   * \param[in] lastBlock The address of the last block in the range.
   *
   * \note This function requests the SD card to do a flash erase for a
   * range of blocks.  The data on the card after an erase operation is
   * either 0 or 1, depends on the card vendor.  The card must support
   * single block erase.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool erase(uint32_t firstBlock, uint32_t lastBlock);
  /** Determine if card supports single block erase.
   *
   * \return true is returned if single block erase is supported.
   * false is returned if single block erase is not supported.
   */
  bool eraseSingleBlockEnable();
  /**
   *  Set SD error code.
   *  \param[in] code value for error code.
   */
  void error(uint8_t code) {
    m_errorCode = code;
  }
  /**
   * \return code for the last error. See SdSpiCard.h for a list of error codes.
   */
  int errorCode() const {
    return m_errorCode;
  }
  /** \return error data for last error. */
  int errorData() const {
    return m_status;
  }
  /**
   * Check for busy.  MISO low indicates the card is busy.
   *
   * \return true if busy else false.
   */
  bool isBusy();
  /**
   * Read a 512 byte block from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readBlock(uint32_t block, uint8_t* dst);
  /**
   * Read multiple 512 byte blocks from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[in] count Number of blocks to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readBlocks(uint32_t block, uint8_t* dst, size_t count);
  /**
   * Read a card's CID register. The CID contains card identification
   * information such as Manufacturer ID, Product name, Product serial
   * number and Manufacturing date.
   *
   * \param[out] cid pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCID(cid_t* cid) {
    return readRegister(CMD10, cid);
  }
  /**
   * Read a card's CSD register. The CSD contains Card-Specific Data that
   * provides information regarding access to the card's contents.
   *
   * \param[out] csd pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCSD(csd_t* csd) {
    return readRegister(CMD9, csd);
  }
  /** Read one data block in a multiple block read sequence
   *
   * \param[out] dst Pointer to the location for the data to be read.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readData(uint8_t *dst);
  /** Read OCR register.
   *
   * \param[out] ocr Value of OCR register.
   * \return true for success else false.
   */
  bool readOCR(uint32_t* ocr);
  /** Start a read multiple blocks sequence.
   *
   * \param[in] blockNumber Address of first block in sequence.
   *
   * \note This function is used with readData() and readStop() for optimized
   * multiple block reads.  SPI chipSelect must be low for the entire sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readStart(uint32_t blockNumber);
  /** End a read multiple blocks sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readStop();
  /** Return SCK divisor.
   *
   * \return Requested SCK divisor.
   */
  uint8_t sckDivisor() {
    return m_sckDivisor;
  }
  /** \return the SD chip select status, true if slected else false. */
  bool selected() {return m_selected;}
  /** Return the card type: SD V1, SD V2 or SDHC
   * \return 0 - SD V1, 1 - SD V2, or 3 - SDHC.
   */
  int type() const {
    return m_type;
  }
  /**
   * Writes a 512 byte block to an SD card.
   *
   * \param[in] blockNumber Logical block to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeBlock(uint32_t blockNumber, const uint8_t* src);
  /**
   * Write multiple 512 byte blocks to an SD card.
   *
   * \param[in] block Logical block to be written.
   * \param[in] count Number of blocks to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeBlocks(uint32_t block, const uint8_t* src, size_t count);
  /** Write one data block in a multiple block write sequence.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeData(const uint8_t* src);
  /** Start a write multiple blocks sequence.
   *
   * \param[in] blockNumber Address of first block in sequence.
   * \param[in] eraseCount The number of blocks to be pre-erased.
   *
   * \note This function is used with writeData() and writeStop()
   * for optimized multiple block writes.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeStart(uint32_t blockNumber, uint32_t eraseCount);
  /** End a write multiple blocks sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeStop();

 private:
  // private functions
  uint8_t cardAcmd(uint8_t cmd, uint32_t arg) {
    cardCommand(CMD55, 0);
    return cardCommand(cmd, arg);
  }
  uint8_t cardCommand(uint8_t cmd, uint32_t arg);
  bool readData(uint8_t* dst, size_t count);
  bool readRegister(uint8_t cmd, void* buf);
  void type(uint8_t value) {
    m_type = value;
  }
  bool waitNotBusy(uint16_t timeoutMillis);
  bool writeData(uint8_t token, const uint8_t* src);
  void spiBegin(uint8_t chipSelectPin) {
    m_spi->begin(chipSelectPin);
  }
  void spiBeginTransaction(uint8_t spiDivisor) {
    m_spi->beginTransaction(spiDivisor);
  }
  void spiEndTransaction() {
    m_spi->endTransaction();
  }
  uint8_t spiReceive() {
    return m_spi->receive();
  }
  uint8_t spiReceive(uint8_t* buf, size_t n) {
    return m_spi->receive(buf, n);
  }
  void spiSend(uint8_t data) {
    m_spi->send(data);
  }
  void spiSend(const uint8_t* buf, size_t n) {
    m_spi->send(buf, n);
  }
  m_spi_t* m_spi;
  bool m_selected;
  uint8_t m_chipSelectPin;
  uint8_t m_errorCode;
  uint8_t m_sckDivisor;
  uint8_t m_status;
  uint8_t m_type;
};
//==============================================================================
/**
 * \class Sd2Card
 * \brief Raw access to SD and SDHC card using default SPI library.
 */
class Sd2Card : public SdSpiCard {
 public:
  /** Initialize the SD card.
   * \param[in] chipSelectPin SD chip select pin.
   * \param[in] sckDivisor SPI clock divisor.
   * \return true for success else false.
   */
  bool begin(uint8_t chipSelectPin = SS, uint8_t sckDivisor = 2) {
    return SdSpiCard::begin(&m_spi, chipSelectPin, sckDivisor);
  }
  /** Initialize the SD card. Obsolete form.
   * \param[in] chipSelectPin SD chip select pin.
   * \param[in] sckDivisor SPI clock divisor.
   * \return true for success else false.
   */
  bool init(uint8_t sckDivisor = 2, uint8_t chipSelectPin = SS) {
    return begin(chipSelectPin, sckDivisor);
  }

 private:
  bool begin(m_spi_t* spi, uint8_t chipSelectPin = SS,
             uint8_t sckDivisor = SPI_FULL_SPEED) {
    (void)spi;
    (void)chipSelectPin;
    (void)sckDivisor;
    return false;
  }
  SpiDefault_t m_spi;
};
#endif  // SpiCard_h
