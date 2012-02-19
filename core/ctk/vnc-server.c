/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * $Id: vnc-server.c,v 1.3 2010/10/19 18:29:04 adamdunkels Exp $
 *
 */

/* A micro implementation of a VNC server. VNC is a protocol for
   remote network displays. See http://www.uk.research.att.com/vnc/
   for information about VNC.

   Initialization states:

   VNC_VERSION (send version string)
   VNC_AUTH    (send auth message)
   VNC_INIT    (send init message)

   Steady state:
   
   VNC_RUNNING (send RFB updates, parse incoming messages)

   What kind of message should be sent:

   SEND_NONE   (No message)
   SEND_BLANK  (Blank screen initially)
   SEND_SCREEN (Send entire screen, initially)
   SEND_UPDATE (Send incremental update)

*/

#include "contiki-net.h"
#include "ctk/vnc-server.h"
#include "ctk/vnc-out.h"

#include <string.h>

/* RFB server initial handshaking string. */
#define RFB_SERVER_VERSION_STRING rfb_server_version_string

/* "RFB 003.003" */
static uint8_t rfb_server_version_string[12] = {82,70,66,32,48,48,51,46,48,48,51,10};

/* uVNC */
static uint8_t uvnc_name[4] = {117,86,78,67};
#if 1
#define PRINTF(x)
#else
#define PRINTF(x) printf x
#endif

