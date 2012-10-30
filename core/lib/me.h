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

/** \addtogroup lib
    @{ */
 
/**
 * \defgroup me Table-driven Manchester encoding and decoding
 *
 * Manchester encoding is a bit encoding scheme which translates each
 * bit into two bits: the original bit and the inverted bit.
 *
 * Manchester encoding is used for transmitting ones and zeroes
 * between two computers. The Manchester encoding reduces the receive
 * oscillator drift by making sure that no consecutive ones or zeroes
 * are ever transmitted.
 *
 * The table driven method of Manchester encoding and decoding uses
 * two tables with 256 entries. One table is a direct mapping of an
 * 8-bit byte into a 16-bit Manchester encoding of the byte. The
 * second table is a mapping of a Manchester encoded 8-bit byte to 4
 * decoded bits.
 *
 * @{
 */

/**
 * \file
 * Header file for the table-driven Manchester encoding and decoding
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __ME_H__
#define __ME_H__

unsigned char  me_valid(unsigned char m);
unsigned short me_encode(unsigned char c);
unsigned char  me_decode16(unsigned short m);
unsigned char  me_decode8(unsigned char m);

#endif /* __ME_H__ */

/** @} */
/** @} */
