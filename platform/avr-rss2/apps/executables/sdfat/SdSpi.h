/* Arduino SdSpi Library
 * Copyright (C) 2013 by William Greiman
 *
 * This file is part of the Arduino SdSpi Library
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
 * along with the Arduino SdSpi Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
* \file
* \brief SdSpi class for V2 SD/SDHC cards
*/
#ifndef SdSpi_h
#define SdSpi_h
#include "SystemInclude.h"
#include "SdFatConfig.h"
//------------------------------------------------------------------------------
/**
 * \class SdSpiBase
 * \brief Virtual SPI class for access to SD and SDHC flash memory cards.
 */
class SdSpiBase {
 public:
  /** Initialize the SPI bus.
   *
   * \param[in] chipSelectPin SD card chip select pin.
   */
  virtual void begin(uint8_t chipSelectPin) = 0;
  /** Set SPI options for access to SD/SDHC cards.
   *
   * \param[in] divisor SCK clock divider relative to the system clock.
   */
  virtual void beginTransaction(uint8_t divisor);
  /**
   * End SPI transaction.
   */
  virtual void endTransaction();
  /** Receive a byte.
   *
   * \return The byte.
   */
  virtual uint8_t receive() = 0;
  /** Receive multiple bytes.
   *
   * \param[out] buf Buffer to receive the data.
   * \param[in] n Number of bytes to receive.
   *
   * \return Zero for no error or nonzero error code.
   */
  virtual uint8_t receive(uint8_t* buf, size_t n) = 0;
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  virtual void send(uint8_t data) = 0;
  /** Send multiple bytes.
  *
  * \param[in] buf Buffer for data to be sent.
  * \param[in] n Number of bytes to send.
  */
  virtual void send(const uint8_t* buf, size_t n) = 0;
};
//------------------------------------------------------------------------------
/**
 * \class SdSpi
 * \brief SPI class for access to SD and SDHC flash memory cards.
 */
#if SD_SPI_CONFIGURATION >= 3
class SdSpi : public SdSpiBase {
#else  // SD_SPI_CONFIGURATION >= 3
class SdSpi {
#endif  // SD_SPI_CONFIGURATION >= 3
 public:
  /** Initialize the SPI bus.
   *
   * \param[in] chipSelectPin SD card chip select pin.
   */
  void begin(uint8_t chipSelectPin);
  /** Set SPI options for access to SD/SDHC cards.
   *
   * \param[in] divisor SCK clock divider relative to the system clock.
   */
  void beginTransaction(uint8_t divisor);
  /**
   * End SPI transaction
   */
  void endTransaction();
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive();
  /** Receive multiple bytes.
   *
   * \param[out] buf Buffer to receive the data.
   * \param[in] n Number of bytes to receive.
   *
   * \return Zero for no error or nonzero error code.
   */
  uint8_t receive(uint8_t* buf, size_t n);
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data);
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t n);
  /** \return true - uses SPI transactions */
#if IMPLEMENT_SPI_INTERFACE_SELECTION
  void setSpiIf(uint8_t spiIf) {
    m_spiIf = spiIf;
  }
 private:
  uint8_t m_spiIf;
#endif  // IMPLEMENT_SPI_INTERFACE_SELECTION
};
//------------------------------------------------------------------------------
/**
 * \class SdSpiLib
 * \brief Arduino SPI library class for access to SD and SDHC flash
 *        memory cards.
 */
