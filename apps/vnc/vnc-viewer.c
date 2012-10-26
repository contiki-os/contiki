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
 *
 */

/* A micro implementation of a VNC client. VNC is a protocol for
   remote network displays. See http://www.uk.research.att.com/vnc/
   for information about VNC.
*/

#include "vnc-draw.h"
#include "net/uip.h"
#include "vnc-viewer.h"

#include <string.h> /* XXX for memcpy */
#include <stdio.h>  /* XXX for printf */

/* RFB server initial handshaking string. */
#define RFB_SERVER_VERSION_STRING rfb_server_version_string

/* "RFB 003.003" */
static uint8_t rfb_server_version_string[12] =
  {82,70,66,32,48,48,51,46,48,48,51,10};

#if 0
#define PRINTF(x)
#else
#define PRINTF(x) printf x
#endif

#ifndef NULL
#define NULL (void *)0
#endif

static struct vnc_viewer_state vnc_viewer_state;
#define vs (&vnc_viewer_state)

/*-----------------------------------------------------------------------------------*/
void
vnc_viewer_init(void)
{
}
/*-----------------------------------------------------------------------------------*/
void
vnc_viewer_close(void)
{
  vs->close = 1;
}
/*-----------------------------------------------------------------------------------*/
void
vnc_viewer_connect(uint16_t *server, uint8_t display)
{
  struct uip_conn *conn;

  vnc_draw_init();

  memset(vs, 0, sizeof(struct vnc_viewer_state));
  conn = uip_connect((uip_ipaddr_t *)server, uip_htons(5900 + display));
  if(conn == NULL) {
    return;
  }
  tcp_markconn(conn, NULL);

  vs->close = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
senddata(void)
{
  register uint8_t *dataptr;
  uint16_t dataleft;

  dataptr = (uint8_t *)uip_appdata;
    
  switch(vs->sendmsg) {
  case VNC_SEND_VERSION:
    PRINTF(("Sending VERSION_STRING\n"));
    uip_send(RFB_SERVER_VERSION_STRING, sizeof(RFB_SERVER_VERSION_STRING));
    break;
  case VNC_SEND_AUTH:
    /* Send 16 bytes of encrypted challange response. */
    /* XXX: not implemented. */
    PRINTF(("Sending AUTH\n"));
    uip_send(uip_appdata, 16);
    break;
  case VNC_SEND_CINIT:
    PRINTF(("Sending CINIT\n"));
    /* Send one byte of client init. */
    *(uint8_t *)dataptr = 1;
    uip_send(uip_appdata, 1);
    break;
  case VNC_SEND_PFMT:
    PRINTF(("Sending PFMT\n"));
    ((struct rfb_set_pixel_format *)dataptr)->type = RFB_SET_PIXEL_FORMAT;

    /* Set to BGR233 pixel format. */
    ((struct rfb_set_pixel_format *)dataptr)->format.bps = 8;
    ((struct rfb_set_pixel_format *)dataptr)->format.depth = 8;
    ((struct rfb_set_pixel_format *)dataptr)->format.endian = 1;
    ((struct rfb_set_pixel_format *)dataptr)->format.truecolor = 1;
    ((struct rfb_set_pixel_format *)dataptr)->format.red_max = uip_htons(7);
    ((struct rfb_set_pixel_format *)dataptr)->format.green_max = uip_htons(7);
    ((struct rfb_set_pixel_format *)dataptr)->format.blue_max = uip_htons(3);
    ((struct rfb_set_pixel_format *)dataptr)->format.red_shift = 0;
    ((struct rfb_set_pixel_format *)dataptr)->format.green_shift = 3;
    ((struct rfb_set_pixel_format *)dataptr)->format.blue_shift = 6;

    uip_send(uip_appdata, sizeof(struct rfb_set_pixel_format));
    
    break;
  case VNC_SEND_ENCODINGS:
    PRINTF(("Sending ENCODINGS\n"));
    ((struct rfb_set_encodings *)dataptr)->type = RFB_SET_ENCODINGS;
    ((struct rfb_set_encodings *)dataptr)->encodings = uip_htons(1);
    dataptr += sizeof(struct rfb_set_encodings);
    dataptr[0] = dataptr[1] = dataptr[2] = 0;
    dataptr[3] = RFB_ENC_RAW;
    /*    ((uint8_t *)dataptr + sizeof(struct rfb_set_encodings))[4] =
      ((uint8_t *)dataptr + sizeof(struct rfb_set_encodings))[5] =
      ((uint8_t *)dataptr + sizeof(struct rfb_set_encodings))[6] = 0;
      ((uint8_t *)dataptr + sizeof(struct rfb_set_encodings))[7] = RFB_ENC_RRE;*/
    uip_send(uip_appdata, sizeof(struct rfb_set_encodings) + 4);
    break;
  case VNC_SEND_UPDATERQ:
    ((struct rfb_fb_update_request *)dataptr)->type = RFB_FB_UPDATE_REQ;
    ((struct rfb_fb_update_request *)dataptr)->incremental = 0;
    ((struct rfb_fb_update_request *)dataptr)->x = uip_htons(vnc_draw_viewport_x());
    ((struct rfb_fb_update_request *)dataptr)->y = uip_htons(vnc_draw_viewport_y());
    ((struct rfb_fb_update_request *)dataptr)->w = uip_htons(vnc_draw_viewport_w());
    ((struct rfb_fb_update_request *)dataptr)->h = uip_htons(vnc_draw_viewport_h());
    uip_send(uip_appdata, sizeof(struct rfb_fb_update_request));
    break;
  case VNC_SEND_UPDATERQ_INC:
    ((struct rfb_fb_update_request *)dataptr)->type = RFB_FB_UPDATE_REQ;
    ((struct rfb_fb_update_request *)dataptr)->incremental = 1;
    ((struct rfb_fb_update_request *)dataptr)->x = uip_htons(vnc_draw_viewport_x());
    ((struct rfb_fb_update_request *)dataptr)->y = uip_htons(vnc_draw_viewport_y());
    ((struct rfb_fb_update_request *)dataptr)->w = uip_htons(vnc_draw_viewport_w());
    ((struct rfb_fb_update_request *)dataptr)->h = uip_htons(vnc_draw_viewport_h());
    uip_send(uip_appdata, sizeof(struct rfb_fb_update_request));
    break;

  case VNC_SEND_EVENTS:

    dataleft = uip_mss();

    vs->eventptr_unacked = vs->eventptr_acked;
    while(vs->eventptr_unacked != vs->eventptr_next &&
	  dataleft > sizeof(struct rfb_key_event)) {
      switch(vs->event_queue[vs->eventptr_unacked].type) {
      case VNC_POINTER_EVENT:
	((struct rfb_pointer_event *)dataptr)->type = RFB_POINTER_EVENT;
	((struct rfb_pointer_event *)dataptr)->buttonmask =
	  vs->event_queue[vs->eventptr_unacked].ev.ptr.buttonmask;
	((struct rfb_pointer_event *)dataptr)->x =
	  uip_htons(vs->event_queue[vs->eventptr_unacked].ev.ptr.x);
	((struct rfb_pointer_event *)dataptr)->y =
	  uip_htons(vs->event_queue[vs->eventptr_unacked].ev.ptr.y);
	/*	uip_send(uip_appdata, sizeof(struct rfb_pointer_event));*/
	dataptr += sizeof(struct rfb_pointer_event);
	dataleft -= sizeof(struct rfb_pointer_event);
      break;
      case VNC_KEY_EVENT:
	PRINTF(("Send key event.\n"));
	((struct rfb_key_event *)dataptr)->type = RFB_KEY_EVENT;
	((struct rfb_key_event *)dataptr)->down =
	  vs->event_queue[vs->eventptr_unacked].ev.key.down;
	((struct rfb_key_event *)dataptr)->key[0] =
	  ((struct rfb_key_event *)dataptr)->key[1];
	((struct rfb_key_event *)dataptr)->key[2] =
	  vs->event_queue[vs->eventptr_unacked].ev.key.key >> 8;
	
	((struct rfb_key_event *)dataptr)->key[3] =
	  vs->event_queue[vs->eventptr_unacked].ev.key.key & 0xff;
	/*	uip_send(uip_appdata, sizeof(struct rfb_key_event));*/
	dataptr += sizeof(struct rfb_key_event);
	dataleft -= sizeof(struct rfb_key_event);
	break;
      case VNC_UPDATERQ_EVENT:
	((struct rfb_fb_update_request *)dataptr)->type = RFB_FB_UPDATE_REQ;
	((struct rfb_fb_update_request *)dataptr)->incremental = 0;
	((struct rfb_fb_update_request *)dataptr)->x =
	  uip_htons(vs->event_queue[vs->eventptr_unacked].ev.urq.x);
	((struct rfb_fb_update_request *)dataptr)->y =
	  uip_htons(vs->event_queue[vs->eventptr_unacked].ev.urq.y);
	((struct rfb_fb_update_request *)dataptr)->w =
	  uip_htons(vs->event_queue[vs->eventptr_unacked].ev.urq.w);
	((struct rfb_fb_update_request *)dataptr)->h =
	  uip_htons(vs->event_queue[vs->eventptr_unacked].ev.urq.h);
	/*	uip_send(uip_appdata, sizeof(struct rfb_fb_update_request));    */
	dataptr += sizeof(struct rfb_fb_update_request);
	dataleft -= sizeof(struct rfb_fb_update_request);
	break;
      }
      vs->eventptr_unacked = (vs->eventptr_unacked + 1) % VNC_EVENTQUEUE_SIZE;
    }

    uip_send(uip_appdata, uip_mss() - dataleft);
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
buffer_data(uint8_t *data, uint16_t datalen)
{
  PRINTF(("Buffering %d bytes of data\n", datalen));

  if(vs->buffersize + datalen > VNC_BUFFERSIZE) {
    PRINTF(("Out of buffer space!\n"));
    vs->buffersize = 0;
    return;
  }
  
  memcpy(&(vs->buffer[vs->buffersize]), data, datalen);

  vs->buffersize += datalen;
  vs->bufferleft -= datalen;
}
/*-----------------------------------------------------------------------------------*/
static void
clearbuffer(void)
{
  PRINTF(("Clearing buffer\n"));
  vs->buffersize = 0;
}
/*-----------------------------------------------------------------------------------*/
/* Returns: the amount of bytes actually read. */
static uint16_t
recv_rectstate(uint8_t *dataptr, uint16_t datalen)
{
  uint16_t pixels;
  uint16_t pixelsleft;
  
  switch(vs->rectstate) {
  case VNC_RECTSTATE_RAW:
    
    if(vs->rectstateleft > datalen) {
      pixels = datalen;
      vs->rectstateleft -= datalen;
    } else {
      pixels = vs->rectstateleft;
      datalen = datalen - vs->rectstateleft;
      vs->rectstateleft = 0;
      vs->rectstate = VNC_RECTSTATE_NONE;
      vs->sendmsg = VNC_SEND_NONE;
      vs->waitmsg = VNC_WAIT_NONE;
      vnc_viewer_refresh();
    }


    pixelsleft = pixels;
    while(pixelsleft > 0) {
      if(pixelsleft >= vs->rectstatex2 - vs->rectstatex) {
	vnc_draw_pixelline(vs->rectstatex, vs->rectstatey,
			   dataptr,
			   vs->rectstatex2 - vs->rectstatex);
	dataptr += vs->rectstatex2 - vs->rectstatex;
	pixelsleft -= vs->rectstatex2 - vs->rectstatex;
	vs->rectstatex = vs->rectstatex0;
	++vs->rectstatey;
	vnc_viewer_refresh();
      } else {
	vnc_draw_pixelline(vs->rectstatex, vs->rectstatey,
			   dataptr,
			   pixelsleft);
	vs->rectstatex += pixelsleft;
	pixelsleft = 0;
      }
      
    }
        
    break;
  case VNC_RECTSTATE_RRE:
    break;
  }
  return pixels;
}
/*-----------------------------------------------------------------------------------*/
/* Returns: the amount of bytes that needs to be buffered before the
   rect can be drawn. */
static unsigned short
recv_update_rect(CC_REGISTER_ARG struct rfb_fb_update_rect_hdr *rhdr,
		 uint16_t dataleft)
{
  struct rfb_rre_hdr *rrehdr;


  if((rhdr->encoding[0] |
      rhdr->encoding[1] |
      rhdr->encoding[2]) == 0) {
    switch(rhdr->encoding[3]) {
    case RFB_ENC_RAW:
      vs->rectstateleft = (uint32_t)uip_htons(rhdr->rect.w) * (uint32_t)uip_htons(rhdr->rect.h);
      vs->rectstate = VNC_RECTSTATE_RAW;
      vs->rectstatex0 = vs->rectstatex = uip_htons(rhdr->rect.x);
      vs->rectstatey0 = vs->rectstatey = uip_htons(rhdr->rect.y);
      vs->rectstatew = uip_htons(rhdr->rect.w);
      vs->rectstateh = uip_htons(rhdr->rect.h);
      vs->rectstatex2 = vs->rectstatex0 + vs->rectstatew;
      vs->rectstatey2 = vs->rectstatey0 + vs->rectstateh;
      break;

    case RFB_ENC_RRE:
      rrehdr = (struct rfb_rre_hdr *)((uint8_t *)rhdr +
				      sizeof(struct rfb_fb_update_rect_hdr));
      PRINTF(("Received RRE subrects %d (%d)\n",
	     (uip_htons(rrehdr->subrects[1]) << 16) +
	     uip_htons(rrehdr->subrects[0]),
	     rrehdr->bgpixel));
      vs->rectstateleft = ((uint32_t)(uip_htons(rrehdr->subrects[1]) << 16) +
			(uint32_t)uip_htons(rrehdr->subrects[0]));
      vs->rectstate = VNC_RECTSTATE_RRE;

      break;

    default:
      PRINTF(("Bad encoding %02x%02x%02x%02x\n", rhdr->encoding[0],
	      rhdr->encoding[1],rhdr->encoding[2],rhdr->encoding[3]));
      break;
    }

  }

  return 0;
	
  PRINTF(("recv_update_rect: returning%d\n", vs->rectstateleft));
  return sizeof(struct rfb_fb_update_rect_hdr) + vs->rectstateleft;
}
/*-----------------------------------------------------------------------------------*/
/* identify_data():
 *
 * This function looks at the state of the connection (i.e., if it is
 * handshaking or in steady-state) as well as on the contents of the
 * incoming message and returns the number of bytes of data that is to
 * be expected.
 */
static uint16_t
identify_data(CC_REGISTER_ARG uint8_t *data, uint16_t datalen)
{
  switch(vs->waitmsg) {
  case VNC_WAIT_VERSION:
    /* Expecting version string (12 bytes). */
    return 12;
    break;
  case VNC_WAIT_AUTH:
    return 4;
    break;
  case VNC_WAIT_AUTH_RESPONSE:
    return 1;
    break;
  case VNC_WAIT_SINIT:
    /* XXX: We should check that the entire message header is
       received, otherwise we should buffer it. */
    return sizeof(struct rfb_server_init) +
      ((struct rfb_server_init *)uip_appdata)->namelength[3] +
      ((struct rfb_server_init *)uip_appdata)->namelength[2];
    
  case VNC_WAIT_UPDATE:
  case VNC_WAIT_NONE:
    switch(*data) {
    case RFB_FB_UPDATE:
      PRINTF(("RFB FB UPDATE received\n"));
      return sizeof(struct rfb_fb_update);
      
    case RFB_BELL:
      return 1;

    case RFB_SERVER_CUT_TEXT:
      PRINTF(("Cut text received, unhandled\n"));
      return 8 + (data[6] << 8) + data[7];

    case RFB_SET_COLORMAP_ENTRIES:
      uip_abort();
      PRINTF(("Set colormap entries received, unhandled\n"));
      return 0;
      
    default:
      uip_abort();
      PRINTF(("Weird message type received (%d)\n", *(uint8_t *)uip_appdata));
      return 0;
    }
    break;
    
  case VNC_WAIT_UPDATE_RECT:
    return sizeof(struct rfb_fb_update_rect_hdr);

  default:
    PRINTF(("identify: bad waitmsg %d\n", vs->waitmsg));
  }
  
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/* handle_data():
 *
 * Handles the data.
 */
static unsigned short
handle_data(CC_REGISTER_ARG uint8_t *data, uint16_t datalen)
{
  
  switch(vs->waitmsg) {
  case VNC_WAIT_VERSION:
    /* Make sure we get the right version string from the server. */
    /* XXX: not implemented. */
    PRINTF(("Got version, waiting for auth, sending version\n"));
    vs->waitmsg = VNC_WAIT_AUTH;
    vs->sendmsg = VNC_SEND_VERSION;
    break;
  case VNC_WAIT_AUTH:
    switch(data[3]) {
    case RFB_AUTH_FAILED:
      PRINTF(("Connection failed.\n"));
      uip_abort();
      return 0;
      
    case RFB_AUTH_NONE:
      vs->sendmsg = VNC_SEND_CINIT;
      vs->waitmsg = VNC_WAIT_SINIT;
      PRINTF(("No authentication needed.\n"));
      break;
    case RFB_AUTH_VNC:
      vs->sendmsg = VNC_SEND_AUTH;
      vs->waitmsg = VNC_WAIT_AUTH_RESPONSE;
      PRINTF(("VNC authentication needed.\n"));
      break;
    }
    break;
  case VNC_WAIT_SINIT:
    /*    PRINTF(("Server init: w %d h %d, bps %d, d %d, name '%s'\n",
	   uip_htons(((struct rfb_server_init *)data)->width),
	   uip_htons(((struct rfb_server_init *)data)->height),
	   ((struct rfb_server_init *)data)->format.bps,
	   ((struct rfb_server_init *)data)->format.depth,
	   ((uint8_t *)data + sizeof(struct rfb_server_init))));*/
    vs->w = uip_htons(((struct rfb_server_init *)data)->width);
    vs->h = uip_htons(((struct rfb_server_init *)data)->height);
    vs->sendmsg = VNC_SEND_PFMT;
    vs->waitmsg = VNC_WAIT_NONE;
    break;

  case VNC_WAIT_UPDATE:
  case VNC_WAIT_NONE:
    switch(*data) {
    case RFB_FB_UPDATE:
      vs->waitmsg = VNC_WAIT_UPDATE_RECT;
      vs->rectsleft = uip_htons(((struct rfb_fb_update *)data)->rects);
      PRINTF(("Handling RFB FB UPDATE for %d rects\n", vs->rectsleft));
      break;
      
    case RFB_BELL:
      PRINTF(("BELL\n"));
      break;

    case RFB_SERVER_CUT_TEXT:
      PRINTF(("Cut text received, unhandled\n"));
      break;

    case RFB_SET_COLORMAP_ENTRIES:
      PRINTF(("Set colormap entries received, unhandled\n"));
      break;
      
    default:
      PRINTF(("Weird message type received (%d)\n", *(uint8_t *)data));
      break;
    }
    break;
    
  case VNC_WAIT_UPDATE_RECT:
    PRINTF(("Handling data in WAIT_UPDATE_RECT, %d rects left (%d bytes)\n", vs->rectsleft, datalen));
    --vs->rectsleft;
    if(vs->rectsleft > 0) {
      vs->waitmsg = VNC_WAIT_UPDATE_RECT;
    } else {
      vs->waitmsg = VNC_WAIT_NONE;
      vs->sendmsg = VNC_SEND_NONE;
      vs->rectstate = VNC_RECTSTATE_NONE;
    }
    return recv_update_rect((struct rfb_fb_update_rect_hdr *)data, datalen);
    break;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/* newdata():
 *
 * Called whenever new data arrives.
 *
 * First, checks if data needs to be buffered before a previously
 * identified request can be fulfilled. If so, the incoming data is
 * buffered and the data handler is called. If no data needs to be
 * buffered, the code proceeds to identify the incoming request. If
 * the incoming request can be processed immediately (i.e., all data
 * is contained in this message) the data handler is invoked. If data
 * has to be buffered to fulfill the request, this is noted and taken
 * care of the next time this function is invoked (i.e., for the next
 * incoming data chunk).
 */
static uint8_t
newdata(void)
{
  uint16_t datalen;
  uint16_t readlen;
  uint8_t *dataptr;
  
  datalen = uip_datalen();
  dataptr = (uint8_t *)uip_appdata;

  PRINTF(("newdata: %d bytes\n", datalen));
  
  /* If we are in a "rectstate", meaning that the incoming data is
     part of a rectangle that is being incrementaly drawn on the
     screen, we handle that first. */
  if(vs->rectstate != VNC_RECTSTATE_NONE) {
    readlen = recv_rectstate(dataptr, datalen);
    PRINTF(("newdata: vs->rectstate %d, datalen %d, readlen %d\n",
	    vs->rectstate, datalen, readlen));
    datalen -= readlen;
    dataptr += readlen;
  }

  /* Next, check if we are supposed to buffer data from the incoming
     segment. */
  while(vs->bufferleft > 0 && datalen > 0) {
    if(datalen >= vs->bufferleft) {
      /* There is more data in the incoming chunk than we need to
	 buffer, so we buffer as much as we can and handle the
	 buffered data, and repeat the (identify->buffer->handle)
	 sequence for the data that is left in the incoming chunk. */
      datalen -= vs->bufferleft;
      dataptr += vs->bufferleft;
      buffer_data((uint8_t *)uip_appdata, vs->bufferleft);
      handle_data(vs->buffer, vs->buffersize);
      clearbuffer();
    } else { /* datalen < vs->bufferleft */
      /* We need to buffer more data than was received with this
         chunk, so we buffer the avaliable data and return. */
      buffer_data(dataptr, datalen);
      return 0;
    }
  }

  /* Finally, if there is data left in the segment, we handle it. */
  while(datalen > 0) {

    if(vs->rectstate != VNC_RECTSTATE_NONE) {
      readlen = recv_rectstate(dataptr, datalen);
      PRINTF(("newdata (2): vs->rectstate %d, datalen %d, readlen %d\n",
	      vs->rectstate, datalen, readlen));
      datalen -= readlen;
      dataptr += readlen;
    } else {
 
      /* We get here if there is data to be identified in the incoming
	 chunk. */
      readlen = identify_data(dataptr, datalen);

      if(readlen == 0) {
	PRINTF(("Identify returned 0\n"));
	return 0;
      }

      PRINTF(("Reading %d bytes more\n", readlen));
      /* The data has been identified and the amount of data that
	 needs to be read to be able to process the data is in the
	 "readlen" variable. If the incoming chunk contains enough
	 data, we handle it directly, otherwise we buffer the incoming
	 data and set the state so that we know that there is more
	 data to be buffered. */
      if(readlen > datalen) {
	clearbuffer(); /* Should not be needed, but just in case... */
	vs->bufferleft = readlen;
	buffer_data(dataptr, datalen);
	return 0;
      }
      if(readlen <= datalen) {
	PRINTF(("Before handle_data %d\n", readlen));
	readlen += handle_data(dataptr, readlen);
	PRINTF(("After handle_data %d\n", readlen));
	datalen -= readlen;
	dataptr += readlen;
      }

    }
    if(datalen > 0) {
      PRINTF(("newdata: there is more data left after first iteration... %d\n", datalen));
    }
    
  }
  
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/* Called when there is nothing else to do - checks for any pending
   events (mouse movements or keypresses). If no events are found, it
   makes sure that we send out an incremental update request. */
static void
check_events(void)
{
  if(vs->sendmsg == VNC_SEND_NONE &&
     vs->eventptr_next != vs->eventptr_acked &&
     vs->eventptr_acked == vs->eventptr_unacked) {
    vs->sendmsg = VNC_SEND_EVENTS;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
request_update(void)
{
  if(vs->sendmsg == VNC_SEND_NONE) {
    vs->sendmsg = VNC_SEND_UPDATERQ_INC;
    vs->waitmsg = VNC_WAIT_UPDATE;
    PRINTF(("request_update: requesting\n"));
  } else {
    PRINTF(("request_update: not requesting\n"));
  }
}
/*-----------------------------------------------------------------------------------*/
static void
acked(void)
{
  switch(vs->sendmsg) {
  case VNC_SEND_PFMT:
    vs->sendmsg = VNC_SEND_ENCODINGS;
    break;
  case VNC_SEND_ENCODINGS:
    vs->sendmsg = VNC_SEND_UPDATERQ;
    vs->waitmsg = VNC_WAIT_UPDATE;
    clearbuffer();
    break;
  case VNC_SEND_EVENTS:
    vs->eventptr_acked = vs->eventptr_unacked;
    vs->sendmsg = VNC_SEND_NONE;
    check_events();
    break;
  default:
    vs->sendmsg = VNC_SEND_NONE;
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_viewer_appcall(void * nullptr)
{
  if(vs->close == 1) {
    uip_close();
    return;
  }
  
  if(uip_connected()) {
    vs->sendmsg = VNC_SEND_NONE;
    vs->waitmsg = VNC_WAIT_VERSION;
    return;
  }
  
  if(uip_acked()) {
    acked();
  }

  if(uip_newdata()) {
    newdata();
  }

  if(vs->sendmsg == VNC_SEND_NONE &&
     vs->waitmsg == VNC_WAIT_NONE &&
     vs->rectstate == VNC_RECTSTATE_NONE) {
    check_events();
    request_update();
  }
  PRINTF(("vs->sendmsg %d, vs->waitmsg %d, vs->rectstate %d\n",
	  vs->sendmsg, vs->waitmsg, vs->rectstate));
  
  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked()) {
    senddata();
  } else if(uip_poll()) {
    check_events();
    senddata();
  }
}
/*-----------------------------------------------------------------------------------*/
void
vnc_viewer_post_event(uint8_t type,
		      uint16_t data1, uint16_t data2,
		      uint16_t data3, uint16_t data4)
{
  register struct vnc_event *ev;
  struct vnc_event *ev0;

  ev0 = &(vs->event_queue[(vs->eventptr_next - 1) % VNC_EVENTQUEUE_SIZE]);
    
  ev = &(vs->event_queue[vs->eventptr_next]);
  switch(type) {
  case VNC_POINTER_EVENT:
    if(0 && vs->eventptr_next != vs->eventptr_acked &&
       ev0->type == VNC_POINTER_EVENT &&
       data1 == ev0->ev.ptr.buttonmask) {
      ev0->ev.ptr.x = data2;
      ev0->ev.ptr.y = data3;
      return;
    } else {
      ev->type = type;
      ev->ev.ptr.buttonmask = data1;
      ev->ev.ptr.x = data2;
      ev->ev.ptr.y = data3;
    }
    break;
  case VNC_KEY_EVENT:
    PRINTF(("Key event posted\n"));
    ev->type = type;
    ev->ev.key.down = data1;
    ev->ev.key.key = data2;
    break;
  case VNC_UPDATERQ_EVENT:
    PRINTF(("Update request event posted\n"));
    ev->type = type;
    ev->ev.urq.x = data1;
    ev->ev.urq.y = data2;
    ev->ev.urq.w = data3;
    ev->ev.urq.h = data4;
    break;
  }
  vs->eventptr_next = (vs->eventptr_next + 1) % VNC_EVENTQUEUE_SIZE;
}
/*-----------------------------------------------------------------------------------*/
