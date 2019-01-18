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
 * $Id: rs232dev-ss.c,v 1.1 2007/05/23 23:11:30 oliverschmidt Exp $
 *
 */

/*
 * This is a generic implementation of the SLIP protocol over an RS232
 * (serial) device. While initially intented for the C64, the code can
 * easily be ported to other platforms as well.
 *
 * Huge thanks to Ullrich von Bassewitz <uz@cc65.org> of cc65 fame for
 * and endless supply of bugfixes, insightsful comments and
 * suggestions, and improvements to this code!
 */

#include "rs232silversurfer.h"
#include <time.h>
#include <string.h>

 /* This will include the system specific header files as well */
#if defined(__CBM__)
#  include <cbm.h>
#elif defined(__ATARI__)
#  include <atari.h>
#endif

#include "uip.h"

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335


#define SIO_RECV(c)  while(rs232_get(&c) == RS_ERR_NO_DATA)
#define SIO_POLL(c)  (rs232_get(&c) != RS_ERR_NO_DATA)
#define SIO_SEND(c)  while(rs232_put(c) == RS_ERR_OVERFLOW)

#define MAX_SIZE (UIP_BUFSIZE - UIP_LLH_LEN)

static u8_t slip_buf[MAX_SIZE + 2];

static u16_t len, tmplen;

#if 1
#define printf(x)
#else
#include <stdio.h>
#endif


/*-----------------------------------------------------------------------------------*/
static void
rs232_err(char err)
{
  switch(err) {
  case RS_ERR_OK:
    printf("RS232 OK\n");
    break;
  case RS_ERR_NOT_INITIALIZED:
    printf("RS232 not initialized\n");
    break;
  case RS_ERR_BAUD_TOO_FAST:
    printf("RS232 baud too fast\n");
    break;
  case RS_ERR_BAUD_NOT_AVAIL:
    printf("RS232 baud rate not available\n");
    break;
  case RS_ERR_NO_DATA:
    printf("RS232 nothing to read\n");
    break;
  case RS_ERR_OVERFLOW:
    printf("RS232 overflow\n");
    break;
  }

}
/*-----------------------------------------------------------------------------------*/
/*
 * rs232dev_send():
 *
 * Sends the packet in the uip_buf and uip_appdata buffers. The first
 * 40 bytes of the packet (the IP and TCP headers) are read from the
 * uip_buf buffer, and the following bytes (the application data) are
 * read from the uip_appdata buffer.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
rs232dev_send(void)
{
  u16_t i;
  u8_t *ptr;
  u8_t c;

  SIO_SEND(SLIP_END);

  ptr = &uip_buf[UIP_LLH_LEN];
  for(i = 0; i < uip_len; ++i) {
    if(i == UIP_TCPIP_HLEN) {
      ptr = uip_appdata;
    }
    c = *ptr++;
    switch(c) {
    case SLIP_END:
      SIO_SEND(SLIP_ESC);
      SIO_SEND(SLIP_ESC_END);
      break;
    case SLIP_ESC:
      SIO_SEND(SLIP_ESC);
      SIO_SEND(SLIP_ESC_ESC);
      break;
    default:
      SIO_SEND(c);
      break;
    }
  }
  SIO_SEND(SLIP_END);
}
/*-----------------------------------------------------------------------------------*/
/*
 * rs232dev_poll():
 *
 * Read all avaliable bytes from the RS232 interface into the slip_buf
 * buffer. If no more bytes are avaliable, it returns with 0 to
 * indicate that no packet was immediately ready. When a full packet
 * has been read into the buffer, the packet is copied into the
 * uip_buf buffer and the length of the packet is returned.
 *
 */
/*-----------------------------------------------------------------------------------*/
u16_t
rs232dev_poll(void)
{
  u8_t c;
  static u8_t lastc;
  
  while(SIO_POLL(c)) {
    /*    printf("c %x\n", c);*/
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
      
      if(len > MAX_SIZE) {
	len = 0;
      }
    
      break;
    }
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/*
 * rs232dev_init():
 *
 * Initializes the RS232 device and sets the parameters of the device.
 *
 */ 
/*-----------------------------------------------------------------------------------*/
void
rs232dev_init(void)
{
  char err;
  
  err = rs232_init(0);
  rs232_err(err);
  err = rs232_params(RS_BAUD_9600 | RS_BITS_8 | RS_STOP_1, RS_PAR_NONE);
  rs232_err(err);

  len = 0;

  return;
}
/*-----------------------------------------------------------------------------------*/

