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
#ifndef fstream_h
#define fstream_h
/**
 * \file
 * \brief \ref fstream, \ref ifstream, and \ref ofstream classes
 */
#include "FatFile.h"
#include "iostream.h"
//==============================================================================
/**
 * \class FatStreamBase
 * \brief Base class for C++ style streams
 */
class FatStreamBase : protected FatFile, virtual public ios {
 protected:
  /// @cond SHOW_PROTECTED
  int16_t getch();
  void putch(char c);
  void putstr(const char *str);
  void open(const char* path, ios::openmode mode);
  /** Internal do not use
   * \return mode
   */
  ios::openmode getmode() {
    return m_mode;
  }
  /** Internal do not use
   * \param[in] mode
   */
  void setmode(ios::openmode mode) {
    m_mode = mode;
  }
  bool seekoff(off_type off, seekdir way);
  bool seekpos(pos_type pos);
  int write(const void* buf, size_t n);
  void write(char c);
  /// @endcond
 private:
  ios::openmode m_mode;
};
//==============================================================================
/**
 * \class fstream
 * \brief file input/output stream.
 */
class fstream : public iostream, FatStreamBase {
 public:
  using iostream::peek;
  fstream() {}
  /** Constructor with open
   *
   * \param[in] path path to open
   * \param[in] mode open mode
   */
  explicit fstream(const char* path, openmode mode = in | out) {
    open(path, mode);
  }
#if DESTRUCTOR_CLOSES_FILE
  ~fstream() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** Clear state and writeError
   * \param[in] state new state for stream
   */
  void clear(iostate state = goodbit) {
    ios::clear(state);
    FatFile::clearWriteError();
  }
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {
    FatFile::close();
  }
  /** Open a fstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * Valid open modes are (at end, ios::ate, and/or ios::binary may be added):
   *
   * ios::in - Open file for reading.
   *
   * ios::out or ios::out | ios::trunc - Truncate to 0 length, if existent,
   * or create a file for writing only.
   *
   * ios::app or ios::out | ios::app - Append; open or create file for
   * writing at end-of-file.
   *
   * ios::in | ios::out - Open file for update (reading and writing).
   *
   * ios::in | ios::out | ios::trunc - Truncate to zero length, if existent,
   * or create file for update.
   *
   * ios::in | ios::app or ios::in | ios::out | ios::app - Append; open or
   * create text file for update, writing at end of file.
   */
  void open(const char* path, openmode mode = in | out) {
    FatStreamBase::open(path, mode);
  }
  /** \return True if stream is open else false. */
  bool is_open() {
    return FatFile::isOpen();
  }

 protected:
  /// @cond SHOW_PROTECTED
  /** Internal - do not use
   * \return
   */
  int16_t getch() {
    return FatStreamBase::getch();
  }
  /** Internal - do not use
  * \param[out] pos
  */
  void getpos(FatPos_t* pos) {
    FatFile::getpos(pos);
  }
  /** Internal - do not use
   * \param[in] c
   */
  void putch(char c) {
    FatStreamBase::putch(c);
  }
  /** Internal - do not use
   * \param[in] str
   */
  void putstr(const char *str) {
    FatStreamBase::putstr(str);
  }
  /** Internal - do not use
   * \param[in] pos
   */
  bool seekoff(off_type off, seekdir way) {
    return FatStreamBase::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {
    return FatStreamBase::seekpos(pos);
  }
  void setpos(FatPos_t* pos) {
    FatFile::setpos(pos);
  }
  bool sync() {
    return FatStreamBase::sync();
  }
  pos_type tellpos() {
    return FatStreamBase::curPosition();
  }
  /// @endcond
};
//==============================================================================
/**
 * \class ifstream
 * \brief file input stream.
 */
class ifstream : public istream, FatStreamBase {
 public:
  using istream::peek;
  ifstream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit ifstream(const char* path, openmode mode = in) {
    open(path, mode);
  }
#if DESTRUCTOR_CLOSES_FILE
  ~ifstream() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {
    FatFile::close();
  }
  /** \return True if stream is open else false. */
  bool is_open() {
    return FatFile::isOpen();
  }
  /** Open an ifstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * \a mode See fstream::open() for valid modes.
   */
  void open(const char* path, openmode mode = in) {
    FatStreamBase::open(path, mode | in);
  }

 protected:
  /// @cond SHOW_PROTECTED
  /** Internal - do not use
   * \return
   */
  int16_t getch() {
    return FatStreamBase::getch();
  }
  /** Internal - do not use
   * \param[out] pos
   */
  void getpos(FatPos_t* pos) {
    FatFile::getpos(pos);
  }
  /** Internal - do not use
   * \param[in] pos
   */
  bool seekoff(off_type off, seekdir way) {
    return FatStreamBase::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {
    return FatStreamBase::seekpos(pos);
  }
  void setpos(FatPos_t* pos) {
    FatFile::setpos(pos);
  }
  pos_type tellpos() {
    return FatStreamBase::curPosition();
  }
  /// @endcond
};
//==============================================================================
/**
 * \class ofstream
 * \brief file output stream.
 */
class ofstream : public ostream, FatStreamBase {
 public:
  ofstream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit ofstream(const char* path, ios::openmode mode = out) {
    open(path, mode);
  }
#if DESTRUCTOR_CLOSES_FILE
  ~ofstream() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** Clear state and writeError
   * \param[in] state new state for stream
   */
  void clear(iostate state = goodbit) {
    ios::clear(state);
    FatFile::clearWriteError();
  }
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {
    FatFile::close();
  }
  /** Open an ofstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * \a mode See fstream::open() for valid modes.
   */
  void open(const char* path, openmode mode = out) {
    FatStreamBase::open(path, mode | out);
  }
  /** \return True if stream is open else false. */
  bool is_open() {
    return FatFile::isOpen();
  }

 protected:
  /// @cond SHOW_PROTECTED
  /**
   * Internal do not use
   * \param[in] c
   */
  void putch(char c) {
    FatStreamBase::putch(c);
  }
  void putstr(const char* str) {
    FatStreamBase::putstr(str);
  }
  bool seekoff(off_type off, seekdir way) {
    return FatStreamBase::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {
    return FatStreamBase::seekpos(pos);
  }
  /**
   * Internal do not use
   * \param[in] b
   */
  bool sync() {
    return FatStreamBase::sync();
  }
  pos_type tellpos() {
    return FatStreamBase::curPosition();
  }
  /// @endcond
};
//------------------------------------------------------------------------------
#endif  // fstream_h
