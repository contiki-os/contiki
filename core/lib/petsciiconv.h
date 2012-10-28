/**
 * \file
 * PETSCII/ASCII conversion functions.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * The Commodore based Contiki targets all have a special character
 * encoding called PETSCII which differs from the ASCII encoding that
 * normally is used for representing characters.
 *
 * \note For targets that do not use PETSCII encoding the C compiler
 * define WITH_ASCII should be used to avoid the PETSCII converting
 * functions.
 *
 */

/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
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
 * This file is part of the Contiki desktop environment for the C64.
 *
 *
 */
#ifndef __PETSCIICONV_H__
#define __PETSCIICONV_H__

#ifdef WITH_PETSCII

#include "contiki-conf.h"

/**
 * Convert a text buffer from PETSCII to ASCII.
 *
 * \param buf A pointer to the buffer which is to be converted.
 * \param len The length of the buffer to be converted.
 */
void petsciiconv_toascii(char *buf, unsigned int len);
/**
 * Convert a text buffer from ASCII to PETSCII.
 *
 * \param buf A pointer to the buffer which is to be converted.
 * \param len The length of the buffer to be converted.
 */
void petsciiconv_topetscii(char *buf, unsigned int len);

#else /* WITH_PETSCII */

#define petsciiconv_toascii(buf, len)
#define petsciiconv_topetscii(buf, len)

#endif /* WITH_PETSCII */

#endif /* __PETSCIICONV_H__ */