/*-----------------------------------------------------------------------------------*/
uint8_t
vnc_server_draw_rect(uint8_t *ptr, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t c)
{
  register struct rfb_fb_update_rect_hdr *recthdr;
  struct rfb_rre_hdr *rrehdr;

  recthdr = (struct rfb_fb_update_rect_hdr *)ptr;
  rrehdr = (struct rfb_rre_hdr *)(ptr + sizeof(struct rfb_fb_update_rect_hdr));
  
  recthdr->rect.x = x;
  recthdr->rect.y = y;
  recthdr->rect.w = w;
  recthdr->rect.h = h; 
  recthdr->encoding[0] =
    recthdr->encoding[1] =
    recthdr->encoding[2] = 0;
  recthdr->encoding[3] = RFB_ENC_RRE;
      
  rrehdr->subrects[0] =
    rrehdr->subrects[1] = 0;
  rrehdr->bgpixel = c;
      
  return sizeof(struct rfb_fb_update_rect_hdr) + sizeof(struct rfb_rre_hdr);
}
/*-----------------------------------------------------------------------------------*/
void
vnc_server_init(void)
{
  vnc_out_init();
}
/*-----------------------------------------------------------------------------------*/
static void
vnc_send_blank(struct vnc_server_state *vs)
{
  switch(vs->type) {
  case 0:	
    vnc_out_send_blank(vs);
    break;
    /*  case 1:
    vnc_stats_send_blank(vs);
    break;   */
  }
}
/*-----------------------------------------------------------------------------------*/
static void
vnc_send_screen(struct vnc_server_state *vs)
{
  switch(vs->type) {
  case 0:	
    vnc_out_send_screen(vs);
    break;
    /*  case 1:
    vnc_stats_send_screen(vs);
    break;*/
  }
}
/*-----------------------------------------------------------------------------------*/
static void
vnc_send_update(struct vnc_server_state *vs)
{
  switch(vs->type) {
  case 0:	
    vnc_out_send_update(vs);
    break;
    /*  case 1:
    vnc_stats_send_update(vs);
    break;*/
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_server_send_data(struct vnc_server_state *vs)
{
  register struct rfb_server_init *initmsg;
  
  switch(vs->state) {
  case VNC_VERSION:
    uip_send(RFB_SERVER_VERSION_STRING, sizeof(RFB_SERVER_VERSION_STRING));
    break;
  case VNC_AUTH:
    ((char *)uip_appdata)[0] = 0;
    ((char *)uip_appdata)[1] = 0;
    ((char *)uip_appdata)[2] = 0;
    ((char *)uip_appdata)[3] = RFB_AUTH_NONE;
    uip_send(uip_appdata, 4);
    break;
  case VNC_INIT:
    initmsg = (struct rfb_server_init *)uip_appdata;
    initmsg->width = uip_htons(vs->width);
    initmsg->height = uip_htons(vs->height);
    /* BGR233 pixel format. */
    initmsg->format.bps = 8;
    initmsg->format.depth = 8;
    initmsg->format.endian = 1;
    initmsg->format.truecolor = 1;
    initmsg->format.red_max = uip_htons(7);
    initmsg->format.green_max = uip_htons(7);
    initmsg->format.blue_max = uip_htons(3);
    initmsg->format.red_shift = 0;
    initmsg->format.green_shift = 3;
    initmsg->format.blue_shift = 6;
    initmsg->namelength[0] = 0;
    initmsg->namelength[1] = 0;
    initmsg->namelength[2] = 0;	    
    initmsg->namelength[3] = 4;
    memcpy(&((char *)uip_appdata)[sizeof(struct rfb_server_init)], uvnc_name, 4);
    /*    ((char *)uip_appdata)[sizeof(struct rfb_server_init)+0] = 'u';
    ((char *)uip_appdata)[sizeof(struct rfb_server_init)+1] = 'V';
    ((char *)uip_appdata)[sizeof(struct rfb_server_init)+2] = 'N';
    ((char *)uip_appdata)[sizeof(struct rfb_server_init)+3] = 'C';*/
    uip_send(uip_appdata, sizeof(struct rfb_server_init) + 4);
    break;
  case VNC_RUNNING:
    switch(vs->sendmsg) {
    case SEND_NONE:
      PRINTF(("Sending none\n"));
      break;
      
    case SEND_BLANK:
    case SENT_BLANK:
      PRINTF(("Sending blank\n"));
      vnc_send_blank(vs);
      break;
      
    case SEND_SCREEN:
      PRINTF(("Sending screen\n"));
      vnc_send_screen(vs);
      break;

    case SEND_UPDATE:
      PRINTF(("Sending update\n"));
      vnc_send_update(vs);
      break;
    }
    break;
    
  default:
    break;
  }

}
/*-----------------------------------------------------------------------------------*/
static void
vnc_key_event(struct vnc_server_state *vs)
{
  switch(vs->type) {
  case 0:	
    vnc_out_key_event(vs);
    break;
    /*  case 1:
    vnc_stats_key_event(vs);
    break;*/
  }
}
/*-----------------------------------------------------------------------------------*/
static void
vnc_pointer_event(struct vnc_server_state *vs)
{
  switch(vs->type) {
  case 0:	
    vnc_out_pointer_event(vs);
    break;
    /*  case 1:
    vnc_stats_pointer_event(vs);
    break;*/
  }
}
/*-----------------------------------------------------------------------------------*/
static uint8_t
vnc_read_data(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  uint8_t *appdata;
  uint16_t len;
  struct rfb_fb_update_request *req;
  /*  uint8_t niter;*/
  
  len = uip_datalen();
  appdata = (uint8_t *)uip_appdata;
  
  /* First, check if there is data left to discard since last read. */
  if(vs->readlen > 0) {
    appdata += vs->readlen;
    if(len > vs->readlen) {
      len -= vs->readlen;
      vs->readlen = 0;
    } else {
      vs->readlen -= len;
      len = 0;
    }
  }

  if(vs->readlen != 0) {
    return 1;
  }

  /* All data read and ignored, parse next message. */
  /*  for(niter = 32; niter > 0 && len > 0; --niter) {*/
  while(len > 0) {
    switch(vs->state) {
    case VNC_VERSION:
    case VNC_VERSION2:
      PRINTF(("Read in version\n"));
      /* Receive and ignore client version string (12 bytes). */
      vs->state = VNC_AUTH;
      vs->readlen = 12;
      break;
      
    case VNC_AUTH:
    case VNC_AUTH2:
      PRINTF(("Read in auth \n"));
      /* Read and discard initialization from client (1 byte). */
      vs->readlen = 1;
      vs->state = VNC_INIT;
      break;
      
    case VNC_INIT:
    case VNC_INIT2:
      PRINTF(("Read in init \n"));
      vs->readlen = 0;
      vs->state = VNC_RUNNING;
      
    case VNC_RUNNING:
      /* Handle all client events. */
      switch(*appdata) {
      case RFB_SET_PIXEL_FORMAT:
	PRINTF(("Set pixel format\n"));
	vs->readlen = sizeof(struct rfb_set_pixel_format);
	/* Check if client runs with BGR233 format. If not, abort the
	   connection. */
	/* XXX: not implemented yet. */
	break;
	
      case RFB_FIX_COLORMAP_ENTRIES:
	PRINTF(("Fix colormap entries\n"));
	return 0;
	
      case RFB_SET_ENCODINGS:
	PRINTF(("Set encodings\n"));
	vs->readlen = sizeof(struct rfb_set_encoding);
	vs->readlen += uip_htons(((struct rfb_set_encoding *)appdata)->encodings) * 4;
	/* Make sure that client supports the encodings we use. */
	/* XXX: not implemented yet. */
	break;
	
      case RFB_FB_UPDATE_REQ:
	PRINTF(("Update request\n"));
	vs->update_requested = 1;
	vs->readlen = sizeof(struct rfb_fb_update_request);
	/* blank the screen initially */
	req = (struct rfb_fb_update_request *)appdata;
	if(req->incremental == 0) {
	  /*	  vs->sendmsg = SEND_BLANK;*/
	  vnc_out_update_area(vs, 0, 0, vs->w, vs->h);
	}
	break;
	
      case RFB_KEY_EVENT:
	vs->readlen = sizeof(struct rfb_key_event);
	vnc_key_event(vs);
	break;
	
      case RFB_POINTER_EVENT:
	vs->readlen = sizeof(struct rfb_pointer_event);
	vnc_pointer_event(vs);
	break;
	
      case RFB_CLIENT_CUT_TEXT:
	PRINTF(("Client cut text\n"));

	if(((struct rfb_client_cut_text *)appdata)->len[0] != 0 ||
	   ((struct rfb_client_cut_text *)appdata)->len[1] != 0) {
	  return 0;
	  
	}
	vs->readlen = sizeof(struct rfb_client_cut_text) +
	  (((struct rfb_client_cut_text *)appdata)->len[2] << 8) +
	  ((struct rfb_client_cut_text *)appdata)->len[3];
	/*	return 0;*/
	break;
	
      default:
	PRINTF(("Unknown message %d\n", *appdata));
	return 0;
      }
      break;
      
    default:
      return 0;
    }

    if(vs->readlen > 0) {
      if(len > vs->readlen) {
	len -= vs->readlen;
	appdata += vs->readlen;
	vs->readlen = 0;
      } else {
	vs->readlen -= len;
	len = 0;
      }
    } else {
      /* Lost data. */
      break;
    }
    
  }

  /*  if(vs->readlen > 0) {
    printf("More data %d\n", vs->readlen);
    }*/
  
  /*  uip_appdata = appdata;*/

  return 1;
}
/*-----------------------------------------------------------------------------------*/
static void
vnc_new(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  vs->counter = 0;
  vs->readlen = 0;
  vs->sendmsg = SEND_NONE;
  vs->update_requested = 1;
  switch(vs->type) {
  case 0:	
    vnc_out_new(vs);
    break;
    /*  case 1:
    vnc_stats_new(vs);
    break;*/
  }
}
/*-----------------------------------------------------------------------------------*/
static void
vnc_acked(CC_REGISTER_ARG struct vnc_server_state *vs)
{
  switch(vs->state) {
  case VNC_VERSION:
    vs->state = VNC_VERSION2;
    break;
    
  case VNC_AUTH:
    vs->state = VNC_AUTH2;
    break;
    
  case VNC_INIT:
    vs->state = VNC_INIT2;
    break;

  case VNC_RUNNING:
    switch(vs->type) {
    case 0:	
      vnc_out_acked(vs);
      break;
      /*    case 1:
      vnc_stats_acked(vs);
      break;*/
    }
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_server_appcall(struct vnc_server_state *vs)
{
  
  vs->type = uip_htons(uip_conn->lport) - 5900;
  
  if(uip_connected()) {      
    vnc_new(vs);
    vs->state = VNC_VERSION;
    vnc_server_send_data(vs);
    return;
  }
  if(uip_acked()) {
    PRINTF(("Acked\n"));
    vnc_acked(vs);
  }
  
  if(uip_newdata()) {
    PRINTF(("Newdata\n"));
    vs->counter = 0;
    if(vnc_read_data(vs) == 0) {
      uip_abort();
      return;
    }
  }
  
  if(uip_rexmit()) {
    PRINTF(("Rexmit\n"));
  }
  
  
  if(uip_newdata() ||
     uip_rexmit() ||
     uip_acked()) {
    vnc_server_send_data(vs);
  } else if(uip_poll()) {
    ++vs->counter;
    /* Abort connection after about 20 seconds of inactivity. */
    if(vs->counter >= 40) {
      uip_abort();
      return;
    }
    
    vnc_out_poll(vs);
  }
  
}
/*-----------------------------------------------------------------------------------*/
