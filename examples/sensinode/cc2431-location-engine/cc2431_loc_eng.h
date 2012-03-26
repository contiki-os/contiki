/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Header file used by the example demonstrating the cc2431 location
 *         engine.
 *
 *         This file contains declarations of the location engine registers and
 *         the LOCENG register bits. It also contains some data structures used
 *         to store calculation parameters and reference node coordinates.
 *
 *         This file only needs to be included bye the blind node code file.
 *
 *         More information on the cc2431 Location Engine can be found in:
 *         - cc2431 Datasheet
 *         - K. Aamodt, "CC2431 Location Engine", Texas Instruments Application
 *           Note 42.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "8051def.h"
#include <lint.h> /* For syntax parsers */

/* Location Engine Registers on the cc2431 */
__xdata __at (0xDF55) unsigned char REFCOORD;
__xdata __at (0xDF56) unsigned char MEASPARM;
__xdata __at (0xDF57) unsigned char LOCENG;
__xdata __at (0xDF58) unsigned char LOCX;
__xdata __at (0xDF59) unsigned char LOCY;

/* LOCENG Register Bits */
#define LOCENG_RUN      0x01
#define LOCENG_REFLD    0x02
#define LOCENG_PARLD    0x04
#define LOCENG_DONE     0x08
#define LOCENG_EN       0x10

/* cc2431 chips report 0x89 when the CHIPID register is read */
#define CC2431_CHIP_ID  0x89

/*
 * Struct for the Calculation Parameters.
 * Values stored here feed the MEASPARM register.
 *
 * Values should be stored here in Location Engine format:
 *    RSSI: 0.5 Precision, without the minus sign. All 16 must be used. Use 0
 *          to reduce the number of ref. nodes used in the calculation.
 *          Value range [-95 dBm , -40 dBm]
 *       A: 0.5 Precision. Value range [30.0 , 50.0] (Thus [60 , 100] decimal)
 *       n: Use the n Index value [0 , 31] - See cc2431 datasheet, Table 2.
 *   delta: Must be present. If we want the calculation to be unrestricted,
 *          use 0xFF
 *
 */
struct meas_params {
  uint8_t alpha;
  uint8_t n;
  uint8_t x_min;
  uint8_t x_delta;
  uint8_t y_min;
  uint8_t y_delta;
  uint8_t rssi[16];
};

/*
 * Store the reference node coordinates here.
 * This will feed REFCOORD.
 *
 * Values should be stored here in Location Engine format:
 * 2 LS bits for the fractional part, 0.25 precision
 * 6 MS bits for the integral part.
 * Value range [0 , 63.75] (thus [0 , 255])
 */
struct refcoords {
  uint8_t x;
  uint8_t y;
};
