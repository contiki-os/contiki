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
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "contiki.h"
#include "ctk/ctk.h"

#include "ctk-console.h"

static HANDLE stdinhandle;
static HANDLE stdouthandle;

static unsigned char width;
static unsigned char height;

static DWORD               saved_inputmode;
static DWORD               saved_outputmode;
static unsigned char       saved_color;
static char                saved_title[1024];
static CONSOLE_CURSOR_INFO saved_cursorinfo;

static unsigned char color;
static unsigned char reversed;

static unsigned char blank[1024];
static unsigned char hline[1024];

static ctk_arch_key_t keys[256];
static unsigned char  available;

static unsigned short xpos;
static unsigned short ypos;
static unsigned char  button;

/*-----------------------------------------------------------------------------------*/
static BOOL WINAPI
ctrlhandler(DWORD ctrltype)
{
  if(ctrltype == CTRL_C_EVENT) {
    exit(EXIT_SUCCESS);
    return TRUE;
  }
  return FALSE;
}
/*-----------------------------------------------------------------------------------*/
void
console_init(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;
  CONSOLE_CURSOR_INFO cursorinfo = {1, FALSE};
  static unsigned char done;

  if(done) {
    return;
  }
  done = 1;

  stdinhandle  = GetStdHandle(STD_INPUT_HANDLE);
  stdouthandle = GetStdHandle(STD_OUTPUT_HANDLE);

  GetConsoleMode(stdinhandle, &saved_inputmode);
  SetConsoleMode(stdinhandle, ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);

  GetConsoleMode(stdouthandle, &saved_outputmode);
  SetConsoleMode(stdouthandle, ENABLE_PROCESSED_OUTPUT);

  screensize(&width, &height);

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  saved_color = (unsigned char)consoleinfo.wAttributes;

  GetConsoleTitle(saved_title, sizeof(saved_title));
  SetConsoleTitle("Contiki");

  GetConsoleCursorInfo(stdouthandle, &saved_cursorinfo);
  SetConsoleCursorInfo(stdouthandle, &cursorinfo);

  SetConsoleCtrlHandler(ctrlhandler, TRUE);
  atexit(console_exit);

  memset(blank, ' ',  sizeof(blank));
  memset(hline, 0xC4, sizeof(hline));
}
/*-----------------------------------------------------------------------------------*/
void
console_exit(void)
{
  static unsigned char done;

  if(done) {
    return;
  }
  done = 1;

  textcolor(saved_color);
  revers(0);
  clrscr();
  gotoxy(0, 0);

  SetConsoleMode(stdinhandle,  saved_inputmode);
  SetConsoleMode(stdouthandle, saved_outputmode);
  SetConsoleTitle(saved_title);
  SetConsoleCursorInfo(stdouthandle, &saved_cursorinfo);
}
/*-----------------------------------------------------------------------------------*/
unsigned char
console_resize(void)
{
  unsigned char new_width;
  unsigned char new_height;

  screensize(&new_width, &new_height);

  if(new_width  != width ||
     new_height != height) {
    width  = new_width;
    height = new_height;
    return 1;
  }

  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
setcolor(void)
{
  SetConsoleTextAttribute(stdouthandle, (WORD)(reversed? (color & 0x0F) << 4 |
							 (color & 0xF0) >> 4
						       : color));
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  return (unsigned char)consoleinfo.dwCursorPosition.X;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  return (unsigned char)consoleinfo.dwCursorPosition.Y;
}
/*-----------------------------------------------------------------------------------*/
void
clrscr(void)
{
  unsigned char i, width, height;

  screensize(&width, &height);
  for(i = 0; i < height; ++i) {
    cclearxy(0, i, width);
  }
}
/*-----------------------------------------------------------------------------------*/
void
bgcolor(unsigned char c)
{
  /* Presume this to be one of the first calls. */
  console_init();
}
/*-----------------------------------------------------------------------------------*/
void
bordercolor(unsigned char c)
{
  /* Presume this to be one of the first calls. */
  console_init();
}
/*-----------------------------------------------------------------------------------*/
void
screensize(unsigned char *x, unsigned char *y)
{
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleScreenBufferInfo(stdouthandle, &consoleinfo);
  *x = consoleinfo.srWindow.Right - consoleinfo.srWindow.Left + 1;
  *y = consoleinfo.srWindow.Bottom - consoleinfo.srWindow.Top + 1;
}
/*-----------------------------------------------------------------------------------*/
void
revers(unsigned char c)
{
  reversed = c;
  setcolor();
}
/*-----------------------------------------------------------------------------------*/
void
console_cputc(char c)
{
  DWORD dummy;

  WriteConsole(stdouthandle, &c, 1, &dummy, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
console_cputs(char *str)
{
  DWORD dummy;

  WriteConsole(stdouthandle, str, (DWORD)strlen(str), &dummy, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
cclear(unsigned char length)
{
  DWORD dummy;

  WriteConsole(stdouthandle, blank, length, &dummy, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
chline(unsigned char length)
{
  DWORD dummy;

  WriteConsole(stdouthandle, hline, length, &dummy, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
cvline(unsigned char length)
{
  unsigned char i, x, y;

  x = wherex();
  y = wherey();

  for(i = 0; i < length; ++i) {
    cputcxy(x, (unsigned char)(y + i), (char)0xB3);
  }
}
/*-----------------------------------------------------------------------------------*/
void
gotoxy(unsigned char x, unsigned char y)
{
  COORD coord = {x, y};

  SetConsoleCursorPosition(stdouthandle, coord);
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
  console_cputs(str);
}
/*-----------------------------------------------------------------------------------*/
void
cputcxy(unsigned char x, unsigned char y, char c)
{
  gotoxy(x, y);
  console_cputc(c);
}
/*-----------------------------------------------------------------------------------*/
void
textcolor(unsigned char c)
{
  color = c;
  setcolor();
}
/*-----------------------------------------------------------------------------------*/
static void
console_readkey(KEY_EVENT_RECORD keyrecord)
{
  ctk_arch_key_t key;

  if(!keyrecord.bKeyDown) {
    return;
  }

  if(keyrecord.wRepeatCount > (WORD)255 - available) {
    keyrecord.wRepeatCount = (WORD)255 - available;
  }

  key = keyrecord.uChar.AsciiChar;
  if(key == CTK_CONF_WIDGETDOWN_KEY && keyrecord.dwControlKeyState & SHIFT_PRESSED) {
    key = CTK_CONF_WIDGETUP_KEY;
  }
  if(key == 0) {
    switch(keyrecord.wVirtualKeyCode) {
    case VK_TAB:
	if(keyrecord.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
	  key = CTK_CONF_WINDOWSWITCH_KEY;
	}
	break;
    case VK_LEFT:
	key = CH_CURS_LEFT;
	break;
    case VK_UP:
	key = CH_CURS_UP;
	break;
    case VK_RIGHT:
	key = CH_CURS_RIGHT;
	break;
    case VK_DOWN:
	key = CH_CURS_DOWN;
	break;
    case VK_F10:
	key = CTK_CONF_MENU_KEY;
	break;
    }
  }

  if(key == 0) {
    return;
  }

  memset(keys + available, key, keyrecord.wRepeatCount);
  available += (unsigned char)keyrecord.wRepeatCount;
}
/*-----------------------------------------------------------------------------------*/
static void
console_readmouse(MOUSE_EVENT_RECORD mouserecord)
{
  xpos = mouserecord.dwMousePosition.X;
  ypos = mouserecord.dwMousePosition.Y;

  button = (unsigned char)mouserecord.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED;
}
/*-----------------------------------------------------------------------------------*/
static void
console_read(void)
{
  INPUT_RECORD inputrecord;
  DWORD count;

  if(!GetNumberOfConsoleInputEvents(stdinhandle, &count) || count == 0) {
    return;
  }
  if(!ReadConsoleInput(stdinhandle, &inputrecord, 1, &count) || count == 0) {
    return;
  }

  switch(inputrecord.EventType) {
  case KEY_EVENT:
    console_readkey(inputrecord.Event.KeyEvent);
    break;
  case MOUSE_EVENT:
    console_readmouse(inputrecord.Event.MouseEvent);
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
char
ctk_arch_getkey(void)
{
  console_read();
  return keys[--available];
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_arch_keyavail(void)
{
  console_read();
  return available;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_init(void)
{
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_x(void)
{
  console_read();
  return xpos;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_y(void)
{
  console_read();
  return ypos;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_xtoc(unsigned short x)
{
  return x;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_ytoc(unsigned short y)
{
  return y;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_button(void)
{
  console_read();
  return button;
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_hide(void)
{
}
/*-----------------------------------------------------------------------------------*/
void
ctk_mouse_show(void)
{
}
/*-----------------------------------------------------------------------------------*/
