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
 * @(#)$Id: me.c,v 1.1 2006/06/17 22:41:18 adamdunkels Exp $
 */

/** \addtogroup me
 * @{ */

/**
 * \file
 * Implementation of the table-driven Manchester encoding and decoding.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

#include "me_tabs.h"

/*---------------------------------------------------------------------------*/
/**
 * Manchester encode an 8-bit byte.
 *
 * This function Manchester encodes an 8-bit byte into a 16-bit
 * word. The function me_decode() does the inverse operation.
 *
 * \param c The byte to be encoded
 *
 * \retval The encoded word.
 */
/*---------------------------------------------------------------------------*/
unsigned short
me_encode(unsigned char c)
{
  return me_encode_tab[c];
}
/*---------------------------------------------------------------------------*/
/**
 * Decode a Manchester encoded 16-bit word.
 *
 * This function decodes a Manchester encoded 16-bit word into a 8-bit
 * byte. The function does not check for parity errors in the encoded
 * byte.
 *
 * \param m The 16-bit Manchester encoded word
 * \return The decoded 8-bit byte
 */
/*---------------------------------------------------------------------------*/
unsigned char
me_decode16(unsigned short m)
{
  unsigned char m1, m2, c;

  m1 = m >> 8;
  m2 = m & 0xff;

  c = (me_decode_tab[m1] << 4) |
    me_decode_tab[m2];
  return c;
}
/*---------------------------------------------------------------------------*/
/**
 * Decode a Manchester encoded 8-bit byte.
 *
 * This function decodes a Manchester encoded 8-bit byte into 4
 * decoded bits.. The function does not check for parity errors in the
 * encoded byte.
 *
 * \param m The 8-bit Manchester encoded byte
 * \return The decoded 4 bits
 */
/*---------------------------------------------------------------------------*/
unsigned char
me_decode8(unsigned char m)
{
  return  me_decode_tab[m];
}
/*---------------------------------------------------------------------------*/
/**
 * Check if an encoded byte is valid.
 */
/*---------------------------------------------------------------------------*/
unsigned char
me_valid(unsigned char m)
{
  return me_valid_tab[m];
}
/*---------------------------------------------------------------------------*/

/** @} */

