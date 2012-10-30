/*
 * Copyright (c) 2001, Adam Dunkels
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
 * This file is part of the uIP TCP/IP stack.
 *
 *
 */

#include "contiki-net.h"
#include "ctk/vnc-server.h"
#include "ctk/vnc-out.h"
#include "ctk/ctk-vncfont.h"

#include "ctk/ctk-mouse.h"

#include "lib/libconio.h"

#ifdef WITH_AVR
#include <avr/pgmspace.h>
#else
#define memcpy_P memcpy
#endif /* WITH_AVR */

#define CHARS_WIDTH    LIBCONIO_CONF_SCREEN_WIDTH
#define CHARS_HEIGHT   LIBCONIO_CONF_SCREEN_HEIGHT

#define SCREEN_X       10
#define SCREEN_Y       8

#define SCREEN_WIDTH  (CHARS_WIDTH * CTK_VNCFONT_WIDTH + 2 * SCREEN_X) /*420*/
#define SCREEN_HEIGHT (CHARS_HEIGHT * CTK_VNCFONT_HEIGHT + 2 * SCREEN_Y) /*300*/
#define BORDER_COLOR 0x00
#define SCREEN_COLOR 0x00 /*0xc0*/

#ifndef CH_HOME
#define CH_HOME 0x50
#endif 

#ifndef CH_TAB
#define CH_TAB  0x09
#endif


#define BGR(b,g,r) (((b) << 6) | (g) << 3 | (r))


static const uint8_t menucolor[] = {
  BGR(3,7,7), /* Background. */           
  BGR(2,6,6), /* Anti-alias font color. */ 
  BGR(0,0,0), /* Font color. */            
};


static const uint8_t activemenucolor[] = {
  BGR(0,0,0), /* Background. */           
  BGR(2,5,5), /* Anti-alias font color. */ 
  BGR(3,7,7), /* Font color. */            
};

#define W  BGR(3,7,7)
#define B  BGR(0,0,0)
#define G0 BGR(0,2,2)
#define G1 BGR(1,2,2)
#define G2 BGR(1,3,3)
#define G3 BGR(2,4,4)
#define G4 BGR(2,5,5)
#define G5 BGR(2,6,6)

#define BG BGR(3,4,4)

static const unsigned char backgroundcolor[] = {BG};

static const unsigned char wincol[] =
  {BGR(2,5,5),BGR(2,2,2),BGR(0,1,1),G2,G3,G4};
  /*  {BGR(2,5,5),BGR(2,2,2),BGR(0,1,1),BGR(1,0,0),BGR(2,0,0),BGR(2,1,1)}; */
static const unsigned char wincol_f[] =
  {BGR(3,7,7),BGR(1,2,2),BGR(0,1,1),G4,G5,W};
  /*  {BGR(3,7,7),BGR(1,2,2),BGR(0,1,1),BGR(2,0,0),BGR(3,2,2),BGR(3,4,4)}; */
static const unsigned char wincol_d[] =
  {BGR(3,7,7),BGR(1,5,5),BGR(0,0,0),BGR(2,0,0),BGR(3,2,2),BGR(3,4,4)};

static const unsigned char sepcol[] =
 {BGR(2,5,5),BGR(2,6,6),BGR(3,6,6)};
static const unsigned char sepcol_f[] =
 {BGR(3,7,7),BGR(3,5,5),BGR(2,5,5)};
static const unsigned char sepcol_d[] =
 {BGR(3,7,7),BGR(1,5,7),BGR(0,0,0)};

static const unsigned char labcol[] =
 {BGR(2,5,5),BGR(1,3,3),BGR(0,1,1)};
static const unsigned char labcol_f[] =
 {BGR(3,7,7),BGR(3,6,6),BGR(0,0,0)};
static const unsigned char labcol_d[] =
 {BGR(3,7,7),BGR(3,6,6),BGR(0,0,0)};


static const unsigned char butcol[] =
 {BGR(2,4,4),BGR(1,3,3),BGR(0,1,1),BGR(2,4,4),BGR(2,4,4),BGR(2,4,4),
  BGR(2,5,5),BGR(2,5,5)};
