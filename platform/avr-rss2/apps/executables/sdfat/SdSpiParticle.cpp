/* Arduino SdFat Library
 * Copyright (C) 2016 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
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
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "SdSpi.h"
#if defined(PLATFORM_ID)

static  uint32_t bugDelay = 0;  // fix for SPI DMA bug.

static volatile bool SPI_DMA_TransferCompleted = false;

static SPIClass* const spiPtr[] = {
  &SPI
#if Wiring_SPI1
  , &SPI1
#if  Wiring_SPI2
  , &SPI2
#endif  // Wiring_SPI2
#endif  // Wiring_SPI1
};
#if SPI_INTERFACE_COUNT == 1
const uint8_t m_spiIf = 0;
#endif
//-----------------------------------------------------------------------------
void SD_SPI_DMA_TransferComplete_Callback(void) {
    SPI_DMA_TransferCompleted = true;
}
//------------------------------------------------------------------------------
void SdSpi::begin(uint8_t chipSelectPin) {
  spiPtr[m_spiIf]->begin(chipSelectPin);
}
//------------------------------------------------------------------------------
void SdSpi::beginTransaction(uint8_t divisor) {
  spiPtr[m_spiIf]->setBitOrder(MSBFIRST);
  spiPtr[m_spiIf]->setDataMode(SPI_MODE0);
#ifndef SPI_CLOCK_DIV128
  spiPtr[m_spiIf]->setClockDivider(divisor);
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
  spiPtr[m_spiIf]->setClockDivider(v);
#endif  // SPI_CLOCK_DIV128
  // delay for SPI transfer done callback too soon bug.
  bugDelay = 24*divisor*(1 + m_spiIf)/60;
}
//-----------------------------------------------------------------------------
void SdSpi::endTransaction() {
}
//-----------------------------------------------------------------------------
/** SPI receive a byte */
uint8_t SdSpi::receive() {
  return spiPtr[m_spiIf]->transfer(0xFF);
}
//-----------------------------------------------------------------------------
uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  SPI_DMA_TransferCompleted = false;
  spiPtr[m_spiIf]->transfer(0, buf, n, SD_SPI_DMA_TransferComplete_Callback);
  while (!SPI_DMA_TransferCompleted) {}
  if (bugDelay) {
    delayMicroseconds(bugDelay);
  }
  return 0;
}
//-----------------------------------------------------------------------------
/** SPI send a byte */
void SdSpi::send(uint8_t b) {
  spiPtr[m_spiIf]->transfer(b);
}
//-----------------------------------------------------------------------------
void SdSpi::send(const uint8_t* buf , size_t n) {
  SPI_DMA_TransferCompleted = false;

  spiPtr[m_spiIf]->transfer(const_cast<uint8_t*>(buf), 0, n,
                            SD_SPI_DMA_TransferComplete_Callback);

  while (!SPI_DMA_TransferCompleted) {}
  if (bugDelay) {
    delayMicroseconds(bugDelay);
  }
}
#endif  // defined(PLATFORM_ID)
