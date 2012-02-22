/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup slip Serial Line IP (SLIP) protocol
 * @{
 *
 * The SLIP protocol is a very simple way to transmit IP packets over
 * a serial line. It does not provide any framing or error control,
 * and is therefore not very widely used today.
 *
 * This SLIP implementation requires two functions for accessing the
 * serial device: slipdev_char_poll() and slipdev_char_put(). These
 * must be implemented specifically for the system on which the SLIP
 * protocol is to be run.
 */

/**
 * \file
 * SLIP protocol implementation
 * \author Adam Dunkels <adam@dunkels.com>
 */

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
 * $Id: slipdev.c,v 1.1 2006/06/17 22:41:18 adamdunkels Exp $
 *
 */

/*
 * This is a generic implementation of the SLIP protocol over an RS232
 * (serial) device. 
 *
 * Huge thanks to Ullrich von Bassewitz <uz@cc65.org> of cc65 fame for
 * and endless supply of bugfixes, insightsful comments and
 * suggestions, and improvements to this code!
 */

#include "uip.h"
#include "uip-fw.h"
#include "slipdev.h"
#include <string.h>  /* For memcpy() */

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

static uint8_t slip_buf[UIP_BUFSIZE];

static uint16_t len, tmplen;
static uint8_t lastc;

/*-----------------------------------------------------------------------------------*/
/**
 * Send the packet in the uip_buf and uip_appdata buffers using the
 * SLIP protocol.
 *
 * The first 40 bytes of the packet (the IP and TCP headers) are read
 * from the uip_buf buffer, and the following bytes (the application
 * data) are read from the uip_appdata buffer.
 *
 * \return This function will always return UIP_FW_OK.
 */
/*-----------------------------------------------------------------------------------*/
uint8_t
slipdev_send(void)
{
  uint16_t i;
  uint8_t *ptr;
  uint8_t c;

  slipdev_char_put(SLIP_END);

  ptr = &uip_buf[UIP_LLH_LEN];
  for(i = 0; i < uip_len; ++i) {
    if(i == UIP_TCPIP_HLEN) {
      ptr = (char *)uip_appdata;
    }
    c = *ptr++;
    switch(c) {
    case SLIP_END:
      slipdev_char_put(SLIP_ESC);
      slipdev_char_put(SLIP_ESC_END);
      break;
    case SLIP_ESC:
      slipdev_char_put(SLIP_ESC);
      slipdev_char_put(SLIP_ESC_ESC);
      break;
    default:
      slipdev_char_put(c);
      break;
    }
  }
  slipdev_char_put(SLIP_END);

  return UIP_FW_OK;
}
/*-----------------------------------------------------------------------------------*/
/** 
 * Poll the SLIP device for an available packet.
 *
 * This function will poll the SLIP device to see if a packet is
 * available. It uses a buffer in which all avaliable bytes from the
 * RS232 interface are read into. When a full packet has been read
 * into the buffer, the packet is copied into the uip_buf buffer and
 * the length of the packet is returned.
 *
 * \return The length of the packet placed in the uip_buf buffer, or
 * zero if no packet is available.
 */
/*-----------------------------------------------------------------------------------*/
uint16_t
slipdev_poll(void)
{
  uint8_t c;
  
  while(slipdev_char_poll(&c)) {
    switch(c) {
    case SLIP_ESC:
      lastc = c;
      break;
      
    case SLIP_END:
      lastc = c;
      /* End marker found, we copy our input buffer to the uip_buf
	 buffer and return the size of the packet we copied. */
      memcpy(&uip_buf[UIP_LLH_LEN], slip_buf, len);
      tmplen = len;
      len = 0;
      return tmplen;
      
    default:     
      if(lastc == SLIP_ESC) {
	lastc = c;
	/* Previous read byte was an escape byte, so this byte will be
	   interpreted differently from others. */
	switch(c) {
	case SLIP_ESC_END:
	  c = SLIP_END;
	  break;
	case SLIP_ESC_ESC:
	  c = SLIP_ESC;
	  break;
	}
      } else {
	lastc = c;
      }
      
      slip_buf[len] = c;
      ++len;
      
      if(len > UIP_BUFSIZE) {
	len = 0;
      }
    
      break;
    }
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Initialize the SLIP module.
 *
 * This function does not initialize the underlying RS232 device, but
 * only the SLIP part.
 */ 
/*-----------------------------------------------------------------------------------*/
void
slipdev_init(void)
{
  lastc = len = 0;
}
/*-----------------------------------------------------------------------------------*/

/** @} */
/** @} */
