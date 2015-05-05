/******************************************************************************
*  Filename:       hw_device.h
*  Revised:        2015-01-13 16:59:55 +0100 (ti, 13 jan 2015)
*  Revision:       42365
*
*  Copyright (c) 2015, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __HW_DEVICE_H__
#define __HW_DEVICE_H__

#ifdef MODULE_CC26XX_7X7
//*****************************************************************************
//
// The following are defines for edge detection on wake up events for the
// CC26xx 7x7 packaged device.
//
//*****************************************************************************
#define AON_EVENT_DIO_0         18
#define AON_EVENT_DIO_1         17
#define AON_EVENT_DIO_2         16
#define AON_EVENT_DIO_3         15
#define AON_EVENT_DIO_4         14
#define AON_EVENT_DIO_5         13
#define AON_EVENT_DIO_6         12
#define AON_EVENT_DIO_7         11
#define AON_EVENT_DIO_8         10
#define AON_EVENT_DIO_9         9
#define AON_EVENT_DIO_10        8
#define AON_EVENT_DIO_11        7
#define AON_EVENT_DIO_12        6
#define AON_EVENT_DIO_13        5
#define AON_EVENT_DIO_14        4
#define AON_EVENT_DIO_15        3
#define AON_EVENT_DIO_16        2
#define AON_EVENT_DIO_17        1
#define AON_EVENT_DIO_18        31
#define AON_EVENT_DIO_19        30
#define AON_EVENT_DIO_20        29
#define AON_EVENT_DIO_21        28
#define AON_EVENT_DIO_22        27
#define AON_EVENT_DIO_23        26
#define AON_EVENT_DIO_24        25
#define AON_EVENT_DIO_25        24
#define AON_EVENT_DIO_26        23
#define AON_EVENT_DIO_27        22
#define AON_EVENT_DIO_28        21
#define AON_EVENT_DIO_29        20
#define AON_EVENT_DIO_30        19
#define AON_EVENT_DIO_31        0x3F
#endif // MODULE_CC26XX_7X7

#ifdef MODULE_CC26XX_5X5
//*****************************************************************************
//
// The following are defines for edge detection on wake up events for the
// CC26xx 5x5 packaged device.
//
//*****************************************************************************
#define AON_EVENT_DIO_0         15
#define AON_EVENT_DIO_1         14
#define AON_EVENT_DIO_2         13
#define AON_EVENT_DIO_3         12
#define AON_EVENT_DIO_4         11
#define AON_EVENT_DIO_5         2
#define AON_EVENT_DIO_6         1
#define AON_EVENT_DIO_7         26
#define AON_EVENT_DIO_8         25
#define AON_EVENT_DIO_9         23
#define AON_EVENT_DIO_10        24
#define AON_EVENT_DIO_11        22
#define AON_EVENT_DIO_12        21
#define AON_EVENT_DIO_13        20
#define AON_EVENT_DIO_14        19
#define AON_EVENT_DIO_15        0x3F
#define AON_EVENT_DIO_16        0x3F
#define AON_EVENT_DIO_17        0x3F
#define AON_EVENT_DIO_18        0x3F
#define AON_EVENT_DIO_19        0x3F
#define AON_EVENT_DIO_20        0x3F
#define AON_EVENT_DIO_21        0x3F
#define AON_EVENT_DIO_22        0x3F
#define AON_EVENT_DIO_23        0x3F
#define AON_EVENT_DIO_24        0x3F
#define AON_EVENT_DIO_25        0x3F
#define AON_EVENT_DIO_26        0x3F
#define AON_EVENT_DIO_27        0x3F
#define AON_EVENT_DIO_28        0x3F
#define AON_EVENT_DIO_29        0x3F
#define AON_EVENT_DIO_30        0x3F
#define AON_EVENT_DIO_31        0x3F
#endif // MODULE_CC26XX_5X5

#ifdef MODULE_CC26XX_4X4
//*****************************************************************************
//
// The following are defines for edge detection on wake up events for the
// CC26xx 4x4 packaged device.
//
//*****************************************************************************
#define AON_EVENT_DIO_0         13
#define AON_EVENT_DIO_1         12
#define AON_EVENT_DIO_2         11
#define AON_EVENT_DIO_3         2
#define AON_EVENT_DIO_4         1
#define AON_EVENT_DIO_5         26
#define AON_EVENT_DIO_6         25
#define AON_EVENT_DIO_7         24
#define AON_EVENT_DIO_8         23
#define AON_EVENT_DIO_9         22
#define AON_EVENT_DIO_10        0x3F
#define AON_EVENT_DIO_11        0x3F
#define AON_EVENT_DIO_12        0x3F
#define AON_EVENT_DIO_13        0x3F
#define AON_EVENT_DIO_14        0x3F
#define AON_EVENT_DIO_15        0x3F
#define AON_EVENT_DIO_16        0x3F
#define AON_EVENT_DIO_17        0x3F
#define AON_EVENT_DIO_18        0x3F
#define AON_EVENT_DIO_19        0x3F
#define AON_EVENT_DIO_20        0x3F
#define AON_EVENT_DIO_21        0x3F
#define AON_EVENT_DIO_22        0x3F
#define AON_EVENT_DIO_23        0x3F
#define AON_EVENT_DIO_24        0x3F
#define AON_EVENT_DIO_25        0x3F
#define AON_EVENT_DIO_26        0x3F
#define AON_EVENT_DIO_27        0x3F
#define AON_EVENT_DIO_28        0x3F
#define AON_EVENT_DIO_29        0x3F
#define AON_EVENT_DIO_30        0x3F
#define AON_EVENT_DIO_31        0x3F
#endif // MODULE_CC26XX_4X4

#endif // __HW_DEVICE_H__
