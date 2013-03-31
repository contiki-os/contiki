/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 * Author: François Revol <revol@free.fr>
 */

#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <term.h>
#include <unistd.h>

#include "contiki.h"
#include "ctk/ctk.h"

#include "ctk-curses.h"

/* references:
 * http://math.hws.edu/orr/s04/cpsc225/curses.html
 * http://linux.die.net/man/3/ncurses
 * http://linux.die.net/HOWTO/NCURSES-Programming-HOWTO/index.html
 */

#define MKPAIR(bg, fg) (bg << 3 | fg)

static unsigned char width;
static unsigned char height;

static unsigned char color;
static unsigned char reversed;

static ctk_arch_key_t keys[256];
static unsigned char keys_in, keys_out;
static unsigned char available;

static unsigned short xpos;
static unsigned short ypos;
static unsigned char button;

/* map CTK colors to curses colors */
static unsigned char ctk_color_map[8] = {
  COLOR_BLACK,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_BLUE,
  COLOR_MAGENTA,
  COLOR_CYAN,
  COLOR_WHITE
};

/*-----------------------------------------------------------------------------------*/
static unsigned char
map_color(unsigned char color)
{
  unsigned char c;

  c = ctk_color_map[color & 0x0f];
  c |= ctk_color_map[(color >> 4) & 0x07] << 4;
  return c;
}
/*-----------------------------------------------------------------------------------*/
static void
ctrlhandler(int sig)
{
  /* make sure we call console_exit() to leave the terminal in a clean state */
  exit(EXIT_SUCCESS);
}
/*-----------------------------------------------------------------------------------*/
void
console_init(void)
{
  /* mouse support is ncurses-specific */
#ifdef NCURSES_MOUSE_VERSION
  mmask_t oldmask;
#endif
  static unsigned char done;
  int bg, fg;

  if(done) {
    return;
  }
  done = 1;

  initscr();
  start_color();
  cbreak();

  /* don't echo typed characters */
  noecho();
  /* disable return -> newline translation */
  nonl();

  /* hide text cursor, CTK draws its own */
  curs_set(0);

  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);

#ifdef NCURSES_MOUSE_VERSION
  /* done here because ctk_mouse_init() is called before anyway */
  mousemask(ALL_MOUSE_EVENTS, &oldmask);
