/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
    Module       : retarget.c
   Description  :
    Date         : December 2012
    Version      : v2.00
    Changelog    : v1.00 Initial
                v2.00 use of UrtLib functions
*/

#include <stdio.h>
#include <rt_misc.h>
#include <include.h>

#pragma import(__use_no_semihosting_swi)

 #define CR     0x0D
struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;


// Re-targetting the Realview library functions
/*
 * writes the character specified by c (converted to an unsigned char) to
 * the output stream pointed to by stream, at the position indicated by the
 * asociated file position indicator (if defined), and advances the
 * indicator appropriately. If the file position indicator is not defined,
 * the character is appended to the output stream.
 * Returns: the character written. If a write error occurs, the error
 *          indicator is set and fputc returns EOF.
 */
int fputc(int ch, FILE * stream )
{
   if(ch == '\n')
      while(!(COMLSR_THRE==(UrtLinSta(0) & COMLSR_THRE)));
   UrtTx(0, CR); /* output CR */
   while(!(COMLSR_THRE==(UrtLinSta(0) & COMLSR_THRE)));
   UrtTx(0, ch);
   return(ch);
}

int __backspace(FILE *stream)
{
   return 0x0;

}
/*
 * obtains the next character (if present) as an unsigned char converted to
 * an int, from the input stream pointed to by stream, and advances the
 * associated file position indicator (if defined).
 * Returns: the next character from the input stream pointed to by stream.
 *          If the stream is at end-of-file, the end-of-file indicator is
 *          set and fgetc returns EOF. If a read error occurs, the error
 *          indicator is set and fgetc returns EOF.
 */
int fgetc(FILE * stream)
{
   return (UrtRx(0));
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch)       { UrtTx(0, ch); }


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
