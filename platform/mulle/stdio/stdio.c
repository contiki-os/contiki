/*
File: printf.c

Copyright (C) 2004  Kustaa Nyholm

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include "stdio.h"
#include "string.h"

typedef void (*putcf) (void*,char);
static putcf stdout_putf;
static void* stdout_putp;
static char stdout_inited = 0;


#ifdef PRINTF_LONG_SUPPORT

static void uli2a(unsigned long num, unsigned long base, int uc,char * bf_out)
{
  int n=0;
  unsigned long d=1;
  char bf_tmp[12];
  char*bf = bf_tmp;

  while (num/d >= base)
    d*=base;
  while (d!=0L) {
      int dgt = num / d;
      num%=d;
      d/=base;
      if (n || dgt>0|| d==0) {
          *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
          ++n;
      }
  }
  *bf=0;
  while (n++ < 8 && base == 16) {
      *bf_out++ = '0';
  }
  strcpy(bf_out, bf_tmp);
}

static void li2a (long num, char * bf)
{
  if (num<0L) {
      num=-num;
      *bf++ = '-';
  }
  uli2a(num,10,0,bf);
}

#endif


#ifdef PRINTF_FLOAT_SUPPORT
unsigned long decimal_array[6] = {0, 10, 100, 1000, 10000, 100000};
static void f2a(double num, int decimals, char* bf)
{
  int i;
  int length;
  char prev = '.';
  int small = 0;


  if(num<0)
    {
      num=num*-1;
      *bf++ = '-';
    }

  if (num < 1)
    {
      small = 1;
      num += 1;
    }

  num = num*decimal_array[decimals%6];
  uli2a((unsigned long)num,10,0,bf);
  length = strlen(bf);
  if (small)
    {
      bf[0] = '0';
    }
  for (i = length-decimals; i < length; ++i)
    {
      char tmp = bf[i];
      bf[i] = prev;
      prev = tmp;
    }
  bf[i] = prev;
  bf[i+1] = 0;
}

#endif // PRINTF_FLOAT_SUPPORT



static void ui2a(unsigned int num, unsigned int base, int uc,char * bf_out)
{
  int n=0;
  unsigned int d=1;
  char bf_tmp[12];
  char*bf = bf_tmp;
  while (num/d >= base)
    d*=base;
  while (d!=0) {
      int dgt = num / d;
      num%= d;
      d/=base;
      if (n || dgt>0 || d==0) {
          *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
          ++n;
      }
  }
  *bf=0;
  while (n++ < 4 && base == 16) {
      *bf_out++ = '0';
  }
  strcpy(bf_out, bf_tmp);
}

static void i2a (int num, char * bf)
{
  if (num<0) {
      num=-num;
      *bf++ = '-';
  }
  ui2a(num,10,0,bf);
}

static int a2d(char ch)
{
  if (ch>='0' && ch<='9')
    return ch-'0';
  else if (ch>='a' && ch<='f')
    return ch-'a'+10;
  else if (ch>='A' && ch<='F')
    return ch-'A'+10;
  else return -1;
}

static char a2i(char ch, const char** src,int base,int* nump)
{
  const char* p= *src;
  int num=0;
  int digit;
  while ((digit=a2d(ch))>=0) {
      if (digit>base) break;
      num=num*base+digit;
      ch=*p++;
  }
  *src=p;
  *nump=num;
  return ch;
}

static void putchw(void* putp,putcf putf,int n, char z, char* bf)
{
  char fc=z? '0' : ' ';
  char ch;
  char* p=bf;
  while (*p++ && n > 0)
    n--;
  while (n-- > 0)
    putf(putp,fc);
  while ((ch= *bf++))
    putf(putp,ch);
}

void tfp_format(void* putp,putcf putf,const char *fmt, va_list va)
{
  char bf[12];

  char ch;


  while ((ch=*(fmt++))) {
      if (ch!='%')
        putf(putp,ch);
      else {
          char lz=0;
#ifdef 	PRINTF_LONG_SUPPORT
          char lng=0;
#endif
          int w=0;
          ch=*(fmt++);
          if (ch=='0') {
              ch=*(fmt++);
              lz=1;
          }
          if (ch>='0' && ch<='9') {
              ch=a2i(ch,&fmt,10,&w);
          }
#ifdef 	PRINTF_LONG_SUPPORT
          if (ch=='l') {
              ch=*(fmt++);
              lng=1;
          }
#endif
          switch (ch) {
          case 0:
            goto abort;
          case 'u' : {
#ifdef 	PRINTF_LONG_SUPPORT
            if (lng)
              uli2a(va_arg(va, unsigned long),10,0,bf);
            else
#endif
              ui2a(va_arg(va, unsigned int),10,0,bf);
            putchw(putp,putf,w,lz,bf);
            break;
          }
          case 'i' :
          case 'd' :  {
#ifdef 	PRINTF_LONG_SUPPORT
            if (lng){
                li2a(va_arg(va, unsigned long),bf);}
            else
#endif
              {i2a(va_arg(va, int),bf); }
            putchw(putp,putf,w,lz,bf);
            break;
          }
          case 'x': case 'X' :
#ifdef 	PRINTF_LONG_SUPPORT
            if (lng)
              uli2a(va_arg(va, unsigned long),16,(ch=='X'),bf);
            else
#endif
              ui2a(va_arg(va, unsigned int),16,(ch=='X'),bf);
            putchw(putp,putf,w,lz,bf);
            break;
#ifdef  PRINTF_FLOAT_SUPPORT
          case 'f' :
            f2a(va_arg(va, double), 2, bf);
            putchw(putp,putf,w,lz,bf);
            break;
#endif
          case 'c' :
            putf(putp,(char)(va_arg(va, int)));
            break;
          case 's' :
            putchw(putp,putf,w,0,va_arg(va, char*));
            break;
          case '%' :
            putf(putp,ch);
          default:
            break;
          }
      }
  }
  abort:;
}

void init_printf(void* putp,void (*putf) (void*,char))
{
  stdout_inited = 1;
  stdout_putf=putf;
  stdout_putp=putp;
}

int tfp_printf(const char *fmt, ...)
{
  va_list va;
  static int inited = 0;
  if (!stdout_inited) {
      return 0;
  }
  va_start(va,fmt);
  tfp_format(stdout_putp,stdout_putf,fmt,va);
  va_end(va);
  return 1;
}

static void putcp(void* p,char c)
{
  *(*((char**)p))++ = c;
}



int tfp_sprintf(char* s, const char *fmt, ...)
{
  va_list va;
  char* tmp = s;
  va_start(va,fmt);
  tfp_format(&s,putcp,fmt,va);
  putcp(&s,0);
  va_end(va);
  return strlen(tmp);
}


