/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 * $Id: ctk-console.h,v 1.5 2007/04/15 13:30:17 oliverschmidt Exp $
 */
#ifndef __CTK_CONSOLE_H__
#define __CTK_CONSOLE_H__

#define cputc console_cputc
#define cputs console_cputs

void console_init(void);
void console_exit(void);
unsigned char console_resize(void);

unsigned char wherex(void);
unsigned char wherey(void);
void clrscr(void);
void bgcolor(unsigned char c);
void bordercolor(unsigned char c);
void screensize(unsigned char *x, unsigned char *y);
void revers(unsigned char c);
void console_cputc(char c);
void console_cputs(char *str);
void cclear(unsigned char length);
void chline(unsigned char length);
void cvline(unsigned char length);
void gotoxy(unsigned char x, unsigned char y);
void cclearxy(unsigned char x, unsigned char y, unsigned char length);
void chlinexy(unsigned char x, unsigned char y, unsigned char length);
void cvlinexy(unsigned char x, unsigned char y, unsigned char length);
void cputsxy(unsigned char x, unsigned char y, char *str);
void cputcxy(unsigned char x, unsigned char y, char c);
void textcolor(unsigned char c);

char ctk_arch_getkey(void);
unsigned char ctk_arch_keyavail(void);

#endif /* __CTK_CONSOLE_H__ */