static const unsigned char butcol_w[] =
 {BGR(2,4,4),BGR(1,3,3),BGR(0,1,1),BGR(2,4,4),BGR(2,4,4),BGR(2,4,4),
  BGR(2,5,5),BGR(2,5,5)};
static const unsigned char butcol_f[] =
 {G5,G4,B,BGR(3,5,5),BGR(3,6,6),BGR(3,7,7),
  BGR(3,6,6),BGR(2,5,5)};
static const unsigned char butcol_fw[] =
 {BGR(3,7,7),BGR(3,6,6),BGR(0,0,0),BGR(1,3,3),BGR(2,7,7),BGR(3,7,7),
  BGR(3,6,6),BGR(3,7,7)};
static const unsigned char butcol_d[] =
 {BGR(2,3,3),BGR(2,5,5),BGR(3,6,6),BGR(1,3,4),BGR(1,5,6),BGR(2,6,7),
  BGR(3,7,7),BGR(2,5,5)};
static const unsigned char butcol_dw[] =
 {BGR(0,0,0),BGR(2,5,5),BGR(3,7,7),BGR(1,3,4),BGR(1,5,6),BGR(2,6,7),
  BGR(3,7,7),BGR(2,5,5)};


static const unsigned char hlcol[] =
 {BGR(2,5,5),BGR(1,3,3),BGR(1,0,0)};
static const unsigned char hlcol_w[] =
 {BGR(2,5,5),BGR(1,3,3),BGR(1,0,0)};
static const unsigned char hlcol_f[] =
 {BGR(3,7,7),BGR(3,5,5),BGR(3,0,0)};
static const unsigned char hlcol_fw[] =
 {BGR(3,7,7),BGR(3,6,7),BGR(3,7,7)};
static const unsigned char hlcol_d[] =
 {BGR(3,7,7),BGR(3,5,5),BGR(2,0,0)};
static const unsigned char hlcol_dw[] =
 {BGR(3,7,7),BGR(1,5,5),BGR(0,0,0)};

static const unsigned char iconcol[] =
  {BG,G4,W,B,G1};
static const unsigned char iconcol_w[] =
 {BGR(0,1,1),BGR(1,3,3),BGR(3,7,7), B,W};



static const uint8_t * const colortheme[] =
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


static int mouse_x, mouse_y, mouse_button;

#ifdef CTK_VNCSERVER_CONF_SCREEN
static uint8_t *screen = CTK_VNCSERVER_CONF_SCREEN;
#else
static uint8_t screen[CHARS_WIDTH * CHARS_HEIGHT];
#endif

#ifdef CTK_VNCSERVER_CONF_COLORSCREEN
staitc uint8_t *colorscreen = CTK_VNCSERVER_CONF_COLORSCREEN;
#else
static uint8_t colorscreen[CHARS_WIDTH * CHARS_HEIGHT];
#endif


#define PRINTF(x)

/*-----------------------------------------------------------------------------------*/
#define MAX_ICONS CTK_VNCSERVER_CONF_MAX_ICONS
struct ctk_icon *icons[MAX_ICONS];

unsigned char
vnc_out_add_icon(struct ctk_icon *icon)
{
  uint8_t i;
  signed int empty;

  empty = -1;
  for(i = 0; i < MAX_ICONS; ++i) {
    if(icon == icons[i]) {
      return i;
    }
    if(icons[i] == NULL && empty < 0){
      empty = i;
    }
  }

  if(empty == -1) {
    empty = 0;
  }
  icons[empty] = icon;
  return empty;
}

/*-----------------------------------------------------------------------------------*/
void
vnc_out_init(void)
{
  uint16_t i;
  for(i = 0; i < CHARS_WIDTH * CHARS_HEIGHT; ++i) {
    screen[i] = 0x20;
  }
}

