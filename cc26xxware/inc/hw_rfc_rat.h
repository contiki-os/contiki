/******************************************************************************
*  Filename:       hw_rfc_rat.h
*  Revised:        2015-01-15 18:41:47 +0100 (to, 15 jan 2015)
*  Revision:       42400
*
* Copyright (c) 2015, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1) Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2) Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3) Neither the name of the ORGANIZATION nor the names of its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __HW_RFC_RAT_H__
#define __HW_RFC_RAT_H__


//*****************************************************************************
//
// This section defines the register offsets of
// RFC_RAT component
//
//*****************************************************************************
// Radio Timer Counter Value
#define RFC_RAT_O_RATCNT                                            0x00000004

//*****************************************************************************
//
// Register: RFC_RAT_O_RATCNT
//
//*****************************************************************************
// Field: [31:0] CNT
//
// Counter value. This is not writable while radio timer counter is enabled.
#define RFC_RAT_RATCNT_CNT_M                                        0xFFFFFFFF
#define RFC_RAT_RATCNT_CNT_S                                        0

#endif // __HW_RFC_RAT_H__
