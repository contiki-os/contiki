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
#ifndef FmtNumber_h
#define FmtNumber_h
//  #include <ctype.h>
inline bool isDigit(char c) {
  return '0' <= c && c <= '9';
}
inline bool isSpace(char c) {
  return c == ' ' || (0X9 <= c && c <= 0XD);
}
#include <math.h>
#include <stdint.h>
char* fmtDec(uint16_t n, char* p);
char* fmtDec(uint32_t n, char* p);
char* fmtFloat(float value, char* p, uint8_t prec);
char* fmtFloat(float value, char* ptr, uint8_t prec, char expChar);
char* fmtHex(uint32_t n, char* p);
float scale10(float v, int8_t n);
float scanFloat(const char* str, char** ptr);
#endif  // FmtNumber_h
