/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file ctrl_status.h ********************************************************
 *
 * \brief
 *      This file contains the interface :
 *         - between USB <-> MEMORY
 *      OR
 *         - between USB <- Access Memory Ctrl -> Memory
 *
 *      This interface may be controled by a "Access Memory Control" for :
 *         - include a management of write protect global or specific
 *         - include a management of access password
 *
 * \addtogroup usbstick
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 ******************************************************************************/
/* 
   Copyright (c) 2004  ATMEL Corporation
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/**
 \addtogroup usbstorage
 @{
*/

#ifndef _CTRL_STATUS_H_
#define _CTRL_STATUS_H_

//_____ D E F I N I T I O N S ______________________________________________

//!   Define control status
typedef enum
{
   CTRL_GOOD         =(PASS  )  // It is ready
,  CTRL_FAIL         =(FAIL  )  // Memory fail
,  CTRL_NO_PRESENT   =(FAIL+1)  // Memory unplug
,  CTRL_BUSY         =(FAIL+2)  // Not initialize
} Ctrl_status;

#endif   // _CTRL_STATUS_H_
/** @} */
