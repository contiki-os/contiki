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
 */
#ifndef __HWCONF_H__
#define __HWCONF_H__

#include "sys/cc.h"

#include <cc2430_sfr.h>

#define HWCONF_PIN(name, port, bit)                                           	\
static CC_INLINE void name##_SELECT() {P##port##SEL &= ~(1 << bit);}          	\
static CC_INLINE void name##_SELECT_IO() {P##port##SEL &= ~(1 << bit);}       	\
static CC_INLINE void name##_SELECT_PM() {P##port##SEL |= 1 << bit;}          	\
static CC_INLINE void name##_SET() {P##port##_##bit = 1; }                	\
static CC_INLINE void name##_CLEAR() {P##port##_##bit = 0; }                	\
static CC_INLINE unsigned char name##_READ() {return P##port##_##bit; }        \
static CC_INLINE void name##_MAKE_OUTPUT() {P##port##DIR |= 1 << bit;}        \
static CC_INLINE void name##_MAKE_INPUT() {P##port##DIR &= ~(1 << bit); }

#define HWCONF_IRQ_XXX(name, port, bit)                                           	\
static CC_INLINE void name##_ENABLE_IRQ() { 				      	\
  if ( port == 2 ) { PICTL |= P2IEN; p2ien |= 1<<bit; IEN2 |= P2IE; } 		   	\
  if (( port == 0) && ( bit <4)) { PICTL |= P0IENL; p0ien |= 1<<bit; IEN1 |= P0IE; }      \
  if ((port == 0) && ( bit >=4)) { PICTL |= P0IENH; p0ien |= 1<<bit; IEN1 |= P0IE; }     \
  if ( port == 1) { P##port##IEN |= 1 << bit; IEN2 |= P1IE; } 			      	\
} 	         							      	\
static CC_INLINE void name##_DISABLE_IRQ() { 				      	\
  if ( port == 2 ) { 						      	\
	p2ien &= ~(1<<bit); 						      	\
	if (p2ien==0) PICTL &= ~P2IEN;					      	\
  } 				      					      	\
  if (( port == 0) && ( bit <4)) { 					\
	p0ien &= ~(1<<bit); 							\
	if ((p0ien&0xf)==0) PICTL &= ~P0IENL; 					\
  } 		      								\
  if (( port == 0) && ( bit >=4)) { 					\
	p0ien &= ~(1<<bit);							\
	if ((p0ien&0xf0)==0) PICTL &= ~P0IENH;					\
  } 		      								\
  if ( port == 1) { P##port##IEN &= ~(1 << bit); } 			      	\
} 	         							      	\
static CC_INLINE int  name##_IRQ_ENABLED() {return P##port##IEN & (1 << bit);} 	\
static CC_INLINE int  name##_CHECK_IRQ() {return P##port##IFG & (1 << bit);} \
static CC_INLINE void  name##_IRQ_FLAG_OFF() { \
  P##port##IFG &= ~(1 << bit); \
  if (port == 0) {IRCON &= ~P0IF;} \
  else {IRCON2 &= ~P##port##IF;} \
}

#define HWCONF_IRQ(name, port, bit)                                           	\
static CC_INLINE void name##_ENABLE_IRQ() { 				      	\
  if ( port == 1) { P##port##IEN |= 1 << bit; } 			      	\
} 	         							      	\
static CC_INLINE void name##_DISABLE_IRQ() { 				      	\
  if ( port == 1) { P##port##IEN &= ~(1 << bit); } 			      	\
} 	         							      	\
static CC_INLINE int  name##_IRQ_ENABLED() {return P##port##IEN & (1 << bit);} 	\
static CC_INLINE int  name##_CHECK_IRQ() {return P##port##IFG & (1 << bit);} \
static CC_INLINE int  name##_IRQ_PORT() {return IRQ_PORT##port;}

#define HWCONF_PORT_0_IRQ(name, bit)                                             \
static CC_INLINE void name##_ENABLE_IRQ() {                                      \
  if ( bit <4 ) { PICTL |= P0IENL; p0ien |= 1<<bit; IEN1 |= P0IE; }              \
  if ( bit >=4 ) { PICTL |= P0IENH; p0ien |= 1<<bit; IEN1 |= P0IE; }             \
}                                                                                \
static CC_INLINE void name##_DISABLE_IRQ() {   \
  if ( bit <4) {                               \
    p0ien &= ~(1<<bit);                        \
    if ((p0ien&0xf)==0) PICTL &= ~P0IENL;      \
  }                                                                              \
  if ( bit >=4) {                              \
    p0ien &= ~(1<<bit);                        \
    if ((p0ien&0xf0)==0) PICTL &= ~P0IENH;     \
  }                                            \
 }                                                                               \
static CC_INLINE int  name##_IRQ_ENABLED() {return p0ien & (1 << bit);}          \
static CC_INLINE void name##_IRQ_EDGE_SELECTD() {PICTL |= P0ICON;}        \
static CC_INLINE void name##_IRQ_EDGE_SELECTU() {PICTL &= ~P0ICON;}       \
static CC_INLINE int  name##_CHECK_IRQ() {return P0IFG & (1 << bit);}            \
static CC_INLINE void  name##_IRQ_FLAG_OFF() { \
  IRCON_P0IF = 0; \
  P0IFG = 0; \
}

#define HWCONF_PORT_1_IRQ(name, bit)                                             \
static CC_INLINE void name##_ENABLE_IRQ()   { P1IEN |= 1 << bit; IEN2 |= P1IE; } \
static CC_INLINE void name##_DISABLE_IRQ()  { \
  P1IEN &= ~(1 << bit);                       \
  if (P1IEN == 0) { IEN2 &= ~P1IE; }          \
}                                                                                \
static CC_INLINE int  name##_IRQ_ENABLED()  { return P1IEN & (1 << bit); }       \
static CC_INLINE void name##_IRQ_EDGE_SELECTD() {PICTL |= P1ICON;}        \
static CC_INLINE void name##_IRQ_EDGE_SELECTU() {PICTL &= ~P1ICON;}       \
static CC_INLINE int  name##_CHECK_IRQ()    { return P1IFG & (1 << bit); }       \
static CC_INLINE void name##_IRQ_FLAG_OFF() {  \
  IRCON2_P1IF = 0;                             \
  P1IFG = 0;                                   \
}

#define HWCONF_PORT_2_IRQ(name, bit)                                             \
static CC_INLINE void name##_ENABLE_IRQ() { \
  PICTL |= P2IEN;                           \
  p2ien |= 1<<bit;                          \
  IEN2 |= P2IE;                             \
}                                                                                \
static CC_INLINE void name##_DISABLE_IRQ() {        \
  p2ien &= ~(1<<bit);                               \
  if (p2ien==0) { PICTL &= ~P2IEN; IEN2 &= ~P2IE; } \
}                                                                                \
static CC_INLINE int  name##_IRQ_ENABLED() {return p2ien & (1 << bit);}          \
static CC_INLINE void name##_IRQ_EDGE_SELECTD() {PICTL |= P2ICON;}        \
static CC_INLINE void name##_IRQ_EDGE_SELECTU() {PICTL &= ~P2ICON;}       \
static CC_INLINE int  name##_CHECK_IRQ() {return P2IFG & (1 << bit);}            \
static CC_INLINE void  name##_IRQ_FLAG_OFF() {  \
  IRCON2_P2IF = 0;                              \
  P2IFG = 0;                                    \
}

#endif /* __HWCONF_H__ */