#endif

  screensize(&width, &height);

  /* we must declare all possible color pairs */
  for(fg = 0; fg < 8; fg++) {
    for(bg = 0; bg < 8; bg++) {
      init_pair(MKPAIR(bg, fg), fg, bg);
    }
  }

  /* set window title */
  putp("\033]0;Contiki\a");

  /* don't block on read, just timeout 1ms */
  timeout(1);

  /* make sure we return the terminal in a clean state */
  signal(SIGINT, ctrlhandler);
  atexit(console_exit);
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

  revers(0);
  clrscr();
  gotoxy(0, 0);

  endwin();
}
/*-----------------------------------------------------------------------------------*/
unsigned char
console_resize(void)
{
  unsigned char new_width;
  unsigned char new_height;

  screensize(&new_width, &new_height);

  if(new_width != width || new_height != height) {
    width = new_width;
    height = new_height;
    return 1;
  }

  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
setcolor(void)
{
  int bg, fg;
  int attrs;

  fg = (color & 0x0F);
  bg = (color & 0xF0) >> 4;

  attrs = COLOR_PAIR(MKPAIR(bg, fg));
  if(reversed) {
    attrs |= WA_REVERSE;
  }
  attrset(attrs);
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
  int x, y;

  getyx(stdscr, y, x);
  (void)y;
  return (unsigned char)x;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
  int x, y;

  getyx(stdscr, y, x);
  (void)x;
  return (unsigned char)y;
}
/*-----------------------------------------------------------------------------------*/
void
clrscr(void)
{
  clear();
}
/*-----------------------------------------------------------------------------------*/
void
bgcolor(unsigned char c)
{
  c = map_color(c);
  color = ((c << 4) | (color & 0xF0));
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
  int mx, my;

  getmaxyx(stdscr, my, mx);
  *x = (unsigned char)mx;
  *y = (unsigned char)my;
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
  int ch = c;

  /* usually ACS_* don't fit in a char */
  switch (c) {
  case CH_ULCORNER:
    ch = ACS_ULCORNER;
    break;
  case CH_LLCORNER:
    ch = ACS_LLCORNER;
    break;
  case CH_URCORNER:
    ch = ACS_URCORNER;
    break;
  case CH_LRCORNER:
    ch = ACS_LRCORNER;
    break;
  default:
    break;
  }
  addch(ch);
  refresh();
}
/*-----------------------------------------------------------------------------------*/
void
console_cputs(char *str)
{
  addstr(str);
  refresh();
}
/*-----------------------------------------------------------------------------------*/
void
cclear(unsigned char length)
{
  hline(' ', length);
  refresh();
}
/*-----------------------------------------------------------------------------------*/
void
chline(unsigned char length)
{
  hline(ACS_HLINE, length);
  refresh();
}
/*-----------------------------------------------------------------------------------*/
void
cvline(unsigned char length)
{
  vline(ACS_VLINE, length);
}
/*-----------------------------------------------------------------------------------*/
void
gotoxy(unsigned char x, unsigned char y)
{
  move(y, x);
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
  color = map_color(c);
  setcolor();
}
/*-----------------------------------------------------------------------------------*/
static void
console_readkey(int k)
{
  ctk_arch_key_t key;

  key = (ctk_arch_key_t) k;
  /*fprintf(stderr, "key: %d\n", k); */
  switch (k) {
#ifdef NCURSES_MOUSE_VERSION
  case KEY_MOUSE:
    {
      MEVENT event;

      if(getmouse(&event) == OK) {
        xpos = event.x;
        ypos = event.y;
        button = event.bstate & BUTTON1_PRESSED
          || event.bstate & BUTTON1_CLICKED
          || event.bstate & BUTTON1_DOUBLE_CLICKED;
        /*fprintf(stderr, "mevent: %d: %d, %d, %d, %lx ; %d\n",
           event.id, event.x, event.y, event.z, event.bstate, button); */
      }
      return;
    }
#endif
  case KEY_LEFT:
    key = CH_CURS_LEFT;
    break;
  case KEY_UP:
    key = CH_CURS_UP;
    break;
  case KEY_RIGHT:
    key = CH_CURS_RIGHT;
    break;
  case KEY_DOWN:
    key = CH_CURS_DOWN;
    break;
  case KEY_F(9):               /* Gnome uses F10 as menu trigger now... */
  case KEY_F(10):
    key = CTK_CONF_MENU_KEY;
    break;
  case '\r':
  case KEY_ENTER:
    key = CH_ENTER;
    break;
  case 127:
  case KEY_BACKSPACE:
  case KEY_DC:
    key = CH_DEL;
    break;
  case KEY_BTAB:
  case KEY_CTAB:
  case KEY_PPAGE:
  case KEY_PREVIOUS:
    key = CTK_CONF_WIDGETUP_KEY;
    break;
  case KEY_NPAGE:
  case KEY_NEXT:
    key = CTK_CONF_WIDGETDOWN_KEY;
    break;
  case KEY_STAB:
  case KEY_HOME:
  case KEY_END:
    key = CTK_CONF_WINDOWSWITCH_KEY;
    break;
  default:
    break;
  }
  if(key == 0) {
    return;
  }

  memset(keys + keys_in, key, sizeof(ctk_arch_key_t));
  keys_in++;
  available++;
}
/*-----------------------------------------------------------------------------------*/
static void
console_read(void)
{
  int k;

  k = getch();
  if(k != ERR) {
    console_readkey(k);
  }
}
/*-----------------------------------------------------------------------------------*/
char
ctk_arch_getkey(void)
{
  char k;

  console_read();
  k = keys[keys_out++];

  available--;
  return k;
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