#if SD_SPI_CONFIGURATION >= 3
class SdSpiLib : public SdSpiBase {
#else  // SD_SPI_CONFIGURATION >= 3
class SdSpiLib {
#endif  // SD_SPI_CONFIGURATION >= 3
 public:
  /** Initialize the SPI bus.
   *
   * \param[in] chipSelectPin SD card chip select pin.
   */
  void begin(uint8_t chipSelectPin) {
    pinMode(chipSelectPin, OUTPUT);
    digitalWrite(chipSelectPin, HIGH);
    SPI.begin();
  }
  /** Set SPI options for access to SD/SDHC cards.
   *
   * \param[in] divisor SCK clock divider relative to the system clock.
   */
  void beginTransaction(uint8_t divisor) {
#if ENABLE_SPI_TRANSACTIONS
    SPI.beginTransaction(SPISettings());
#else  // #if ENABLE_SPI_TRANSACTIONS
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
#endif  // #if ENABLE_SPI_TRANSACTIONS

#ifndef SPI_CLOCK_DIV128
    SPI.setClockDivider(divisor);
#else  // SPI_CLOCK_DIV128
    int v;
    if (divisor <= 2) {
      v = SPI_CLOCK_DIV2;
    } else  if (divisor <= 4) {
      v = SPI_CLOCK_DIV4;
    } else  if (divisor <= 8) {
      v = SPI_CLOCK_DIV8;
    } else  if (divisor <= 16) {
      v = SPI_CLOCK_DIV16;
    } else  if (divisor <= 32) {
      v = SPI_CLOCK_DIV32;
    } else  if (divisor <= 64) {
      v = SPI_CLOCK_DIV64;
    } else {
      v = SPI_CLOCK_DIV128;
    }
    SPI.setClockDivider(v);
#endif  // SPI_CLOCK_DIV128
  }
  /**
   * End SPI transaction.
   */
  void endTransaction() {
#if ENABLE_SPI_TRANSACTIONS
    SPI.endTransaction();
#endif  // ENABLE_SPI_TRANSACTIONS
  }
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {
    return SPI.transfer(0XFF);
  }
  /** Receive multiple bytes.
   *
   * \param[out] buf Buffer to receive the data.
   * \param[in] n Number of bytes to receive.
   *
   * \return Zero for no error or nonzero error code.
   */
#if defined(PLATFORM_ID) && USE_SPI_LIB_DMA
  uint8_t receive(uint8_t* buf, size_t n);
#else  // defined(PLATFORM_ID) && USE_SPI_LIB_DMA
  uint8_t receive(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      buf[i] = SPI.transfer(0XFF);
    }
    return 0;
  }
#endif  // defined(PLATFORM_ID) && USE_SPI_LIB_DMA
  /** Send a byte.
   *
   * \param[in] b Byte to send
   */
  void send(uint8_t b) {
    SPI.transfer(b);
  }
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
#if defined(PLATFORM_ID) && USE_SPI_LIB_DMA
  void send(const uint8_t* buf , size_t n);
#else  // defined(PLATFORM_ID) && USE_SPI_LIB_DMA
  void send(const uint8_t* buf , size_t n) {
    for (size_t i = 0; i < n; i++) {
      SPI.transfer(buf[i]);
    }
  }
#endif  // defined(PLATFORM_ID) && USE_SPI_LIB_DMA
};
//------------------------------------------------------------------------------
#if SD_SPI_CONFIGURATION > 1 || defined(DOXYGEN)
#if defined(PLATFORM_ID)  // Only defined if a Particle device
#include "SoftSPIParticle.h"
#endif  // defined(PLATFORM_ID)
/**
 * \class SdSpiSoft
 * \brief Software SPI class for access to SD and SDHC flash memory cards.
 */