void
vnc_out_update_screen(uint8_t xpos, uint8_t ypos, uint8_t c, uint8_t color)
{
  screen[xpos + ypos * CHARS_WIDTH] = c;
  colorscreen[xpos + ypos * CHARS_WIDTH] = color;
}
/*-----------------------------------------------------------------------------------*/
void
vnc_out_update_area(struct vnc_server_state *vs,
		    uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  uint8_t x2, y2, ax2, ay2;
  register struct vnc_server_update *a, *b;

  PRINTF(("update_area_connection: should update (%d:%d) (%d:%d)\n",
	 x, y, w, h));
  
  /* First check if we already have a full update queued. If so, there
     is no need to put this update on the list. If there is a full
     update, it is always the first one on the list, so there is no
     need to go step the list in search for it. */

  if(vs->updates_pending != NULL &&
     vs->updates_pending->type == VNC_SERVER_UPDATE_FULL) {
    PRINTF(("Update_area_connecion: full update already queued...\n"));
    return;
  }

 again:
  
  /* Check that we don't update the same area twice by going through
     the list and search for an update with the same coordinates. */
  for(a = vs->updates_pending; a != NULL; a = a->next) {
    if(a->x == x && a->y == y &&
       a->w == w && a->h == h) {
      PRINTF(("Update_area_connecion: found equal area\n"));
      return;
    }    
  }

  /* Next we check if this update covers an existing update. If so, we
     remove the old update, expand this update so that it covers both
     areas to be updated and run through the process again. */
  b = NULL;
  for(a = vs->updates_pending; a != NULL; a = a->next) {      
    x2 = x + w;
    y2 = y + h;
    
    ax2 = a->x + a->w;
    ay2 = a->y + a->h;

    /* Test the corners of both updates to see if they are inside the
       other area. */
#define INSIDE(x,y,x1,y1,x2,y2) ((x1) <= (x) && \
                                 (x2) >= (x) && \
                                 (y1) <= (y) && \
                                 (y2) >= (y))
    if(INSIDE(x, y, a->x, a->y, ax2, ay2) ||
       INSIDE(x, y2, a->x, a->y, ax2, ay2) ||
       INSIDE(x2, y2, a->x, a->y, ax2, ay2) ||
       INSIDE(x2, y, a->x, a->y, ax2, ay2) ||
       INSIDE(a->x, a->y, x, y, x2, y2) ||
       INSIDE(a->x, ay2, x, y, x2, y2) ||
       INSIDE(ax2, ay2, x, y, x2, y2) ||
       INSIDE(ax2, a->y, x, y, x2, y2)) {

      /* Remove the old update from the list. */
      vnc_server_update_remove(vs, a);

      /* Put it on the free list. */
      vnc_server_update_free(vs, a);

      PRINTF(("update_area_connection: inside (%d:%d, %d:%d)\n",
	     a->x, a->y, ax2, ay2));
      
      /* Find the area that covers both updates. */
#define MIN(a,b) ((a) < (b)? (a): (b))
#define MAX(a,b) ((a) > (b)? (a): (b))
      x = MIN(a->x, x);
      y = MIN(a->y, y);
      ax2 = MAX(ax2, x2);
      ay2 = MAX(ay2, y2);
      w = ax2 - x;
      h = ay2 - y;

      /* This should really be done by a recursive call to this
	 function: update_area_connection(vs, x, y, w, h); but because
	 some compilers might not be able to optimize away the
	 recursive call, we do it using a goto instead. */
      PRINTF(("Update_area_connecion: trying larger area (%d:%d) (%d:%d)\n", x, y, w, h));
      goto again;
    }
    if(b != NULL) {
      b = b->next;
    }
  }
  
  /* Allocate an update object by pulling it off the free list. If
     there are no free objects, we go for a full update instead. */

  /*  a = vs->updates_free;*/
  a = vnc_server_update_alloc(vs);
  if(a == NULL) {
    PRINTF(("Update_area_connecion: no free updates, doing full\n"));
    /* Put all pending updates, except for one, on the free list. Use
       the remaining update as a full update. */
    while(vs->updates_pending != NULL) {
      a = vs->updates_pending;
      vnc_server_update_remove(vs, a);
      vnc_server_update_free(vs, a);
    }

    a = vnc_server_update_alloc(vs);
    a->type = VNC_SERVER_UPDATE_FULL;
    vnc_server_update_add(vs, a);
					

  } else {
    
    PRINTF(("Update_area_connecion: allocated update for (%d:%d) (%d:%d)\n", x, y, w, h));
  /* Else, we put the update object at the end of the pending
     list. */
    a->type = VNC_SERVER_UPDATE_PARTS;
    a->x = x;
    a->y = y;
    a->w = w;
    a->h = h;
    vnc_server_update_add(vs, a);
  }
}
/*-----------------------------------------------------------------------------------*/
static void
init_send_screen(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  vs->sendmsg = SEND_SCREEN;
  vs->x = vs->y = 0;
  vs->x1 = vs->y1 = 0;
  vs->x2 = vs->y2 = 0;
  vs->w = CHARS_WIDTH;
  vs->h = CHARS_HEIGHT;
}
/*-----------------------------------------------------------------------------------*/
static void
check_updates(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  
  if(vs->state == VNC_RUNNING &&
     vs->sendmsg == SEND_NONE &&
     vs->updates_current == NULL) {
    if(vs->updates_pending != NULL &&
       vs->update_requested != 0) {
      vs->update_requested = 0;
      /*      vs->updates_current = vs->updates_pending;
      vs->updates_pending = vs->updates_pending->next;
      vs->updates_current->next = NULL;*/

      vs->updates_current = vnc_server_update_dequeue(vs);
      
      if(vs->updates_current->type == VNC_SERVER_UPDATE_PARTS) {
	vs->x = vs->x1 = vs->x2 = vs->updates_current->x;
	vs->y = vs->y1 = vs->y2 = vs->updates_current->y;
	vs->w = vs->updates_current->w;
	vs->h = vs->updates_current->h;
	vs->sendmsg = SEND_UPDATE;
	
	PRINTF(("New update from (%d:%d) (%d:%d) to (%d:%d)\n",
	       vs->x, vs->y, vs->x1, vs->y1, vs->x + vs->w,
	       vs->y + vs->h));
      } else if(vs->updates_current->type == VNC_SERVER_UPDATE_FULL) {
	init_send_screen(vs);
	PRINTF(("New full update\n"));
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static uint8_t tmp[CTK_VNCFONT_WIDTH * CTK_VNCFONT_HEIGHT];
static void
makechar(CC_REGISTER_ARG char *ptr, uint8_t x, uint8_t y)
{
  uint8_t i, *tmpptr;
  register uint8_t *colorscheme;
  unsigned char *bitmap;
  uint8_t b, b2;
  uint8_t xmove, ymove;
  unsigned char c, color;

  color = colorscreen[x + y * CHARS_WIDTH];
  c = screen[x + y * CHARS_WIDTH];

  colorscheme = (uint8_t *)colortheme[color];
      
  /* First check if the character is a special icon character. These
     are to be interpreted in a special manner: the first character of
     the icon (the top left corner) has the highest bit set, but not
     bit 6. All other characters have bit 6 set, and also count the
     number of positions away from the top left corner. Only the top
     left corner contains enough information to identify the icon, all
     other chars only contain the number of steps to reach the
     identifying icon. */
  if((c & 0x80) != 0) {
    xmove = c & 0x0f;
    ymove = (c & 0x30) >> 4;
    
    c = colorscreen[x + y * CHARS_WIDTH];

    if(icons[c % MAX_ICONS] == NULL) {
      c = 0;
    }
    bitmap = icons[c % MAX_ICONS]->bitmap;

    if(bitmap != NULL) {
      bitmap = bitmap + ymove * 8*3;
      colorscheme = (uint8_t *)colortheme[VNC_OUT_ICONCOLOR + (c >> 6)];
      switch(xmove) {
      case 0:
	for(i = 0; i < CTK_VNCFONT_HEIGHT; ++i) {
	  b = bitmap[i];
	  *ptr++ = colorscheme[((b >> 7) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 6) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 5) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 4) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 3) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 2) & 0x01) << 2];		
	}
	break;
      case 1:
	for(i = 0; i < CTK_VNCFONT_HEIGHT; ++i) {
	  b = bitmap[i];
	  b2 = bitmap[i + 8];
	  *ptr++ = colorscheme[((b >> 1) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 0) & 0x01) << 2];
	  *ptr++ = colorscheme[((b2 >> 7) & 0x01) << 2];
	  *ptr++ = colorscheme[((b2 >> 6) & 0x01) << 2];
	  *ptr++ = colorscheme[((b2 >> 5) & 0x01) << 2];
	  *ptr++ = colorscheme[((b2 >> 4) & 0x01) << 2];		
	}
	break;
      case 2:
	for(i = 0; i < CTK_VNCFONT_HEIGHT; ++i) {
	  b = bitmap[i + 8];
	  b2 = bitmap[i + 16];
	  *ptr++ = colorscheme[((b >> 3) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 2) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 1) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 0) & 0x01) << 2];
	  *ptr++ = colorscheme[((b2 >> 7) & 0x01) << 2];
	  *ptr++ = colorscheme[((b2 >> 6) & 0x01) << 2];		
	}
	break;
      case 3:
	for(i = 0; i < CTK_VNCFONT_HEIGHT; ++i) {
	  b = bitmap[i + 16];
	  *ptr++ = colorscheme[((b >> 5) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 4) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 3) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 2) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 1) & 0x01) << 2];
	  *ptr++ = colorscheme[((b >> 0) & 0x01) << 2];		
	}
	break;
      }
    }
  } else {
    memcpy_P(tmp, &ctk_vncfont[c * (CTK_VNCFONT_WIDTH * CTK_VNCFONT_HEIGHT)],
	     CTK_VNCFONT_WIDTH * CTK_VNCFONT_HEIGHT);
    
    tmpptr = tmp;

    
    for(i = 0; i < CTK_VNCFONT_HEIGHT * CTK_VNCFONT_WIDTH; ++i) {
      *ptr++ = colorscheme[*tmpptr++];
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_out_new(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  uint8_t i;
  
  vs->width = SCREEN_WIDTH;
  vs->height = SCREEN_HEIGHT;
  vs->x = vs->y = vs->x1 = vs->y1 = vs->x2 = vs->y2 = 0;
  vs->w = CHARS_WIDTH;
  vs->h = CHARS_HEIGHT;

  /* Initialize the linked list of updates. */
  for(i = 0; i < VNC_SERVER_MAX_UPDATES - 1; ++i) {
    vs->updates_pool[i].next = &vs->updates_pool[i + 1];    
  }
  vs->updates_pool[VNC_SERVER_MAX_UPDATES].next = NULL;

  vs->updates_free = &vs->updates_pool[0];
  vs->updates_pending = vs->updates_current = NULL;
}
/*-----------------------------------------------------------------------------------*/
void
vnc_out_send_blank(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  register struct rfb_fb_update *umsg;
  uint8_t *ptr;
  uint16_t len;
  uint8_t msglen;
      
  vs->x = vs->y = 0;
  vs->x2 = vs->y2 = 0;
	  
  umsg = (struct rfb_fb_update *)uip_appdata;
  
  umsg->type = RFB_FB_UPDATE;
  umsg->rects = UIP_HTONS(2);

  ptr = (uint8_t *)umsg + sizeof(struct rfb_fb_update);
  len = sizeof(struct rfb_fb_update);
  
  msglen = vnc_server_draw_rect(ptr, 0, 0,
				UIP_HTONS(SCREEN_WIDTH),
				UIP_HTONS(SCREEN_HEIGHT),
				BORDER_COLOR);

  
  ptr += msglen;
  len += msglen;

  msglen = vnc_server_draw_rect(ptr,
				UIP_HTONS(SCREEN_X), UIP_HTONS(SCREEN_Y),
				UIP_HTONS(SCREEN_WIDTH - SCREEN_X * 2),
				UIP_HTONS(SCREEN_HEIGHT - SCREEN_Y * 2),
				SCREEN_COLOR);

  uip_send(uip_appdata, len + msglen);

  vs->sendmsg = SENT_BLANK;
}
/*-----------------------------------------------------------------------------------*/
void
vnc_out_send_screen(struct vnc_server_state *vs)
{
  vnc_out_send_update(vs);
}
/*-----------------------------------------------------------------------------------*/
static short tmpbuf[30];
void
vnc_out_send_update(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  uint8_t x, y, x0;
  uint8_t msglen;
  uint16_t len, n;
  uint8_t *ptr;
  struct rfb_fb_update *umsg;
  register struct rfb_fb_update_rect_hdr *recthdr;
  struct rfb_rre_hdr *rrehdr;
  uint8_t c, color, lastcolor;
  uint8_t numblanks;

  /* First, check if we need to feed the update function with a new
     pending update. */
  check_updates(vs);

  /*  PRINTF(("Sending Update from (%d:%d) (%d:%d) to (%d:%d)\n",
	 vs->x, vs->y, vs->x1, vs->y1, vs->x + vs->w,
	 vs->y + vs->h));*/
  
  umsg = (struct rfb_fb_update *)uip_appdata;

  umsg->type = RFB_FB_UPDATE;

  x0 = vs->x1;
  n = 0;
  msglen = 0;
  ptr = (uint8_t *)umsg + sizeof(struct rfb_fb_update);
  len = sizeof(struct rfb_fb_update);
  
  /* Loop over all characters that are covered by this update. */
  for(y = vs->y1; y < vs->y + vs->h; ++y) {
    for(x = x0; x < vs->x + vs->w; ++x) {


      /* First check if there are any blank space characters, and if
	 so, find out how many of them there are in a row. Instead of
	 sending the individual space characters as raw bitmaps, we
	 can send the entire string of blanks as a single color
	 rectangle instead. */
      
      c = screen[x + y * CHARS_WIDTH];
      numblanks = 0;
      lastcolor = color = colorscreen[x + y * CHARS_WIDTH];

      /* If the character is a blank, we continue reading characters
	 until we find one that has a different color, or one that is
	 not a blank. We must keep within the update rectangle, so we
	 make sure that the "x" variable does not increase beyond the
	 edge. The "numblanks" variable is used to keep track of how
	 many blank characters we have found. */
      while(lastcolor == color &&
	    c == 0x20 &&
	    x < vs->x + vs->w) {
	++numblanks;

	
	++x;
	lastcolor = color;
	color = colorscreen[x + y * CHARS_WIDTH];
	c = screen[x + y * CHARS_WIDTH];
      }

      if(numblanks > 0) {
	
	/*	PRINTF(("Found %d blanks (%d:%d -> %d:%d)\n",
		numblanks, x - numblanks, y, x, y));*/
	
	/* There were one or more blank characters, so we send out a
	   single color rectangle with the right width. But first we
	   make sure that there is enough space in the current TCP
	   segment to put the rectangle. If there isn't we have to
	   backtrack the "x" variable to where we found the first
	   blank character so that the next TCP segment will be able
	   to update this area instead. */	

	msglen = sizeof(struct rfb_fb_update_rect_hdr) +
	  /*sizeof(struct rfb_rre_hdr)*/5;

	if(msglen >= uip_mss() - len) {
	  /*	  PRINTF(("Not enouch space for blanks (%d, left %d)\n",
		  msglen, uip_mss() - len));*/
	  /* There is not enough space in the segment, so we remember
	     where we were ... */
	  vs->x2 = x - numblanks;
	  vs->y2 = y;

	  /* ... and we break out of the loop. */
	  goto loopend;	  
	}

	/* We construct a rectangle with the right width and color. */
	/*	recthdr = (struct rfb_fb_update_rect_hdr *)ptr;*/
	recthdr = (struct rfb_fb_update_rect_hdr *)tmpbuf;
	rrehdr = (struct rfb_rre_hdr *)((char *)recthdr +
		 sizeof(struct rfb_fb_update_rect_hdr));

	/*	PRINTF(("Blankign (%d:%d) to (%d:%d)\n",
	       (x - numblanks) * CTK_VNCFONT_WIDTH,
	       y * CTK_VNCFONT_HEIGHT,
	       CTK_VNCFONT_WIDTH * numblanks,
	       CTK_VNCFONT_HEIGHT));*/
	recthdr->rect.x = uip_htons(SCREEN_X + (x - numblanks) *
				CTK_VNCFONT_WIDTH);
	recthdr->rect.y = uip_htons(SCREEN_Y + y * CTK_VNCFONT_HEIGHT);
	recthdr->rect.w = uip_htons(CTK_VNCFONT_WIDTH * numblanks);
	recthdr->rect.h = UIP_HTONS(CTK_VNCFONT_HEIGHT);
	recthdr->encoding[0] =
	  recthdr->encoding[1] =
	  recthdr->encoding[2] = 0;
	recthdr->encoding[3] = RFB_ENC_RRE;
	
	rrehdr->subrects[0] =
	  rrehdr->subrects[1] = 0;
	rrehdr->bgpixel = colortheme[lastcolor][0];

	--x;
      } else {

	/* So there were no blank characters. */

	/*	PRINTF(("An char at (%d:%d)\n", x, y));*/
	/* First we must make sure that there is enough space in the
	   outgoing TCP segment. */

	msglen = sizeof(struct rfb_fb_update_rect_hdr) +
	  CTK_VNCFONT_HEIGHT * CTK_VNCFONT_WIDTH;
	if(msglen >= uip_mss() - len) {
	  /*	  PRINTF(("Not enouch space for char (%d, left %d)\n",
		  msglen, uip_mss() - len));*/
	  
	  /* There is not enough space in the segment, so we remember
	     where we were ... */
	  vs->x2 = x;
	  vs->y2 = y;

	  /* ... and we break out of the loop. */
	  goto loopend;	  
	}

	/*	PRINTF(("ptr %p\n",ptr);*/
	/*	recthdr = (struct rfb_fb_update_rect_hdr *)ptr;*/
	recthdr = (struct rfb_fb_update_rect_hdr *)tmpbuf;

	recthdr->rect.x = uip_htons(SCREEN_X + x * CTK_VNCFONT_WIDTH);
	recthdr->rect.y = uip_htons(SCREEN_Y + y * CTK_VNCFONT_HEIGHT);
	recthdr->rect.w = UIP_HTONS(CTK_VNCFONT_WIDTH);
	recthdr->rect.h = UIP_HTONS(CTK_VNCFONT_HEIGHT);
	recthdr->encoding[0] =
	  recthdr->encoding[1] =
	  recthdr->encoding[2] = 0;
	recthdr->encoding[3] = RFB_ENC_RAW;

	makechar((uint8_t *)recthdr +
		 sizeof(struct rfb_fb_update_rect_hdr),
		 x, y);
      }
      memcpy(ptr, tmpbuf, msglen);
      PRINTF(("Msglen %d (%d:%d)\n", msglen, x, y));
      len += msglen;
      ptr += msglen;
      ++n;
    }
    x0 = vs->x;
  }
  
 loopend:

  umsg->rects = uip_htons(n);
	
  if(y == vs->y + vs->h && x == vs->x + vs->w) {
    vs->x2 = vs->y2 = 0;
  }

  if(n > 0) {
    /*    printf("Sending %d rects, %d bytes (%p, %p, %p)\n", n, len,
	  uip_appdata, umsg, ptr);*/
    uip_send(uip_appdata, len);
  }

}
/*-----------------------------------------------------------------------------------*/
#define NUMKEYS 20
static char keys[NUMKEYS];
static int firstkey, lastkey;


char
vnc_out_keyavail(void)
{
  return firstkey != lastkey;
}

char
vnc_out_getkey(void)
{
  char key;
  key = keys[firstkey];
  
  if(firstkey != lastkey) {
    ++firstkey;
    if(firstkey >= NUMKEYS) {
      firstkey = 0;
    }
  }
  
  return key;  
}

void
vnc_out_key_event(struct vnc_server_state *vs)
{
  register struct rfb_key_event *ev;
  
  ev = (struct rfb_key_event *)uip_appdata;

  if(ev->down != 0) {
    if(vs->sendmsg == SEND_NONE) {
      vs->sendmsg = SEND_UPDATE;
    }


    if(ev->key[2] == 0 ||
       (ev->key[2] == 0xff &&	
	(ev->key[3] == CH_HOME ||
	 ev->key[3] == CH_TAB ||
	 ev->key[3] == CH_ESC ||
	 ev->key[3] == CH_DEL ||
	 ev->key[3] == CH_ENTER ||
	 ev->key[3] == CH_CURS_LEFT ||
	 ev->key[3] == CH_CURS_UP ||
	 ev->key[3] == CH_CURS_RIGHT ||
	 ev->key[3] == CH_CURS_DOWN))) {
      
      keys[lastkey] = ev->key[3];
      ++lastkey;
      if(lastkey >= NUMKEYS) {
	lastkey = 0;
      }
    }
  }

  check_updates(vs);
}
/*-----------------------------------------------------------------------------------*/
void
vnc_out_pointer_event(struct vnc_server_state *vs)
{
  struct rfb_pointer_event *ev;
  uint16_t evx, evy;
  
  ev = (struct rfb_pointer_event *)uip_appdata;

  evx = uip_htons(ev->x);
  evy = uip_htons(ev->y);
  
  if(evx > SCREEN_X && evx < SCREEN_WIDTH - 2 * SCREEN_X &&
     evy > SCREEN_Y && evy < SCREEN_HEIGHT - 2 * SCREEN_Y) {

    mouse_button = ev->buttonmask & RFB_BUTTON_MASK1;
    
    mouse_x = evx - SCREEN_X;
    mouse_y = evy - SCREEN_Y;

    check_updates(vs);    
  }    
}
/*-----------------------------------------------------------------------------------*/
void
vnc_out_acked(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  if(vs->state != VNC_RUNNING) {
    return;
  }
  if(vs->sendmsg == SENT_BLANK) {
    init_send_screen(vs);
  } else if(vs->sendmsg == SEND_BLANK) {
    /* Do nothing until sendmsg == SENT_BLANK. */
  } else if(vs->sendmsg == SEND_SCREEN) {
    /* When the screen has been fully drawn, ->x2 and ->y2 are both
       set to 0 to indicate this.*/
    if(vs->x2 == 0 && vs->y2 == 0) {
      vs->sendmsg = SEND_NONE;
      
      /* If there was an updaterequest for the entire screen, we can
	 clear that flag now. */
      if(vs->updates_current != NULL) {	
	vnc_server_update_free(vs, vs->updates_current);
	vs->updates_current = NULL;
      }
      check_updates(vs);
    } else {
      vs->x1 = vs->x2;
      vs->y1 = vs->y2;
    }
	      
  } else if(vs->sendmsg == SEND_UPDATE) {
    if(vs->x2 == 0 && vs->y2 == 0) {
      /* So, we have updated the area that we needed. We now check if
	 there have been any recent full screen update requests. If
	 so, we need to go to the SEND_SCREEN state. Else, we see if
	 there were more areas that needed to be updated and if so,
	 we'll continue with those. */

      vs->sendmsg = SEND_NONE;

      if(vs->updates_current != NULL) {
	vnc_server_update_free(vs, vs->updates_current);
	vs->updates_current = NULL;

      }
      check_updates(vs);
#if 0
      if(vs->updaterequest == VNC_SERVER_UPDATE_FULL) {
	check_updates(vs);
      } else {
	vs->updatesptr2 = (vs->updatesptr2 + 1) %
	  VNC_SERVER_MAX_UPDATES;

	/* If there are no more updates to do, we'll go back to the
	   SEND_NONE state. */	
	if(vs->updatesptr2 == vs->updatesptr) {
	  vs->updatetype = VNC_SERVER_UPDATE_NONE;
	} else {	 
	  /* Otherwise, we continue to update the next area. */
	  vs->updaterequest = VNC_SERVER_UPDATE_PARTS;
	  check_updates(vs);
	}
      }
#endif /* 0 */
    } else {	     
      vs->x1 = vs->x2;
      vs->y1 = vs->y2;
    }    
  } else {
    vs->sendmsg = SEND_NONE;
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_out_poll(struct vnc_server_state *vs)
{
  /*  PRINTF(("vs->state %d, sendmsg %d, updatetype %d, updatereq %d\n",
      vs->state, vs->sendmsg, vs->updatetype, vs->updaterequest);*/
  
  if(vs->state == VNC_RUNNING &&
     vs->sendmsg == SEND_NONE) {
    check_updates(vs);
    vnc_server_send_data(vs);
  }
}
/*-----------------------------------------------------------------------------------*/
#if CTK_CONF_MOUSE_SUPPORT
void
ctk_mouse_init(void)
{

}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_x(void)
{
  return mouse_x;
}
/*-----------------------------------------------------------------------------------*/
unsigned short
ctk_mouse_y(void)
{
  return mouse_y;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_mouse_button(void)
{
  return mouse_button;
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
#endif /* CTK_CONF_MOUSE_SUPPORT */
