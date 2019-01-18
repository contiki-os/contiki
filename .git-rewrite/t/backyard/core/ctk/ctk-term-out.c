/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: ctk-term-out.c,v 1.1 2007/05/26 21:54:33 oliverschmidt Exp $
 */
#include "libconio.h"
#include "ctk-term-int.h"
#include <string.h>
#include <stdio.h> // sprintf

#define PRINTF(x)

/*-----------------------------------------------------------------------------------*/
/* 
 * #defines and enums
 */
/*-----------------------------------------------------------------------------------*/

#define CHARS_WIDTH    LIBCONIO_CONF_SCREEN_WIDTH
#define CHARS_HEIGHT   LIBCONIO_CONF_SCREEN_HEIGHT

/*-----------------------------------------------------------------------------------*/
/* 
 * Local variables
 */
/*-----------------------------------------------------------------------------------*/

/* ANSI/VT100 colors
  0 - None
  1 - Bold (inc. inten)
  4 - Underscore
  7 - Reverse
  x0 - black
  x1 - red
  x2 - green
  x3 - yellow
  x4 - blue
  x5 - magenta
  x6 - cyan
  x7 - white
  x = 3 fg x = 4 bg
*/
#if 0 /* Colorfull theme */
static const char backgroundcolor[] = "\033[0;37;40m";

static const char wincol[] = "\033[0;37;40m";
static const char wincol_f[] = "\033[0;1;37;40m";
static const char wincol_d[] = "\033[0;30;47m";

static const char sepcol[] = "\033[0;37;40m";
static const char sepcol_f[] = "\033[0;1;37;40m";
static const char sepcol_d[] = "\033[0;30;47m";

static const char labcol[] = "\033[0;37;40m";
static const char labcol_f[] = "\033[1;37;40m";
static const char labcol_d[] = "\033[0;30;47m";

static const char butcol[] = "\033[0;37;40m";
static const char butcol_w[] = "\033[0;30;47m";
static const char butcol_f[] = "\033[0;1;37;40m";
static const char butcol_fw[] = "\033[0;1;37;46m";
static const char butcol_d[] = "\033[0;30;47m";
static const char butcol_dw[] = "\033[0;37;46m";

static const char hlcol[] = "\033[0;4;36;40m";
static const char hlcol_w[] = "\033[0;4;30;47m";
static const char hlcol_f[] = "\033[0;1;4;36;40m";
static const char hlcol_fw[] = "\033[0;1;4;37;46m";
static const char hlcol_d[] = "\033[0;4;34;47m";
static const char hlcol_dw[] = "\033[0;4;37;46m";

static const char iconcol[] = "\033[0;32;40m";
static const char iconcol_w[] = "\033[0;30;42m";

static const char menucolor[] = "\033[0;37;43m";
static const char activemenucolor[] = "\033[0;1;37;43m";
#endif

#if 1 /* B/W theme */
static const char backgroundcolor[] = "\033[0m";

static const char wincol[] = "\033[0m";
static const char wincol_f[] = "\033[0;1m";
static const char wincol_d[] = "\033[0;7m";

static const char sepcol[] = "\033[0m";
static const char sepcol_f[] = "\033[0;1m";
static const char sepcol_d[] = "\033[0;7m";

static const char labcol[] = "\033[0m";
static const char labcol_f[] = "\033[0;1m";
static const char labcol_d[] = "\033[0;7m";

static const char butcol[] = "\033[0m";
static const char butcol_w[] = "\033[0m";
static const char butcol_f[] = "\033[0;1m";
static const char butcol_fw[] = "\033[0;1;7m";
static const char butcol_d[] = "\033[0;7m";
static const char butcol_dw[] = "\033[0m";

static const char hlcol[] = "\033[0;4m";
static const char hlcol_w[] = "\033[0;4;7m";
static const char hlcol_f[] = "\033[0;1;4m";
static const char hlcol_fw[] = "\033[0;1;4;7m";
static const char hlcol_d[] = "\033[0;4;7m";
static const char hlcol_dw[] = "\033[0;4m";

static const char iconcol[] = "\033[0m";
static const char iconcol_w[] = "\033[0;7m";

static const char menucolor[] = "\033[0;7m";
static const char activemenucolor[] = "\033[0m";

#endif

static const char* const colortheme[] =
{
    backgroundcolor,
    
    /* Window colors */
    wincol, wincol, wincol_f, wincol_f, wincol_d, wincol_d,

    /* Separator colors. */
    sepcol, sepcol, sepcol_f, sepcol_f, sepcol_d, sepcol_d,    

    /* Label colors. */
    labcol, labcol, labcol_f, labcol_f, labcol_d, labcol_d,    

    /* Button colors. */
    butcol, butcol_w, butcol_f, butcol_fw, butcol_d, butcol_dw,    

    /* Hyperlink colors. */
    hlcol, hlcol_w, hlcol_f, hlcol_fw, hlcol_d, hlcol_dw,

    /* Textentry colors. */
    butcol, butcol_w, butcol_f, butcol_fw, butcol_d, butcol_dw,

    /* Icon colors */
    iconcol, iconcol_w, iconcol, iconcol_w, iconcol, iconcol_w,
    
    /* Menu colors. */
    menucolor, activemenucolor, activemenucolor
  };

static unsigned char  
  screen[CHARS_WIDTH * CHARS_HEIGHT],
  colorscreen[CHARS_WIDTH * CHARS_HEIGHT];

