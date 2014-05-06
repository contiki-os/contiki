/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Board configuration defines for Mulle platform.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

/**
 * CPU silicon revision (some registers are moved or added between revisions 1 and 2)
 */
#if !defined(MULLE_BOARD_SERIAL_NUMBER)
/* Default to revision 1 unless the serial number is specified in the build. */
#define K60_CPU_REV 1
#elif defined(MULLE_BOARD_SERIAL_NUMBER) && \
    (MULLE_BOARD_SERIAL_NUMBER >= 200) && \
    (MULLE_BOARD_SERIAL_NUMBER <= 219)
/* Only Mulles with serial numbers 200 through 219 have revision 1.x silicon
 * (revision 1.4, 4N30D mask set), see the sticker on the CPU top on the Mulle */
#define K60_CPU_REV 1
#else
/* Newer boards have the revision 2 silicon */
#define K60_CPU_REV 2
#endif

/**
 * Baud rate of the debug console (UART1)
 */
#define K60_DEBUG_BAUD 115200
