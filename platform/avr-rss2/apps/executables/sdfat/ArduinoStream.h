/* FatLib Library
 * Copyright (C) 2013 by William Greiman
 *
 * This file is part of the FatLib Library
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
 * along with the FatLib Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef ArduinoStream_h
#define ArduinoStream_h
/**
 * \file
 * \brief ArduinoInStream and ArduinoOutStream classes
 */
#include "FatLibConfig.h"
#if ENABLE_ARDUINO_FEATURES
#include "SysCall.h"
#include "bufstream.h"
//==============================================================================
/**
 * \class ArduinoInStream
 * \brief Input stream for Arduino Stream objects
 */
class ArduinoInStream : public ibufstream {
 public:
  /**
   * Constructor
   * \param[in] hws hardware stream
   * \param[in] buf buffer for input line
   * \param[in] size size of input buffer
   */
  ArduinoInStream(Stream &hws, char* buf, size_t size) {
    m_hw = &hws;
    m_line = buf;
    m_size = size;
  }
  /** read a line. */
  void readline() {
    size_t i = 0;
    uint32_t t;
    m_line[0] = '\0';
    while (!m_hw->available()) {
      SysCall::yield();
    }

    while (1) {
      t = millis();
      while (!m_hw->available()) {
        if ((millis() - t) > 10) {
          goto done;
        }
      }
      if (i >= (m_size - 1)) {
        setstate(failbit);
        return;
      }
      m_line[i++] = m_hw->read();
      m_line[i] = '\0';
    }
done:
    init(m_line);
  }

 protected:
  /** Internal - do not use.
   * \param[in] off
   * \param[in] way
   * \return true/false.
   */
  bool seekoff(off_type off, seekdir way) {
    (void)off;
    (void)way;
    return false;
  }
  /** Internal - do not use.
   * \param[in] pos
   * \return true/false.
   */
  bool seekpos(pos_type pos) {
    (void)pos;
    return false;
  }

 private:
  char *m_line;
  size_t m_size;
  Stream* m_hw;
};
//==============================================================================
/**
 * \class ArduinoOutStream
 * \brief Output stream for Arduino Print objects
 */
class ArduinoOutStream : public ostream {
 public:
  /** constructor
   *
   * \param[in] pr Print object for this ArduinoOutStream.
   */
  explicit ArduinoOutStream(Print& pr) : m_pr(&pr) {}

 protected:
  /// @cond SHOW_PROTECTED
  /**
   * Internal do not use
   * \param[in] c
   */
  void putch(char c) {
    if (c == '\n') {
      m_pr->write('\r');
    }
    m_pr->write(c);
  }
  void putstr(const char* str) {
    m_pr->write(str);
  }
  bool seekoff(off_type off, seekdir way) {
    (void)off;
    (void)way;
    return false;
  }
  bool seekpos(pos_type pos) {
    (void)pos;
    return false;
  }
  bool sync() {
    return true;
  }
  pos_type tellpos() {
    return 0;
  }
  /// @endcond
 private:
  ArduinoOutStream() {}
  Print* m_pr;
};
#endif  // ENABLE_ARDUINO_FEATURES
#endif  // ArduinoStream_h
