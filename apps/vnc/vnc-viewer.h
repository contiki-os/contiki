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
 * This file is part of the Contiki VNC client.
 *
 * $Id: vnc-viewer.h,v 1.2 2007/08/30 14:39:17 matsutsuka Exp $
 *
 */

#ifndef __VNC_VIEWER_H__
#define __VNC_VIEWER_H__


struct vnc_viewer_state;

/*typedef unsigned long uint32_t;*/

void vnc_viewer_init(void);
void vnc_viewer_appcall(void *nullptr);

void vnc_viewer_connect(uint16_t *server, uint8_t display);
void vnc_viewer_close(void);

/* Callback: redraws the VNC viewer bitmap area. */
void vnc_viewer_refresh(void);

#define VNC_POINTER_EVENT RFB_POINTER_EVENT
#define VNC_KEY_EVENT     RFB_KEY_EVENT
#define VNC_UPDATERQ_EVENT 7

#define VNC_VIEWER_POST_POINTER_EVENT(x, y, button) \
        vnc_viewer_post_event(VNC_POINTER_EVENT, button, x, y, 0)

#define VNC_VIEWER_POST_KEY_EVENT(key) \
        vnc_viewer_post_event(VNC_KEY_EVENT, key, 0, 0, 0)

#define VNC_VIEWER_POST_UPDATERQ_EVENT(x1,y1,x2,y2) \
        vnc_viewer_post_event(VNC_UPDATERQ_EVENT, x1, y1, x2, y2)

void vnc_viewer_post_event(uint8_t event,
			   uint16_t data1, uint16_t data2,
			   uint16_t data3, uint16_t data4);

/* UIP_APPCALL: the name of the application function. This function
   must return void and take no arguments (i.e., C type "void
   appfunc(void)"). */
#ifndef UIP_APPCALL
#define UIP_APPCALL     vnc_viewer_app
#endif

struct vnc_key_event {
  uint8_t down;
  uint16_t key;
};

struct vnc_pointer_event {
  uint8_t buttonmask;
  uint16_t x, y;
};

struct vnc_updaterq_event {
  uint16_t x, y;
  uint16_t w, h;
};

struct vnc_event {
  uint8_t type;
  union {
    struct vnc_key_event key;
    struct vnc_pointer_event ptr;
    struct vnc_updaterq_event urq;
  } ev;
};

enum vnc_sendmsg {
  VNC_SEND_NONE,
  
  VNC_SEND_VERSION,
  VNC_SEND_AUTH,
  VNC_SEND_CINIT,
  VNC_SEND_PFMT,
  VNC_SEND_ENCODINGS,
  
  VNC_SEND_UPDATERQ,
  VNC_SEND_UPDATERQ_INC,
  VNC_SEND_EVENTS,
};

enum vnc_waitmsg {
  VNC_WAIT_NONE,
  
  VNC_WAIT_VERSION,
  VNC_WAIT_AUTH,
  VNC_WAIT_AUTH_RESPONSE,
  VNC_WAIT_SINIT,
  
  VNC_WAIT_UPDATE,
  VNC_WAIT_UPDATE_RECT,
  
};


enum vnc_rectstate {
  VNC_RECTSTATE_NONE,
  VNC_RECTSTATE_RAW,
  VNC_RECTSTATE_RRE,
};

struct vnc_viewer_state {
  uint8_t close;
  uint16_t w, h;
  
  uint8_t sendmsg;

  uint8_t waitmsg;
  
  uint16_t rectsleft;
  
  uint8_t rectstate;
  uint32_t rectstateleft;
  uint16_t rectstatex, rectstatey;
  uint16_t rectstateh, rectstatew;
  uint16_t rectstatex0, rectstatey0;
  uint16_t rectstatex2, rectstatey2;

  
  uint16_t eventptr_acked;
  uint16_t eventptr_unacked;
  uint16_t eventptr_next;
#define VNC_EVENTQUEUE_SIZE 32
  struct vnc_event event_queue[VNC_EVENTQUEUE_SIZE];
  
  
  uint16_t bufferleft;
  uint16_t buffersize;
#define VNC_BUFFERSIZE 64
  uint8_t buffer[VNC_BUFFERSIZE];
};

