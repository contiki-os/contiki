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
 * $Id: cs8900a.c,v 1.1 2007/05/23 23:11:29 oliverschmidt Exp $
 *
 */

/* cs8900a.c: device driver for the CS8900a chip in 8-bit mode. Mostly
   written in 6502 assembler for speed. */

#include "cs8900a.h"
#include "contiki-net.h"

extern u8_t *cs8900a_rxtxreg,
            *cs8900a_txcmd,
            *cs8900a_txlen,
            *cs8900a_packetpp,
            *cs8900a_ppdata;

static u16_t len;
static u16_t cnt;


/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
cs8900a_init(void)
{
  /* Turn on transmission and reception of frames. */
  /* PACKETPP = 0x0112;
     PPDATA   = 0x00c0; */
  asm("lda #$12");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda #$c0");
  asm("sta %v", cs8900a_ppdata);
  asm("lda #$00");
  asm("sta %v+1", cs8900a_ppdata);

  /* Accept valid unicast+broadcast frames. */
  /* PACKETPP = 0x0104;
     PPDATA   = 0x0d05; */
  asm("lda #$04");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda #$05");
  asm("sta %v", cs8900a_ppdata);
  asm("lda #$0d");
  asm("sta %v+1", cs8900a_ppdata);

  /* Set MAC address. */
  /* PACKETPP = 0x0158;
     PPDATA   = (ETHADDR1 << 8) | (ETHADDR0); */
  asm("lda #$58");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v", uip_ethaddr);
  asm("sta %v", cs8900a_ppdata);
  asm("lda %v+1", uip_ethaddr);
  asm("sta %v+1", cs8900a_ppdata);

  /* PACKETPP = 0x015a;
     PPDATA   = (ETHADDR3 << 8) | (ETHADDR2); */
  asm("lda #$5a");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v+2", uip_ethaddr);
  asm("sta %v", cs8900a_ppdata);
  asm("lda %v+3", uip_ethaddr);
  asm("sta %v+1", cs8900a_ppdata);

  /* PACKETPP = 0x015c;
     PPDATA   = (ETHADDR5 << 8) | (ETHADDR4); */
  asm("lda #$5c");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v+4", uip_ethaddr);
  asm("sta %v", cs8900a_ppdata);
  asm("lda %v+5", uip_ethaddr);
  asm("sta %v+1", cs8900a_ppdata);
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
void
cs8900a_send(void)
{
  /* Transmit command. */
  asm("lda #$c0");
  asm("sta %v", cs8900a_txcmd);
  asm("lda #$00");
  asm("sta %v+1", cs8900a_txcmd);
  asm("lda %v", uip_len);
  asm("sta %v", cs8900a_txlen);
  asm("lda %v+1", uip_len);
  asm("sta %v+1", cs8900a_txlen);

  asm("ldy #8");
tryagain:
  /* Check for avaliable buffer space. */
  asm("lda #$38");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v+1", cs8900a_ppdata);
  asm("and #1");
  asm("bne %g", send);

  /* No space avaliable, skip a received frame and try again. */
  asm("lda #$02");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v", cs8900a_ppdata);
  asm("ora #$40");
  asm("sta %v", cs8900a_ppdata);

  asm("dey");
  asm("bne %g", tryagain);
  return;

  /* Send the frame. */
send:

  /* First, send 14+40=54 bytes of header. */
  asm("ldy #0");
sendloop1:
  asm("lda %v,y", uip_buf);
  asm("sta %v", cs8900a_rxtxreg);
  asm("iny");
  asm("lda %v,y", uip_buf);
  asm("sta %v+1", cs8900a_rxtxreg);
  asm("iny");
  asm("cpy #%b", UIP_LLH_LEN + UIP_TCPIP_HLEN);
  asm("bne %g", sendloop1);

  if(uip_len <= UIP_LLH_LEN + UIP_TCPIP_HLEN) {
    return;
  }

  /* Next, send rest of the packet. */
  cnt = uip_len - (UIP_LLH_LEN + UIP_TCPIP_HLEN);

  asm("lda %v", cnt);
  asm("lsr");
  asm("bcc %g", noinc);
  asm("inc %v", cnt);
  asm("bne %g", noinc);
  asm("inc %v+1", cnt);
noinc:

  asm("lda %v", uip_appdata);
  asm("sta ptr1");
  asm("lda %v+1", uip_appdata);
  asm("sta ptr1+1");  

  asm("ldy #0");
sendloop2:
  asm("lda (ptr1),y");
  asm("sta %v", cs8900a_rxtxreg);
  asm("iny");
  asm("lda (ptr1),y");
  asm("sta %v+1", cs8900a_rxtxreg);
  asm("iny");
  asm("bne %g", check);
  asm("inc ptr1+1");
check:
  asm("cpy %v", cnt);
  asm("bne %g", sendloop2);
  asm("dec %v+1", cnt);
  asm("bpl %g", sendloop2);
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
static void
skip_frame(void)
{
  /* PACKETPP = 0x0102;
     PPDATA   = PPDATA | 0x0040; */
  asm("lda #$02");
  asm("sta %v", cs8900a_packetpp);
  asm("lda #$01");
  asm("sta %v+1", cs8900a_packetpp);
  asm("lda %v", cs8900a_ppdata);
  asm("ora #$40");
  asm("sta %v", cs8900a_ppdata);
}
#pragma optimize(pop)
/*-----------------------------------------------------------------------------------*/
#pragma optimize(push, off)
u16_t
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
  asm("bne %g", noreturn);
  /* No frame ready. */
  return 0;
  
noreturn:
  /* Process the incoming frame. */
  
  /* Read receiver event and discard it. */
  /* RXTXREG; */
  asm("lda %v+1", cs8900a_rxtxreg);
  asm("lda %v", cs8900a_rxtxreg);
  
  /* Read frame length. */
  /* cnt = len = RXTXREG; */
  asm("lda %v+1", cs8900a_rxtxreg);
  asm("sta %v+1", len);
  asm("sta %v+1", cnt);
  asm("lda %v", cs8900a_rxtxreg);
  asm("sta %v", len);
  asm("sta %v", cnt);

  asm("lsr");
  asm("bcc %g", noinc);
  asm("inc %v", cnt);
  asm("bne %g", noinc);
  asm("inc %v+1", cnt);
noinc:

  if(cnt > UIP_BUFSIZE) {
    skip_frame();
    return 0;
  }

  /* Read bytes into uip_buf. */
  asm("lda #<%v", uip_buf);
  asm("sta ptr1");
  asm("lda #>%v", uip_buf);
  asm("sta ptr1+1");  
  
  asm("ldy #0");
readloop:
  asm("lda %v", cs8900a_rxtxreg);
  asm("sta (ptr1),y");
  asm("iny");
  asm("lda %v+1", cs8900a_rxtxreg);
  asm("sta (ptr1),y");
  asm("iny");
  asm("bne %g", check);
  asm("inc ptr1+1");
check:
  asm("cpy %v", cnt);
  asm("bne %g", readloop);
  asm("dec %v+1", cnt);
  asm("bpl %g", readloop);
  return len;
}
#pragma optimize(pop)