/*-----------------------------------------------------------------------------------*/
/* 
 * Add a character to the screen buffer 
 */
/*-----------------------------------------------------------------------------------*/
void
ctk_term_out_update_screen(unsigned char  xpos, 
			   unsigned char  ypos, 
			   unsigned char  c, 
			   unsigned char  color)
{
  if (c < 0x20) c = 0x20;
  screen[xpos + ypos * CHARS_WIDTH] = c;
  colorscreen[xpos + ypos * CHARS_WIDTH] = color;
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Check if there are any updated pending. If so, make the first one current
 */
/*-----------------------------------------------------------------------------------*/
static void 
check_updates(struct ctk_term_state* ts)
{
  if (ts->updates_current != NULL) return;
  ts->updates_current = ctk_term_update_dequeue(ts);
  if (ts->updates_current != NULL) {
    ts->x = ts->updates_current->x;
    ts->y = ts->updates_current->y;
    ts->w = ts->updates_current->w;
    ts->h = ts->updates_current->h;
    ts->x1 = ts->x2 = ts->x;
    ts->y1 = ts->y2 = ts->y;
  }
}

/*-----------------------------------------------------------------------------------*/
/** \internal
 * Adds a cursor position change to buffer. Returns 0 if string doesn't fit else
 * number of bytes actually written is returned.
 *
 * \param x X coordinate (screen coordinates)
 * \param y Y coordinate (screen coordinates)
 * \param buf Output buffer
 * \param maxlen Maximum number of bytes to store in buffer
 */
/*-----------------------------------------------------------------------------------*/
static unsigned short
move_to(unsigned char x, unsigned char y, unsigned char* buf, unsigned short maxlen)
{
  if (maxlen < 14) return 0;
  return (unsigned short)sprintf((char*)buf, "\033[%d;%dH", y+1, x+1);
}

/*-----------------------------------------------------------------------------------*/
/** \internal
 * Adds a attribute string to buffer. Returns 0 if string doesn't fit else
 * number of bytes actually written is returned.
 *
 * \param c Color number
 * \param buf Output buffer
 * \param maxlen Maximum number of bytes to store in buffer
 */
/*-----------------------------------------------------------------------------------*/
static unsigned short
set_color(unsigned char c, unsigned char* buf, unsigned short maxlen)
{
  int len = strlen((const char*)colortheme[c]);
  if (maxlen < len) return 0;
  memcpy(buf, colortheme[c], len);
  return len;
}

/*-----------------------------------------------------------------------------------*/
/**
 * Stores terminal data in buffer provided by caller. Returns number of bytes written
 * to the output buffer.
 *
 * \param ts State information
 * \param buf Output buffer
 * \param maxlen Maximum number of bytes to store in buffer
 */
/*-----------------------------------------------------------------------------------*/
unsigned short 
ctk_term_send(struct ctk_term_state* ts, 
	      unsigned char* buf, 
	      unsigned short maxlen)
{
  unsigned char x, y, x0;
  unsigned char col, c;
  unsigned short tmp;
  unsigned short totlen;
  
  check_updates(ts);
  
  if (ts->updates_current == NULL) return 0;

  x0 = ts->x1;
  col = ts->c1;
  totlen = 0;
  /* Loop across the update region starting at (x1,y1) */
  for(y = ts->y1; y < ts->y + ts->h; ++y) {
    for(x = x0; x < ts->x + ts->w; ++x) {
      /* New line ? */
      if (x == ts->x) {
	/* Move cursor to start of line */
	tmp = move_to(x,y,buf,maxlen);
	if (tmp == 0) goto loopend;
	buf += tmp;
	totlen += tmp;
	maxlen -= tmp;
      }
      /* Check color */
      c = colorscreen[x + y * CHARS_WIDTH];
      if (c != col) {
	PRINTF(("colorchange at (%d, %d) to %d\n", x,y,c));
	/* Send new color information */
	tmp = set_color(c, buf, maxlen);
	if (tmp == 0) goto loopend;
	col = c;
	buf += tmp;
	totlen += tmp;
	maxlen -= tmp;
      }
      /* Check remaining space */
      if (maxlen < 1) goto loopend;
      /* Add character */
      *buf = screen[x + y * CHARS_WIDTH];
      buf++;
      maxlen--;
      totlen++;
    }
    x0 = ts->x;
  }
loopend:
  /* Always save current color state */
  ts->c2 = col;  
  PRINTF(("ending loop at (%d, %d)\n", x,y));
  /* Check if done */
  if (x == ts->x+ts->w && y == ts->y+ts->h) {
    /* Signal done with this update */
    ts->x2 = ts->y2 = 0;
  }
  else {
    /* Not done. Save state */
    ts->x2 = x;
    ts->y2 = y;
  }
  return totlen;
}

/*-----------------------------------------------------------------------------------*/
/**
 * Called by client when the data returned by ctk_term_send() are successfully sent.
 *
 * \param ts State information
 */
/*-----------------------------------------------------------------------------------*/
void ctk_term_sent(struct ctk_term_state* ts)
{
  if (ts->updates_current != NULL) {
    /* Check if current update done */
    if (ts->x2 == 0 && ts->y2 == 0) {
      /* Yes, free it */
      ctk_term_update_free(ts, ts->updates_current);
      ts->updates_current = NULL;
    }
    else {
      /* Nop. Update start posititions */
      ts->x1 = ts->x2;
      ts->y1 = ts->y2;
    }
    ts->c1 = ts->c2;
  }
}
