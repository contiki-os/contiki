/*
   File: printf.c

   Copyright (c) 2004,2008 Kustaa Nyholm / SpareTimeLabs

   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or other
   materials provided with the distribution.

   Neither the name of the Kustaa Nyholm or SpareTimeLabs nor the names of its
   contributors may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
   OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
   OF SUCH DAMAGE.

 */

/*
 * This is BSD code obtained from http://www.sparetimelabs.com/printfrevisited/index.html
 * From the web page:
 * "The code is GPL and BSD lincensed, download the BSD licensed version from the link
 * above or use the GPL licensed code from this page below."
 *
 * modified by Beshr Al Nahas <beshr@sics.se> and Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dev/uart0.h"

static char *bf, buf[14], uc, zs;
static unsigned int num;

static void
out(char c)
{
  *bf++ = c;
}
static void
outDgt(char dgt)
{
  out(dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10));
  zs = 1;
}
static void
divOut(unsigned int div)
{
  unsigned char dgt = 0;
  while(num >= div) {
    num -= div;
    dgt++;
  }
  if(zs || dgt > 0) {
    outDgt(dgt);
  }
}
int
vsnprintf(char *str, size_t n, const char *fmt, __VALIST va)
{
  char ch, *p, *str_orig = str;
  char next_ch;

  while((ch = *fmt++) && str - str_orig < n) {
    if(ch != '%') {
      *str++ = ch;
    } else {
      char lz = 0;
      char w = 0;
      ch = *(fmt++);
      if(ch == '0') {
        ch = *(fmt++);
        lz = 1;
      }
      if(ch >= '0' && ch <= '9') {
        w = 0;
        while(ch >= '0' && ch <= '9') {
          w = (((w << 2) + w) << 1) + ch - '0';
          ch = *fmt++;
        }
      }
      bf = buf;
      p = bf;
      zs = 0;
start_format:
      next_ch = *fmt;
      switch(ch) {
      case 0:
        goto abort;
      case 'l':
        if(next_ch == 'x'
           || next_ch == 'X'
           || next_ch == 'u'
           || next_ch == 'd') {
          ch = *(fmt++);
          goto start_format;
        }
      case 'u':
      case 'd':
        num = va_arg(va, unsigned int);
        if(ch == 'd' && (int)num < 0) {
          num = -(int)num;
          out('-');
        }
        divOut(1000000000);
        divOut(100000000);
        divOut(10000000);
        divOut(1000000);
        divOut(100000);
        divOut(10000);
        divOut(1000);
        divOut(100);
        divOut(10);
        outDgt(num);
        break;
      case 'p':
      case 'x':
      case 'X':
        uc = ch == 'X';
        num = va_arg(va, unsigned int);
        /* divOut(0x100000000UL); */
        divOut(0x10000000);
        divOut(0x1000000);
        divOut(0x100000);
        divOut(0x10000);
        divOut(0x1000);
        divOut(0x100);
        divOut(0x10);
        outDgt(num);
        break;
      case 'c':
        out((char)(va_arg(va, int)));
        break;
      case 's':
        p = va_arg(va, char *);
        break;
      case '%':
        out('%');
      default:
        break;
      }
      *bf = 0;
      bf = p;

      while(*bf++ && w > 0) {
        w--;
      }
      while(w-- > 0) {
        if(str - str_orig < n) {
          *str++ = lz ? '0' : ' ';
        } else {
          goto abort;
        }
      }
      while((ch = *p++)) {
        if(str - str_orig < n) {
          *str++ = ch;
        } else {
          goto abort;
        }
      }
    }
  }

abort:
  if(str - str_orig < n) {
    *str = '\0';
  } else {
    *(--str) = '\0';
  } return str - str_orig;
}
int
sprintf(char *str, const char *fmt, ...)
{
  int m;
  __VALIST va;
  va_start(va, fmt);
  m = vsnprintf(str, 0xffffffff, fmt, va);
  va_end(va);
  return m;
}
int
snprintf(char *str, size_t n, const char *fmt, ...)
{
  int m;
  __VALIST va;
  va_start(va, fmt);
  m = vsnprintf(str, n, fmt, va);
  va_end(va);
  return m;
}
int
printf(const char *fmt, ...)
{
  int m, i;
  char str[256];
  __VALIST va;
  va_start(va, fmt);
  m = vsnprintf(str, sizeof(str), fmt, va);
  va_end(va);
  for(i = 0; i < m; i++) {
    putchar(str[i]);
  }
  return m;
}
int
puts(const char *s)
{
  char c;
  while((c = *s++) != '\0') {
    putchar(c);
  }
  putchar('\n');
  return strlen(s);
}