template<uint8_t MisoPin, uint8_t MosiPin, uint8_t SckPin>
class SdSpiSoft : public SdSpiBase {
 public:
  /** Initialize the SPI bus.
   *
   * \param[in] chipSelectPin SD card chip select pin.
   */
  void begin(uint8_t chipSelectPin) {
    pinMode(chipSelectPin, OUTPUT);
    digitalWrite(chipSelectPin, HIGH);
    m_spi.begin();
  }
  /**
   * Initialize hardware SPI - dummy for soft SPI
   * \param[in] divisor SCK divisor - ignored.
   */
  void beginTransaction(uint8_t divisor) {
    (void)divisor;
  }
  /**
   * End SPI transaction - dummy for soft SPI
   */
  void endTransaction() {}
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {
    return m_spi.receive();
  }
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] n Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      buf[i] = receive();
    }
    return 0;
  }
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data) {
    m_spi.send(data);
  }
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
  void send(const uint8_t* buf , size_t n) {
    for (size_t i = 0; i < n; i++) {
      send(buf[i]);
    }
  }

 private:
  SoftSPI<MisoPin, MosiPin, SckPin, 0> m_spi;
};
#endif  // SD_SPI_CONFIGURATION > 1 || defined(DOXYGEN)
//------------------------------------------------------------------------------
#if SD_SPI_CONFIGURATION == 2
/** Default is software SPI. */
typedef SdSpiSoft<SOFT_SPI_MISO_PIN, SOFT_SPI_MOSI_PIN, SOFT_SPI_SCK_PIN>
SpiDefault_t;
#elif SD_SPI_CONFIGURATION == 1 || !SD_HAS_CUSTOM_SPI
/** Default is Arduino library SPI. */
typedef SdSpiLib SpiDefault_t;
#else  // SpiDefault_t
/** Default is custom fast SPI. */
typedef SdSpi SpiDefault_t;
#endif  // SpiDefault_t
//------------------------------------------------------------------------------
// Use of in-line for AVR to save flash.
#ifdef __AVR__
//------------------------------------------------------------------------------
inline void SdSpi::begin(uint8_t chipSelectPin) {
#ifdef __AVR_ATmega328P__
  // Save a few bytes for 328 CPU - gcc optimizes single bit '|' to sbi.
  PORTB |= 1 << 2;  // SS high
  DDRB  |= 1 << 2;  // SS output mode
  DDRB  |= 1 << 3;  // MOSI output mode
  DDRB  |= 1 << 5;  // SCK output mode
#else  // __AVR_ATmega328P__

  // set SS high - may be chip select for another SPI device
  digitalWrite(SS, HIGH);

  // SS must be in output mode even it is not chip select
  pinMode(SS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
#endif  // __AVR_ATmega328P__
  pinMode(chipSelectPin, OUTPUT);
  digitalWrite(chipSelectPin, HIGH);
}
//------------------------------------------------------------------------------
inline void SdSpi::beginTransaction(uint8_t divisor) {
#if ENABLE_SPI_TRANSACTIONS
  SPI.beginTransaction(SPISettings());
#endif  // ENABLE_SPI_TRANSACTIONS
  uint8_t b = 2;
  uint8_t r = 0;

  // See AVR processor documentation.
  for (; divisor > b && r < 7; b <<= 1, r += r < 5 ? 1 : 2) {}
  SPCR = (1 << SPE) | (1 << MSTR) | (r >> 1);
  SPSR = r & 1 ? 0 : 1 << SPI2X;
}
//------------------------------------------------------------------------------
inline void SdSpi::endTransaction() {
#if ENABLE_SPI_TRANSACTIONS
  SPI.endTransaction();
#endif  // ENABLE_SPI_TRANSACTIONS
}
//------------------------------------------------------------------------------
inline uint8_t SdSpi::receive() {
  SPDR = 0XFF;
  while (!(SPSR & (1 << SPIF))) {}
  return SPDR;
}
//------------------------------------------------------------------------------
inline uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  if (n-- == 0) {
    return 0;
  }
  SPDR = 0XFF;
  for (size_t i = 0; i < n; i++) {
    while (!(SPSR & (1 << SPIF))) {}
    uint8_t b = SPDR;
    SPDR = 0XFF;
    buf[i] = b;
  }
  while (!(SPSR & (1 << SPIF))) {}
  buf[n] = SPDR;
  return 0;
}
//------------------------------------------------------------------------------
inline void SdSpi::send(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF))) {}
}
//------------------------------------------------------------------------------
inline void SdSpi::send(const uint8_t* buf , size_t n) {
  if (n == 0) {
    return;
  }
  SPDR = buf[0];
  if (n > 1) {
    uint8_t b = buf[1];
    size_t i = 2;
    while (1) {
      while (!(SPSR & (1 << SPIF))) {}
      SPDR = b;
      if (i == n) {
        break;
      }
      b = buf[i++];
    }
  }
  while (!(SPSR & (1 << SPIF))) {}
}
#endif  // __AVR__
#endif  // SdSpi_h
