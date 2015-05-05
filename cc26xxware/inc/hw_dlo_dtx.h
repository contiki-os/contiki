/******************************************************************************
*  Filename:       hw_dlo_dtx.h
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

#ifndef __HW_DLO_DTX_H__
#define __HW_DLO_DTX_H__


//*****************************************************************************
//
// This section defines the register offsets of
// DLO_DTX component
//
//*****************************************************************************
// Digital TX 5
#define DLO_DTX_O_DTX05                                             0x00000054

// FSM Control 1
#define DLO_DTX_O_FSMCTL1                                           0x00000064

// Spare 0
#define DLO_DTX_O_SPARE0                                            0x0000006C

//*****************************************************************************
//
// Register: DLO_DTX_O_DTX05
//
//*****************************************************************************
// Field: [31:24] SHAPE23
//
// Shape element 23
#define DLO_DTX_DTX05_SHAPE23_M                                     0xFF000000
#define DLO_DTX_DTX05_SHAPE23_S                                     24

// Field: [23:16] SHAPE22
//
// Shape element 22
#define DLO_DTX_DTX05_SHAPE22_M                                     0x00FF0000
#define DLO_DTX_DTX05_SHAPE22_S                                     16

// Field: [15:8] SHAPE21
//
// Shape element 21
#define DLO_DTX_DTX05_SHAPE21_M                                     0x0000FF00
#define DLO_DTX_DTX05_SHAPE21_S                                     8

// Field: [7:0] SHAPE20
//
// Shape element 20
#define DLO_DTX_DTX05_SHAPE20_M                                     0x000000FF
#define DLO_DTX_DTX05_SHAPE20_S                                     0

//*****************************************************************************
//
// Register: DLO_DTX_O_FSMCTL1
//
//*****************************************************************************
// Field: [7:0] FINE_START_CODE
//
// Starting code of fine bank. Used as a starting point for all calibration and at
// the start of PLL state. Remember to set FINE_START_CODE in the middle of
// DIGCFG2.FINE_BOT_CODE_DURING_CALIB and DIGCFG2.FINE_TOP_CODE_DURING_CALIB.
//
// Encoding: Binary
//
#define DLO_DTX_FSMCTL1_FINE_START_CODE_M                           0x000000FF
#define DLO_DTX_FSMCTL1_FINE_START_CODE_S                           0

//*****************************************************************************
//
// Register: DLO_DTX_O_SPARE0
//
//*****************************************************************************
#endif // __HW_DLO_DTX_H__
