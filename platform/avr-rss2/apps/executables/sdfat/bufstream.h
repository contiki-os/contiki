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
#ifndef bufstream_h
#define bufstream_h
/**
 * \file
 * \brief \ref ibufstream and \ref obufstream classes
 */
#include <string.h>
#include "iostream.h"
//==============================================================================
/**
 * \class ibufstream
 * \brief parse a char string
 */
class ibufstream : public istream {
 public:
  /** Constructor */
  ibufstream() : m_buf(0), m_len(0) {}
  /** Constructor
   * \param[in] str pointer to string to be parsed
   * Warning: The string will not be copied so must stay in scope.
   */
  explicit ibufstream(const char* str) {
    init(str);
  }
  /** Initialize an ibufstream
   * \param[in] str pointer to string to be parsed
   * Warning: The string will not be copied so must stay in scope.
   */
  void init(const char* str) {
    m_buf = str;
    m_len = strlen(m_buf);
    m_pos = 0;
    clear();
  }

 protected:
  /// @cond SHOW_PROTECTED
  int16_t getch() {
    if (m_pos < m_len) {
      return m_buf[m_pos++];
    }
    setstate(eofbit);
    return -1;
  }
  void getpos(FatPos_t *pos) {
    pos->position = m_pos;
  }
  bool seekoff(off_type off, seekdir way) {
    (void)off;
    (void)way;
    return false;
  }
  bool seekpos(pos_type pos) {
    if (pos < m_len) {
      m_pos = pos;
      return true;
    }
    return false;
  }
  void setpos(FatPos_t *pos) {
    m_pos = pos->position;
  }
  pos_type tellpos() {
    return m_pos;
  }
  /// @endcond
 private:
  const char* m_buf;
  size_t m_len;
  size_t m_pos;
};
//==============================================================================
/**
 * \class obufstream
 * \brief format a char string
 */
class obufstream : public ostream {
 public:
  /** constructor */
  obufstream() : m_in(0) {}
  /** Constructor
   * \param[in] buf buffer for formatted string
   * \param[in] size buffer size
   */
  obufstream(char *buf, size_t size) {
    init(buf, size);
  }
  /** Initialize an obufstream
   * \param[in] buf buffer for formatted string
   * \param[in] size buffer size
   */
  void init(char *buf, size_t size) {
    m_buf = buf;
    buf[0] = '\0';
    m_size = size;
    m_in = 0;
  }
  /** \return a pointer to the buffer */
  char* buf() {
    return m_buf;
  }
  /** \return the length of the formatted string */
  size_t length() {
    return m_in;
  }

 protected:
  /// @cond SHOW_PROTECTED
  void putch(char c) {
    if (m_in >= (m_size - 1)) {
      setstate(badbit);
      return;
    }
    m_buf[m_in++] = c;
    m_buf[m_in] = '\0';
  }
  void putstr(const char *str) {
    while (*str) {
      putch(*str++);
    }
  }
  bool seekoff(off_type off, seekdir way) {
    (void)off;
    (void)way;
    return false;
  }
  bool seekpos(pos_type pos) {
    if (pos > m_in) {
      return false;
    }
    m_in = pos;
    m_buf[m_in] = '\0';
    return true;
  }
  bool sync() {
    return true;
  }

  pos_type tellpos() {
    return m_in;
  }
  /// @endcond
 private:
  char *m_buf;
  size_t m_size;
  size_t m_in;
};
#endif  // bufstream_h