extern static struct vnc_viewer_state vnc_viewer_state;

/* Definitions of the RFB (Remote Frame Buffer) protocol
   structures and constants. */

#include "net/uipopt.h"


/* Generic rectangle - x, y coordinates, width and height. */
struct rfb_rect {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
};

/* Pixel format definition. */
struct rfb_pixel_format {
  uint8_t bps;       /* Bits per pixel: 8, 16 or 32. */
  uint8_t depth;     /* Color depth: 8-32 */
  uint8_t endian;    /* 1 - big endian (motorola), 0 - little endian
		     (x86) */
  uint8_t truecolor; /* 1 - true color is used, 0 - true color is not used. */

  /* The following fields are only used if true color is used. */
  uint16_t red_max, green_max, blue_max;
  uint8_t red_shift, green_shift, blue_shift;
  uint8_t pad1;
  uint16_t pad2;
};


/* RFB authentication constants. */

#define RFB_AUTH_FAILED      0
#define RFB_AUTH_NONE        1
#define RFB_AUTH_VNC         2

#define RFB_VNC_AUTH_OK      0
#define RFB_VNC_AUTH_FAILED  1
#define RFB_VNC_AUTH_TOOMANY 2

/* RFB message types. */

/* From server to client: */
#define RFB_FB_UPDATE            0
#define RFB_SET_COLORMAP_ENTRIES 1
#define RFB_BELL                 2
#define RFB_SERVER_CUT_TEXT      3

/* From client to server. */
#define RFB_SET_PIXEL_FORMAT     0
#define RFB_FIX_COLORMAP_ENTRIES 1
#define RFB_SET_ENCODINGS        2
#define RFB_FB_UPDATE_REQ        3
#define RFB_KEY_EVENT            4
#define RFB_POINTER_EVENT        5
#define RFB_CLIENT_CUT_TEXT      6

/* Encoding types. */
#define RFB_ENC_RAW      0
#define RFB_ENC_COPYRECT 1
#define RFB_ENC_RRE      2
#define RFB_ENC_CORRE    3
#define RFB_ENC_HEXTILE  4

/* Message definitions. */

/* Server to client messages. */

struct rfb_server_init {
  uint16_t width;
  uint16_t height;
  struct rfb_pixel_format format;
  uint8_t namelength[4];
  /* Followed by name. */
};

struct rfb_fb_update {
  uint8_t type;
  uint8_t pad;
  uint16_t rects; /* Number of rectanges (struct rfb_fb_update_rect_hdr +
		  data) that follows. */
};

struct rfb_fb_update_rect_hdr {
  struct rfb_rect rect;
  uint8_t encoding[4];
};

struct rfb_copy_rect {
  uint16_t srcx;
  uint16_t srcy;
};

struct rfb_rre_hdr {
  uint16_t subrects[2];  /* Number of subrectangles (struct
			 rfb_rre_subrect) to follow. */
  uint8_t bgpixel;
};

struct rfb_rre_subrect {
  uint8_t pixel;
  struct rfb_rect rect;
};

struct rfb_corre_rect {
  uint8_t x;
  uint8_t y;
  uint8_t w;
  uint8_t h;
};

/* Client to server messages. */

struct rfb_set_pixel_format {
  uint8_t type;
  uint8_t pad;
  uint16_t pad2;
  struct rfb_pixel_format format;
};

struct rfb_fix_colormap_entries {
  uint8_t type;
  uint8_t pad;
  uint16_t firstcolor;
  uint16_t colors;
};

struct rfb_set_encodings {
  uint8_t type;
  uint8_t pad;
  uint16_t encodings;
};

struct rfb_fb_update_request {
  uint8_t type;
  uint8_t incremental;
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
};

struct rfb_key_event {
  uint8_t type;
  uint8_t down;
  uint16_t pad;
  uint8_t key[4];
};

#define RFB_BUTTON_MASK1 1
#define RFB_BUTTON_MASK2 2
#define RFB_BUTTON_MASK3 4
struct rfb_pointer_event {
  uint8_t type;
  uint8_t buttonmask;
  uint16_t x;
  uint16_t y;
};

#endif /* __VNC_VIEWER_H__ */
