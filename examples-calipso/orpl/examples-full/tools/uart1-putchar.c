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
 */


/* This code is adapted from the serial driver by Joris Borms
 * (jborms@users.sourceforge.net), and is available via Contiki projects.
 *
 * -- Fredrik Osterlind (fros@sics.se), 2011 */

/*
 * http://www.tinyos.net/tinyos-2.x/doc/html/tep113.html
 * http://mail.millennium.berkeley.edu/pipermail/tinyos-2-commits/2006-June/003534.html
 * http://www.capsule.hu/local/sensor/documentation/deciphering_tinyOS_serial_packets.pdf

 SF [ { Pr Seq Disp ( Dest Src len Grp Type | Payload ) } CRC ] EF

 SF      1   0x7E    Start Frame Byte
 Pr      1   0x45    Protocol Byte (SERIAL_PROTO_PACKET_NOACK)
 Seq     0           (not used) Sequence number byte, not used due to SERIAL_PROTO_PACKET_NOACK
 Disp    1   0x00    Packet format dispatch byte (TOS_SERIAL_ACTIVE_MESSAGE_ID)
 Dest    2   0xFFFF  (not used)
 Src     2   0x0000  (not used)
 len     1   N       Payload length
 Grp     1   0x00    Group
 Type    1           Message ID
 Payload N           The actual serial message
 CRC     2           Checksum of {Pr -> end of payload}
 EF      1   0x7E    End Frame Byte
 */

#include "deployment.h"
#include "dev/uart1.h"
#include "dev/serial-line.h"
#include "lib/ringbuf.h"

#if IN_INDRIYA || IN_TWIST
/* TODO handle multiple message types */

#ifndef AM_WILAB_CONTIKI_PRINTF
#define AM_WILAB_CONTIKI_PRINTF 65
#endif

#ifndef SERIAL_FRAME_SIZE_CONF
#define SERIAL_FRAME_SIZE 100
#else
#define SERIAL_FRAME_SIZE SERIAL_FRAME_SIZE_CONF
#endif

/* store all characters so we can calculate CRC */
static unsigned char serial_buf[SERIAL_FRAME_SIZE];
static unsigned char serial_buf_index = 0;

static u16_t crcByte(u16_t crc, u8_t b) {
  crc = (u8_t)(crc >> 8) | (crc << 8);
  crc ^= b;
  crc ^= (u8_t)(crc & 0xff) >> 4;
  crc ^= crc << 12;
  crc ^= (crc & 0xff) << 5;
  return crc;
}

int
putchar(int c)
{
  char ch = ((char) c);
  if (serial_buf_index < SERIAL_FRAME_SIZE){
    serial_buf[serial_buf_index] = ch;
    serial_buf_index++;
  }
  if (serial_buf_index == SERIAL_FRAME_SIZE || ch == '\n') {

    u8_t msgID = AM_WILAB_CONTIKI_PRINTF; // TODO look up type?

    /* calculate CRC */
    u16_t crc;
    crc = 0;
    crc = crcByte(crc, 0x45); // Pr byte
    crc = crcByte(crc, 0x00);
    crc = crcByte(crc, 0x0FF); crc = crcByte(crc, 0x0FF); // dest bytes
    crc = crcByte(crc, 0x00); crc = crcByte(crc, 0x00); // src bytes
    crc = crcByte(crc, SERIAL_FRAME_SIZE); // len byte
    crc = crcByte(crc, 0x00);
    crc = crcByte(crc, msgID);
    /* XXX since all of the above are constant, do we need to buffer
     * the characters to calculate CRC? Maybe we don't need the buffer? */
    int i;
    for (i=0; i<serial_buf_index; i++){
      crc = crcByte(crc, serial_buf[i]);
    }
    for (i=serial_buf_index; i<SERIAL_FRAME_SIZE; i++){
      crc = crcByte(crc, 0); // pad with zeroes
    }

    /* send message */
    uart1_writeb(0x7E);
    uart1_writeb(0x45);
    uart1_writeb(0x00);
    uart1_writeb(0x0FF); uart1_writeb(0x0FF);
    uart1_writeb(0x00); uart1_writeb(0x00);
    uart1_writeb(SERIAL_FRAME_SIZE);
    uart1_writeb(0x00);
    uart1_writeb(msgID);
    for (i=0; i<serial_buf_index; i++){
      /* test if bytes need to be escaped
      7d -> 7d 5d
      7e -> 7d 5e */
      if (serial_buf[i] == 0x7d){
        uart1_writeb(0x7d);
        uart1_writeb(0x5d);
      } else if (serial_buf[i] == 0x7e){
        uart1_writeb(0x7d);
        uart1_writeb(0x5e);
      } else {
        uart1_writeb(serial_buf[i]);
      }
    }
    for (i=serial_buf_index; i<SERIAL_FRAME_SIZE; i++){
      uart1_writeb(0); // pad with zeroes
    }
    // crc in reverse-byte oreder
    uart1_writeb((u8_t)(crc & 0x00FF));
    uart1_writeb((u8_t)((crc & 0xFF00) >> 8));
    uart1_writeb(0x7E);

    serial_buf_index = 0;
  }
  return c;
}

void
uart1_tinyos_frames(int active) {

}

#else /* IN_INDRIYA || IN_TWIST */
/* Original Contiki-only version */
int
putchar(int c)
{
  uart1_writeb((char)c);
  return c;
}
#endif /* IN_INDRIYA || IN_TWIST */
