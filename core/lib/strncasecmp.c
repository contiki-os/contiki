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

/* This file contains a naive and non-standards compliant
   implementation of strncasecmp() for systems that don't implement
   the function. It works with Contiki, but should most probably not
   be used anywhere else.

   It copies the n first bytes two strings into two buffers and
   compares them with strcasecmp.
*/

#define MAX_STRLEN 40


int strncmp(const char *s1, const char *s2, int len);

/*static char buf1[MAX_STRLEN],
  buf2[MAX_STRLEN];*/
/*-----------------------------------------------------------------------------------*/
unsigned char
strncasecmp(const char *s1, const char *s2, unsigned char n)
{
  unsigned char len;

  return strncmp(s1, s2, n);
  
  /*  len = MAX_STRLEN;
  if(n < MAX_STRLEN) {
    len = n;
  }
  strncpy(buf1, s1, len);
  buf1[MAX_STRLEN - 1] = 0;
  strncpy(buf2, s2, len);
  buf2[MAX_STRLEN - 1] = 0;
  return strcasecmp(buf1, buf2);*/
}
/*-----------------------------------------------------------------------------------*/
