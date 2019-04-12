/* Arduino SdFat Library
 * Copyright (C) 2012 by William Greiman
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
#include "SdFat.h"
//------------------------------------------------------------------------------
void SdFatBase::errorHalt(Print* pr) {
  errorPrint(pr);
  SysCall::halt();
}
//------------------------------------------------------------------------------
void SdFatBase::errorHalt(Print* pr, char const* msg) {
  errorPrint(pr, msg);
  SysCall::halt();
}
//------------------------------------------------------------------------------
void SdFatBase::errorPrint(Print* pr) {
  if (!cardErrorCode()) {
    return;
  }
  pr->print(F("SD errorCode: 0X"));
  pr->print(cardErrorCode(), HEX);
  pr->print(F(",0X"));
  pr->println(cardErrorData(), HEX);
}
//------------------------------------------------------------------------------
void SdFatBase::errorPrint(Print* pr, char const* msg) {
  pr->print(F("error: "));
  pr->println(msg);
  errorPrint(pr);
}
//------------------------------------------------------------------------------
void SdFatBase::initErrorHalt(Print* pr) {
  initErrorPrint(pr);
  SysCall::halt();
}
//------------------------------------------------------------------------------
void SdFatBase::initErrorHalt(Print* pr, char const *msg) {
  pr->println(msg);
  initErrorHalt(pr);
}
//------------------------------------------------------------------------------
void SdFatBase::initErrorPrint(Print* pr) {
  if (cardErrorCode()) {
    pr->println(F("Can't access SD card. Do not reformat."));
    if (cardErrorCode() == SD_CARD_ERROR_CMD0) {
      pr->println(F("No card, wrong chip select pin, or SPI problem?"));
    }
    errorPrint(pr);
  } else if (vol()->fatType() == 0) {
    pr->println(F("Invalid format, reformat SD."));
  } else if (!vwd()->isOpen()) {
    pr->println(F("Can't open root directory."));
  } else {
    pr->println(F("No error found."));
  }
}
//------------------------------------------------------------------------------
void SdFatBase::initErrorPrint(Print* pr, char const *msg) {
  pr->println(msg);
  initErrorPrint(pr);
}
#if defined(ARDUINO) || defined(DOXYGEN)
//------------------------------------------------------------------------------
void SdFatBase::errorPrint(Print* pr, const __FlashStringHelper* msg) {
  pr->print(F("error: "));
  pr->println(msg);
  errorPrint(pr);
}
//------------------------------------------------------------------------------
void SdFatBase::errorHalt(Print* pr, const __FlashStringHelper* msg) {
  errorPrint(pr, msg);
  SysCall::halt();
}
//------------------------------------------------------------------------------
void SdFatBase::initErrorHalt(Print* pr, const __FlashStringHelper* msg) {
  pr->println(msg);
  initErrorHalt(pr);
}
//------------------------------------------------------------------------------
void SdFatBase::initErrorPrint(Print* pr, const __FlashStringHelper* msg) {
  pr->println(msg);
  initErrorPrint(pr);
}
#endif  // defined(ARDUINO) || defined(DOXYGEN)
