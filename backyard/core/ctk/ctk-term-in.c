/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: ctk-term-in.c,v 1.1 2007/05/26 21:54:33 oliverschmidt Exp $
 */
#include <string.h>
#include "ctk/ctk.h"
#include "ctk-term.h"
#include "ctk-term-int.h"
#include "ctk-term-ascii.h"

#define PRINTF(x) 

/*-----------------------------------------------------------------------------------*/
/* 
 * #defines and enums
 */
/*-----------------------------------------------------------------------------------*/

/* Size of input key buffer */
#define NUMKEYS 20 

/* ANSI character classes */
enum {ACC_C0, ACC_INTERM, ACC_PARAM, ACC_LOWCASE, ACC_UPCASE,ACC_C1, ACC_G1, ACC_DEL, ACC_SPEC };

/*-----------------------------------------------------------------------------------*/
/* 
 * Structures
 */
/*-----------------------------------------------------------------------------------*/

/* Structure for mapping a character sequence to a key */
struct seqmap
{
  const char* seq;
  const ctk_arch_key_t key;
};

/*-----------------------------------------------------------------------------------*/
/* 
 * Local variables
 */
/*-----------------------------------------------------------------------------------*/

static ctk_arch_key_t keys[NUMKEYS];
static int firstkey, lastkey;

/* Sequences starting with ESC [ .... */
const static struct seqmap ctrlmap[] =
{
  {"A",CH_CURS_UP},
  {"B",CH_CURS_DOWN},
  {"C",CH_CURS_RIGHT},
  {"D",CH_CURS_LEFT},

  {"11~",CH_F1},
  {"12~",CH_F2},
  {"13~",CH_F3},
  {"14~",CH_F4},
  // linux console
  {"[A",CH_F1},
  {"[B",CH_F2},
  {"[C",CH_F3},
  {"[D",CH_F4},
  {0,0}
};

/* Sequences starting with ESC O .... */
const static struct seqmap ss3map[] =
{
  {"A",CH_CURS_UP},
  {"B",CH_CURS_DOWN},
  {"C",CH_CURS_RIGHT},
  {"D",CH_CURS_LEFT},
  {"P",CH_F1},
  {"Q",CH_F2},
  {"R",CH_F3},
  {"S",CH_F4},
  {0,0}
};


/*-----------------------------------------------------------------------------------*/
/* 
 * Add a key to the input buffer
 */
