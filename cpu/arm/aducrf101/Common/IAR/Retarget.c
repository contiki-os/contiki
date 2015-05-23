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
    Module       : Retarget.c
    Description  : uart interface
    Date         : December 2012
    Version      : v2.00
    Changelog    : v1.00 Initial
                v2.00 use of UrtLib functions
*/
#include <include.h>
#include "UrtLib.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define CR     0x0D



/*************************************************************************/
/* size_t __read(int handle,unsigned char *buf,size_t bufSize)           */
/*              Write data to a stream                                   */
/*        Needed for retargetting the IAR DLIB library for the ADUCRF101  */
/*************************************************************************/
size_t __read(int handle,unsigned char *buf,size_t bufSize)
{
   size_t i;
   for (i=0x0; i<bufSize;i++)
      {
      // Wait for character available
      while(!(COMLSR_DR==(pADI_UART->COMLSR & COMLSR_DR)));
      buf[i] = pADI_UART->COMRX;
      }
   return i;
}

/*************************************************************************/
/* __write(int handle,const unsigned char *buf,size_t bufSize)           */
/*              Read data from a stream                                  */
/*        Needed for retargetting the IAR DLIB library for the ADUCRF101  */
/*************************************************************************/
size_t __write(int handle,const unsigned char *buf,size_t bufSize)
{
   size_t i;
   for (i=0x0; i<bufSize;i++)
      {
      if (buf[i] == '\n')
         {
         while(!(COMLSR_THRE==(UrtLinSta(0) & COMLSR_THRE)));
         UrtTx(0, 0x0D);
         }
      while(!(COMLSR_THRE==(UrtLinSta(0) & COMLSR_THRE)));
      UrtTx(0, buf[i]);
      }
   return i;
}











