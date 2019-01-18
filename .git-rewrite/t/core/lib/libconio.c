/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki desktop environment
 *
 * $Id: libconio.c,v 1.2 2007/09/09 12:24:44 matsutsuka Exp $
 *
 */

#include <string.h>
#include "contiki.h"
#include "libconio.h"

static unsigned char cursx, cursy;
static unsigned char reversed;
static unsigned char color;

/*-----------------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
  return cursx;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
  return cursy;
}
/*-----------------------------------------------------------------------------------*/
void
clrscr(void)
{
  unsigned char x, y;

  for(x = 0; x < LIBCONIO_SCREEN_WIDTH; ++x) {
    for(y = 0; y < LIBCONIO_SCREEN_HEIGHT; ++y) {
      gotoxy(x, y);
      cputc(' ');
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
revers(unsigned char c)
{
  reversed = c;
}
/*-----------------------------------------------------------------------------------*/
void
cputc(char c)
{
  ctk_arch_draw_char(c, cursx, cursy, reversed, color);
  ++cursx;
}
/*-----------------------------------------------------------------------------------*/
void
cputs(char *str)
{
  while(*str != 0) {
    cputc(*str++);
  }
  
  /*  int i;
  for(i = 0; i < strlen(str); ++i) {
    cputc(str[i]);
    }*/
}
/*-----------------------------------------------------------------------------------*/
void
cclear(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc(' ');
  }  
}
/*-----------------------------------------------------------------------------------*/
void
chline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc('-');
  }
}
/*-----------------------------------------------------------------------------------*/
void
cvline(unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i) {
    cputc('|');
    --cursx;
    ++cursy;
  }
}
/*-----------------------------------------------------------------------------------*/
void
gotoxy(unsigned char x, unsigned char y)
{
  cursx = x;
  cursy = y;
}
/*-----------------------------------------------------------------------------------*/
void
cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cclear(length);
}
/*-----------------------------------------------------------------------------------*/
void
chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  chline(length);
}
/*-----------------------------------------------------------------------------------*/
void
cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
  gotoxy(x, y);
  cvline(length);
}
/*-----------------------------------------------------------------------------------*/
void
cputsxy(unsigned char x, unsigned char y, char *str)
{
  gotoxy(x, y);
  cputs(str);
}
/*-----------------------------------------------------------------------------------*/
void
cputcxy(unsigned char x, unsigned char y, char c)
{
  gotoxy(x, y);
  cputc(c);
}
/*-----------------------------------------------------------------------------------*/
void
textcolor(unsigned char c)
{
  color = c;
}
/*-----------------------------------------------------------------------------------*/
void
bgcolor(unsigned char c)
{

}
/*-----------------------------------------------------------------------------------*/
void
bordercolor(unsigned char c)
{

}
/*-----------------------------------------------------------------------------------*/
void
screensize(unsigned char *x, unsigned char *y)
{
  *x = LIBCONIO_SCREEN_WIDTH;
  *y = LIBCONIO_SCREEN_HEIGHT;
}
/*-----------------------------------------------------------------------------------*/