/*-----------------------------------------------------------------------------------*/
static void 
enqueue_key(ctk_arch_key_t k)
{
  keys[lastkey] = k;
  ++lastkey;
  if(lastkey >= NUMKEYS) {
    lastkey = 0;
  }
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Classify a character 
 */
/*-----------------------------------------------------------------------------------*/
static unsigned char 
classify(unsigned char c)
{
  if (0x00 <= c && c <=0x1f) return ACC_C0;
  if (0x20 <= c && c <=0x2f) return ACC_INTERM;
  if (0x30 <= c && c <=0x3f) return ACC_PARAM;
  if (0x40 <= c && c <=0x5f) return ACC_UPCASE;
  if (0x60 <= c && c <=0x7e) return ACC_LOWCASE;
  if (c == 0x7f) return ACC_DEL;
  if (0x90 <= c && c <=0x9f) return ACC_C1;
  if (c == 0xa0) return ACC_SPEC;
  if (0xA1 <= c && c <=0xfe) return ACC_G1;
  if (0x90 <= c && c <=0x9f) return ACC_C1;
  if (c == 0xff) return ACC_SPEC;
  return ACC_SPEC;
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Lookup a key sequence in a sequencemap and queue the key if sequence found
 */
/*-----------------------------------------------------------------------------------*/
static void 
lookup_seq(const char* str, const struct seqmap* map)
{
  while (map->seq !=  0) {
    if (strcmp(str,map->seq) == 0) {
      enqueue_key(map->key);
      return;
    }
    map++;
  }
}

/*-----------------------------------------------------------------------------------*/
/* \internal
 * Parse a character stream 
 * Returns -1 if c is consumed by the state machine 1 else.
 */
/*-----------------------------------------------------------------------------------*/
static int 
parse_input(struct ctk_term_state* st, unsigned char c)
{
  unsigned char cl = classify(c);
  int ret = -1;
  switch(st->inputstate) {
  case ANS_IDLE:
    switch(cl) {
    case ACC_C0:
      {
	switch(c) {
	case ASCII_ESC: st->inputstate = ANS_ESCSEQ; break;
	case ASCII_BS:  enqueue_key(CH_DEL); break;
	case ASCII_HT:  enqueue_key(CH_TAB); break;
	case ASCII_FF:  ctk_term_redraw(st); break;
	case ASCII_CR:  enqueue_key(CH_ENTER); break;
	}
      }
      break;
      
    case ACC_INTERM:
    case ACC_PARAM:
    case ACC_LOWCASE:
    case ACC_UPCASE:
    case ACC_G1:
      ret = 1;
      break;
    case ACC_C1:
      if (c == ASCII_CSI) {
	st->inputstate = ANS_CTRLSEQ;
	st->ctrlCnt = 0;
      }
      else if (c == ASCII_SS3) {
	st->inputstate = ANS_SS3;
	st->ctrlCnt = 0;
      }
      break;
    case ACC_DEL:
      enqueue_key(CH_DEL);
      break;
    case ACC_SPEC:
      break;
    }
    break;
    
  case ANS_ESCSEQ:
    {
      switch(cl) {
      case ACC_C0:
      case ACC_DEL:
	break;
      case ACC_INTERM:
	st->inputstate = ANS_ESCSEQ_1;
	break;
      case ACC_UPCASE:
	/* C1 control character */
	if (c == '[') {
	  st->inputstate = ANS_CTRLSEQ;
	  st->ctrlCnt = 0;
	}
	else if (c == 'O') {
	  st->inputstate = ANS_SS3;
	  st->ctrlCnt = 0;
	}
	else {
	  st->inputstate = ANS_IDLE;
	}
	break;
      case ACC_PARAM:
	/* Private 2-character sequence */
      case ACC_LOWCASE:
	/* Standard 2-character sequence */
      default:
	st->inputstate = ANS_IDLE;
	break;
      }
    }
    break;
    
  case ANS_ESCSEQ_1:
    {
      switch(cl) {
      case ACC_C0:
      case ACC_INTERM:
	break;
      case ACC_PARAM:
	/* Private function*/
      case ACC_LOWCASE:
      case ACC_UPCASE:
	/* Standard function */
      default:
	st->inputstate = ANS_IDLE;
	break;
      }
    }
    break;
  case ANS_SS3:
    {
      switch(cl) {
      case ACC_PARAM:
	if (st->ctrlCnt < CTK_TERM_CTRLBUFLEN) st->ctrlbuf[st->ctrlCnt++]=c;
	break;
      case ACC_UPCASE:
	/* VT100 PF seq */
	if (st->ctrlCnt < CTK_TERM_CTRLBUFLEN) st->ctrlbuf[st->ctrlCnt++]=c;
	st->inputstate = ANS_IDLE;
	st->ctrlbuf[st->ctrlCnt] = 0;
	lookup_seq((const char*)(st->ctrlbuf), ss3map);
	break;
      default:
	st->inputstate = ANS_IDLE;
	break;
      }
    }
    break;
  case ANS_CTRLSEQ:
    {
      switch(cl) {
      case ACC_C0:
	break;
      case ACC_INTERM:
      case ACC_PARAM:
	if (st->ctrlCnt < CTK_TERM_CTRLBUFLEN) st->ctrlbuf[st->ctrlCnt++]=c;
	break;
      case ACC_LOWCASE:
      case ACC_UPCASE:
	/* Standard control sequence */
	if (st->ctrlCnt < CTK_TERM_CTRLBUFLEN) st->ctrlbuf[st->ctrlCnt++]=c;
	/* Cygwin console sends ESC [ [ A for function keys */
	if (c != '[') {
	  st->ctrlbuf[st->ctrlCnt] = 0;
	  lookup_seq((const char*)(st->ctrlbuf), ctrlmap);
	  st->inputstate = ANS_IDLE;
	}
	break;
      default:
	st->inputstate = ANS_IDLE;
	break;
      }
    }
    break;
  }
  return ret;
}

/*-----------------------------------------------------------------------------------*/
/* 
 * Initialize the input buffer
 */
/*-----------------------------------------------------------------------------------*/
void ctk_term_input_init()
{
  firstkey = lastkey = 0;
}

/*-----------------------------------------------------------------------------------*/
/** 
 * Handles an input character provided by the client
 *
 * \param ts State information
 * \param b  Input character
 */
/*-----------------------------------------------------------------------------------*/
void ctk_term_input(struct ctk_term_state* ts, unsigned char b)
{
  int ret = parse_input(ts, b);
  PRINTF(("terminput: 0x%02x\n", b));
  if (ret > 0) {
    enqueue_key((ctk_arch_key_t)b);
  }
}

/*-----------------------------------------------------------------------------------*/
/**
 * Checks the key press input queue to see if there are pending
 * keys. Called by the CTK module.
 *
 * \return Zero if no key presses are in buffer, non-zero if there are
 * key presses in input buffer.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
ctk_arch_keyavail(void)
{
  return firstkey != lastkey;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Retrieves key presses from the VNC client. Called by the CTK
 * module.
 *
 * \return The next key in the input queue.
 */
/*-----------------------------------------------------------------------------------*/
ctk_arch_key_t
ctk_arch_getkey(void)
{
  ctk_arch_key_t key;
  key = keys[firstkey];
  
  if(firstkey != lastkey) {
    ++firstkey;
    if(firstkey >= NUMKEYS) {
      firstkey = 0;
    }
  }
  
  return key;  
}

