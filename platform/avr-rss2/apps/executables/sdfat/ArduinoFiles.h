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
/**
 * \file
 * \brief PrintFile class
 */
#ifndef ArduinoFiles_h
#define ArduinoFiles_h
#include "FatLibConfig.h"
#if ENABLE_ARDUINO_FEATURES
#include "FatFile.h"
#include <limits.h>
//------------------------------------------------------------------------------
/** Arduino SD.h style flag for open for read. */
#define FILE_READ O_READ
/** Arduino SD.h style flag for open at EOF for read/write with create. */
#define FILE_WRITE (O_RDWR | O_CREAT | O_AT_END)
//==============================================================================
/**
 * \class PrintFile
 * \brief FatFile with Print.
 */
class PrintFile : public FatFile, public Print {
 public:
  PrintFile() {}
  /**  Create a file object and open it in the current working directory.
   *
   * \param[in] path A path for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   * bitwise-inclusive OR of open flags. see
   * FatFile::open(FatFile*, const char*, uint8_t).
   */
  PrintFile(const char* path, uint8_t oflag) : FatFile(path, oflag) {}
#if DESTRUCTOR_CLOSES_FILE
  ~PrintFile() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  using FatFile::clearWriteError;
  using FatFile::getWriteError;
  using FatFile::read;
  using FatFile::write;
  /** \return number of bytes available from the current position to EOF
   *   or INT_MAX if more than INT_MAX bytes are available.
   */
  int available() {
    uint32_t n = FatFile::available();
    return n > INT_MAX ? INT_MAX : n;
  }
  /** Ensure that any bytes written to the file are saved to the SD card. */
  void flush() {
    FatFile::sync();
  }
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */
  int peek() {
    return FatFile::peek();
  }
  /** Read the next byte from a file.
   *
   * \return For success return the next byte in the file as an int.
   * If an error occurs or end of file is reached return -1.
   */
//  int read() {
//    return FatFile::read();
//  }
  /** Write a byte to a file. Required by the Arduino Print class.
   * \param[in] b the byte to be written.
   * Use getWriteError to check for errors.
   * \return 1 for success and 0 for failure.
   */
  size_t write(uint8_t b) {
    return FatFile::write(b);
  }
  /** Write data to an open file.  Form required by Print.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] size Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an
   * I/O error.
   */
  size_t write(const uint8_t *buf, size_t size) {
    return FatFile::write(buf, size);
  }
};
//==============================================================================
/**
 * \class File
 * \brief Arduino SD.h style File API
 */
#if ARDUINO_FILE_USES_STREAM
class File : public FatFile, public Stream {
#else  // ARDUINO_FILE_USES_STREAM
class File : public FatFile, public Print {
#endif  // ARDUINO_FILE_USES_STREAM
 public:
  File() {}
  /**  Create a file object and open it in the current working directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   * bitwise-inclusive OR of open flags. see
   * FatFile::open(FatFile*, const char*, uint8_t).
   */
  File(const char* path, uint8_t oflag) {
    open(path, oflag);
  }
  using FatFile::clearWriteError;
  using FatFile::getWriteError;
  using FatFile::read;
  using FatFile::write;
  /** The parenthesis operator.
    *
    * \return true if a file is open.
    */
  operator bool() {
    return isOpen();
  }
  /** \return number of bytes available from the current position to EOF
   *   or INT_MAX if more than INT_MAX bytes are available.
   */
  int available() {
    uint32_t n = FatFile::available();
    return n > INT_MAX ? INT_MAX : n;
  }
  /** Ensure that any bytes written to the file are saved to the SD card. */
  void flush() {
    FatFile::sync();
  }
  /** This function reports if the current file is a directory or not.
  * \return true if the file is a directory.
  */
  bool isDirectory() {
    return isDir();
  }
  /** No longer implemented due to Long File Names.
   *
   * Use getName(char* name, size_t size).
   * \return a pointer to replacement suggestion. 
   */
  const char* name() const {
    return "use getName()";
  }
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */
  int peek() {
    return FatFile::peek();
  }
  /** \return the current file position. */
  uint32_t position() {
    return curPosition();
  }
  /** Opens the next file or folder in a directory.
   *
   * \param[in] mode open mode flags.
   * \return a File object.
   */
  File openNextFile(uint8_t mode = O_READ) {
    File tmpFile;
    tmpFile.openNext(this, mode);
    return tmpFile;
  }
  /** Read the next byte from a file.
   *
   * \return For success return the next byte in the file as an int.
   * If an error occurs or end of file is reached return -1.
   */
  int read() {
    return FatFile::read();
  }
  /** Rewind a file if it is a directory */
  void rewindDirectory() {
    if (isDir()) {
      rewind();
    }
  }
  /**
   * Seek to a new position in the file, which must be between
   * 0 and the size of the file (inclusive).
   *
   * \param[in] pos the new file position.
   * \return true for success else false.
   */
  bool seek(uint32_t pos) {
    return seekSet(pos);
  }
  /** \return the file's size. */
  uint32_t size() {
    return fileSize();
  }
  /** Write a byte to a file. Required by the Arduino Print class.
   * \param[in] b the byte to be written.
   * Use getWriteError to check for errors.
   * \return 1 for success and 0 for failure.
   */
  size_t write(uint8_t b) {
    return FatFile::write(b);
  }
  /** Write data to an open file.  Form required by Print.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] size Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an
   * I/O error.
   */
  size_t write(const uint8_t *buf, size_t size) {
    return FatFile::write(buf, size);
  }
};
#endif  // ENABLE_ARDUINO_FEATURES
#endif  // ArduinoFiles_h
