/******************************************************************************
*  Filename:       hw_cpu_tiprop.h
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

#ifndef __HW_CPU_TIPROP_H__
#define __HW_CPU_TIPROP_H__


//*****************************************************************************
//
// This section defines the register offsets of
// CPU_TIPROP component
//
//*****************************************************************************
// Mux Selector for SWV or TRACECLK
#define CPU_TIPROP_O_TRACECLKMUX                                    0x00000FF8

// Clock Gating Scheme Configuration
#define CPU_TIPROP_O_DYN_CG                                         0x00000FFC

//*****************************************************************************
//
// Register: CPU_TIPROP_O_TRACECLKMUX
//
//*****************************************************************************
// Field: [0]    TRACECLK_N_SWV
//
// Mux selector for SWV or TRACECLK
// This bit is reset when ITM is reset.
// ENUMs:
// SWV                  SWV
// TRACECLK             TRACECLK
//
#define CPU_TIPROP_TRACECLKMUX_TRACECLK_N_SWV                       0x00000001
#define CPU_TIPROP_TRACECLKMUX_TRACECLK_N_SWV_BITN                  0
#define CPU_TIPROP_TRACECLKMUX_TRACECLK_N_SWV_M                     0x00000001
#define CPU_TIPROP_TRACECLKMUX_TRACECLK_N_SWV_S                     0
#define CPU_TIPROP_TRACECLKMUX_TRACECLK_N_SWV_SWV                   0x00000000
#define CPU_TIPROP_TRACECLKMUX_TRACECLK_N_SWV_TRACECLK              0x00000001

//*****************************************************************************
//
// Register: CPU_TIPROP_O_DYN_CG
//
//*****************************************************************************
// Field: [1:0] DYN_CG
//
// Clock gating scheme configuration:
//
// 0: No clock gating from bus monitor
// 1: Clock gating as soon as the clock gating conditions are met (most
// aggressive).
// 2: Clock gating if the clock gating conditions stays valid more than one
// cycle.
// 3: Clock gating if the clock gating conditions stays valid more than two
// cycles.
//
// This field is reset when CPU core is reset.
#define CPU_TIPROP_DYN_CG_DYN_CG_M                                  0x00000003
#define CPU_TIPROP_DYN_CG_DYN_CG_S                                  0

#endif // __HW_CPU_TIPROP_H__
