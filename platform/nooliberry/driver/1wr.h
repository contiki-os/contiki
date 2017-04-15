/*
 * Copyright (c) 2013, NooliTIC
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
 * \file 1WR.h
 *  Provides the functionality of 1 wire.
 *
 * \author
 *  Ludovic WIART <ludovic.wiart@noolitic.biz>
 *
 *  History:
 *     16/03/2012 L. Wiart - Created
 *     08/02/2013 S. Dawans - Code Style & Integration in Contiki fork
 */

/*----------------------------------------------------------------------------*
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 *----------------------------------------------------------------------------*/

#ifndef __1WR_H__
#define __1WR_H__

/*----------------------------------------------------------------------------*
                   Define(s) section
 *----------------------------------------------------------------------------*/

/* standard speed */
#define tSLOT	65
#define tRSTL	500
#define tMSP	60
#define tW1L	10
#define tW0L	65
#define tRL		5
#define tMSR	12
#define tREC	5

/* overdrive speed */
#define tOSLOT	8
#define tORSTL	60
#define tOMSP	6
#define tOW1L	1
#define tOW0L	6
#define tORL	1
#define tOMSR	2

void owr_writeb(unsigned byte);
unsigned owr_readb(void);
int owr_reset(void);

#endif /* __1WR_H__ */
/* eof 1WR.h */
