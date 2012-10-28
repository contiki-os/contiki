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
 *
 */

#ifndef __LIBCONIO_H__
#define __LIBCONIO_H__

/* This function must be implemented specifically for the
   architecture: */
void ctk_arch_draw_char(char c,
			unsigned char xpos,
			unsigned char ypos,
			unsigned char reversedflag,
			unsigned char color);

/* Default definitions that should be overridden by calling module. */
#ifndef LIBCONIO_CONF_SCREEN_WIDTH
#define LIBCONIO_SCREEN_WIDTH 40
#else
#define LIBCONIO_SCREEN_WIDTH LIBCONIO_CONF_SCREEN_WIDTH
#endif /* LIBCONIO_SCREEN_WIDTH */

#ifndef LIBCONIO_CONF_SCREEN_HEIGHT
#define LIBCONIO_SCREEN_HEIGHT 25
#else
#define LIBCONIO_SCREEN_HEIGHT LIBCONIO_CONF_SCREEN_HEIGHT
#endif /* LIBCONIO_CONF_SCREEN_HEIGHT */



/* These are function declarations for functions implemented in libconio.c */
unsigned char wherex(void);
unsigned char wherey(void);
void clrscr(void);
void bgcolor(unsigned char c);
void bordercolor(unsigned char c);
void screensize(unsigned char *x, unsigned char *y);
void revers(unsigned char c);
void cputc(char c);
void cputs(char *str);
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



#endif /* __LIBCONIO_H__ */
