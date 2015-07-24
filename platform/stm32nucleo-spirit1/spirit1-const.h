/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 *  \file
 *      spirit1-const.h
 *   \author
 *      Marcus Lunden <marcus@thingsquare.com>>
 *   \desc
 *      Spirit1 constants
 *      
 *      
 */

#ifndef __SPIRIT1_CONST_H__
#define __SPIRIT1_CONST_H__


/* The state bitfield and values for different states, as read from MC_STATE[1:0] registers, 
which are returned on any SPI read or write operation. */
#define SPIRIT1_STATE_STATEBITS           (0x00FE)

#define SPIRIT1_STATE_STANDBY             ((0x0040)<<1)
#define SPIRIT1_STATE_SLEEP               ((0x0036)<<1)
#define SPIRIT1_STATE_READY               ((0x0003)<<1)
#define SPIRIT1_STATE_LOCK                ((0x000F)<<1)
#define SPIRIT1_STATE_RX                  ((0x0033)<<1)
#define SPIRIT1_STATE_TX                  ((0x005F)<<1)
/* NB the below states were extracted from ST drivers, but are not specified in the datasheet */
#define SPIRIT1_STATE_PM_SETUP            ((0x003D)<<1)
#define SPIRIT1_STATE_XO_SETTLING         ((0x0023)<<1)
#define SPIRIT1_STATE_SYNTH_SETUP         ((0x0053)<<1)
#define SPIRIT1_STATE_PROTOCOL            ((0x001F)<<1)
#define SPIRIT1_STATE_SYNTH_CALIBRATION   ((0x004F)<<1)


/* strobe commands */
#define SPIRIT1_STROBE_TX             0x60
#define SPIRIT1_STROBE_RX             0x61
#define SPIRIT1_STROBE_READY          0x62
#define SPIRIT1_STROBE_STANDBY        0x63
#define SPIRIT1_STROBE_SLEEP          0x64
#define SPIRIT1_STROBE_SABORT         0x67
#define SPIRIT1_STROBE_SRES           0x70
#define SPIRIT1_STROBE_FRX            0x71
#define SPIRIT1_STROBE_FTX            0x72


#endif /* __SPIRIT1_CONST_H__ */

