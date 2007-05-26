/* UNFINISHED CODE: the code works, but has yet to be converted into
   proper C code. It is mostly 6502 assembler for now. */

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
 * This file is part of the C64 RealAudio server demo project.
 *
 * $Id: cs8900a.c,v 1.1 2007/05/26 22:00:33 oliverschmidt Exp $
 *
 */

/* cs8900a.c: device driver for the CS8900a chip in 8-bit mode. Mostly
   written in 6502 assembler for speed. */

#include "cs8900a.h"
#include "uip.h"
#include "uip_arp.h"

#define UIP_ETHADDR0 0x00
#define UIP_ETHADDR1 0x00
#define UIP_ETHADDR2 0x00
#define UIP_ETHADDR3 0x64
#define UIP_ETHADDR4 0x64
#define UIP_ETHADDR5 0x64

extern u8_t *cs8900a_rxtxreg,
  *cs8900a_txcmd,
  *cs8900a_txlen,
  *cs8900a_packetpp,
  *cs8900a_ppdata;

static u16_t len;
static u8_t r;

#define WRITE(P, D) do { PACKETPP = (P); \
		         PPDATA   = (D); } while(0)


/*-----------------------------------------------------------------------------------*/
void
cs8900a_init(void)
{
  /* Turn on transmission and reception of frames. */
  WRITE(0x0112, 0x00c0);

  /* Accept valid unicast+broadcast frames. */
  WRITE(0x0104, 0x0d05);

  /* Set MAC address. */
  WRITE(0x0158, (ETHADDR1 << 8) | (ETHADDR0));
  WRITE(0x015a, (ETHADDR3 << 8) | (ETHADDR2));
  WRITE(0x015c, (ETHADDR5 << 8) | (ETHADDR4));

}
/*-----------------------------------------------------------------------------------*/
void
cs8900a_send(void)
{
  if(uip_len > UIP_BUFSIZE) {
    return;
  }
    
  /* Transmit command. */
  /* *cs8900a_txcmd = 0x00c0 */
  asm("lda #$c0");
  asm("sta %v", cs8900a_txcmd);
  asm("lda #$00");
  asm("sta %v+1", cs8900a_txcmd);

  /* *cs8900a_txlen = uip_len */
  
  asm("lda _uip_len");
  asm("sta %v", cs8900a_txlen);
#if UIP_BUFSIZE > 255  
  asm("lda _uip_len+1");
#else
  asm("lda #0");
#endif      
  asm("sta %v+1", cs8900a_txlen);

  asm("ldx #8");
  asm("tryagain:");
  /* Check for avaliable buffer space. */
  asm("lda #$38");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v+1", cs8900a_ppdata);
  asm("and #1");
  asm("bne send");

  /* No space avaliable, skip a received frame and try again. */
  asm("lda #$02");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v", cs8900a_ppdata);
  asm("ora #$40");
  asm("sta %v", cs8900a_ppdata);

  asm("dex");
  asm("bne tryagain");
  
  asm("bailout:");
  return;

  /* Send the frame. */
  asm("send:");

  
  /* First, send 40+14=54 bytes of header. */
  
  asm("ldx #54");
  asm("ldy #0");
  asm("sendloop1:");
  asm("lda _uip_buf,y");
  asm("sta %v", cs8900a_rxtxreg);
  asm("lda _uip_buf+1,y");
  asm("sta %v+1", cs8900a_rxtxreg);
  asm("iny");
  asm("iny");
  asm("dex");
  asm("dex");
  asm("bne sendloop1");

  if(uip_len <= 54) {
    return;
  }

  /* Next, send rest of the packet. */

  uip_len -= 54;


  asm("lda ptr1");
  asm("pha");
  asm("lda ptr1+1");
  asm("pha");
  
  asm("lda _uip_appdata");
  asm("sta ptr1");
  asm("lda _uip_appdata+1");
  asm("sta ptr1+1");  

  asm("sendloop2:");
  asm("lda _uip_len");  
  asm("tax");
  asm("and #1");
  asm("beq noinc");
  asm("inx");
  asm("noinc:");
#if UIP_BUFSIZE > 255
  asm("lda _uip_len+1");
#else
  asm("lda #0");
#endif
  asm("beq nozero");
  asm("ldx #0");
  asm("nozero:");
  asm("ldy #0");
  asm("sendloop:");
  asm("lda (ptr1),y");
  asm("sta %v", cs8900a_rxtxreg);
  asm("iny");
  asm("lda (ptr1),y");
  asm("sta %v+1", cs8900a_rxtxreg);
  asm("iny");
  asm("dex");
  asm("dex");
  asm("bne sendloop");
  asm("inc ptr1+1");
#if UIP_BUFSIZE > 255
  asm("dec _uip_len+1");
  asm("bpl sendloop2");
#endif

  asm("pla");
  asm("sta ptr1+1");
  asm("pla");
  asm("sta ptr1");  
}
/*-----------------------------------------------------------------------------------*/
static void
skip_frame(void)
{
  WRITE(0x0102, PPDATA | 0x0040);
}
/*-----------------------------------------------------------------------------------*/
u8_t
cs8900a_poll(void)
{
  /* Check receiver event register to see if there are any valid
     unicast frames avaliable.  */
  /* PACKETPP = 0x0124;
     if(PPDATA & 0x000d == 0x0000) {
       return 0;
     }
  */
  asm("lda #$24");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v+1", cs8900a_ppdata);
  asm("and #$0d");
  asm("cmp #$00");
  asm("bne noreturn");
  /* No frame ready. */
  return 0;
  
  asm("noreturn:");
  /* Process the incoming frame. */
  
  /* Read receiver event and discard it. */
  /* dummy = RXTXREG; */
     
  asm("lda %v+1", cs8900a_rxtxreg);
  asm("sta _len+1");
  asm("lda %v", cs8900a_rxtxreg);
  asm("sta _len");
  
  /* Read frame length. */
  /* len = uip_len = RXTXREG; */
  asm("lda %v+1", cs8900a_rxtxreg);
  asm("sta _len+1");
  asm("sta _uip_len+1");
  asm("lda %v", cs8900a_rxtxreg);
  asm("sta _len");
  asm("sta _uip_len");

  
  if(len > UIP_BUFSIZE) {
    skip_frame();
    return 0;
  }
  
  /* Read bytes into uip_buf. */
  asm("lda ptr1");
  asm("pha");
  asm("lda ptr1+1");
  asm("pha");
  
  asm("lda #<_uip_buf");
  asm("sta ptr1");
  asm("lda #>_uip_buf");
  asm("sta ptr1+1");  
  
  asm("lda _len+1");
  asm("beq read256");
  
    /* Read first 256*n bytes. */
  asm("ldy #0");
  asm("read256loop:");
  asm("lda %v", cs8900a_rxtxreg);
  asm("sta (ptr1),y");
  asm("iny");
  asm("lda %v+1", cs8900a_rxtxreg);
  asm("sta (ptr1),y");
  asm("iny");
  asm("bne read256loop");
  asm("inc ptr1+1");
  
  asm("dec _len+1");
  asm("bne read256loop");
  
  /* Read last 255 or less bytes. */
  asm("read256:");
  asm("lda _len");
  asm("lsr");
  asm("bcc noinc");
  asm("inc _len");
  asm("noinc:");
  asm("ldy #$0");
  asm("readloop:");
  asm("lda %v", cs8900a_rxtxreg);
  asm("sta (ptr1),y");
  asm("iny");
  asm("lda %v+1", cs8900a_rxtxreg);
  asm("sta (ptr1),y");
  asm("iny");
  asm("cpy _len");
  asm("bne readloop");

  asm("pla");
  asm("sta ptr1+1");
  asm("pla");
  asm("sta ptr1");
  return len;
}
