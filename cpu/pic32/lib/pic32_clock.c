/*
 * Contiki PIC32 Port project
 * 
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup pic32 PIC32 Contiki Port
 *
 * @{
 */

/** 
 * \file   pic32_clock.c
 * \brief  CLOCK interface for PIC32MX (pic32mx795f512l)
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-04-11
 */

/*
 * PIC32MX795F512L - Specific Functions
 * 
 * All the functions in this part of the file are specific for the
 * pic32mx795f512l that is characterized by registers' name that differ from
 * the 3xx and 4xx families of the pic32mx.
 */

#include <pic32_clock.h>

#include <p32xxxx.h>

#include <stdint.h>

#define CLOCK_SOURCE   8000000ul

#define DEVCFG2        0xBFC02FF4 /* virtual address of the register */

#define PLLIDIV_MASK   0x7

static const uint16_t FPPLIDIV[8] = { 1, 2, 3, 4, 5, 6, 10, 12 };
static const uint16_t PLLODIV[8] = { 1, 2, 4, 8, 16, 32, 64, 256 };
static const uint16_t PLLMULT[8] = { 15, 16, 17, 18, 19, 20, 21, 24 };
static const uint16_t PBDIV[4] = { 1, 2, 4, 8 };

/*---------------------------------------------------------------------------*/
/**
 * \brief  Calculate the system clock.
 *
 * \return the system clock value.
 */
uint32_t
pic32_clock_get_system_clock(void)
{
  return CLOCK_SOURCE *
         PLLMULT[(uint32_t) OSCCONbits.PLLMULT] /
         (FPPLIDIV[(*((uint32_t *) DEVCFG2)) & PLLIDIV_MASK] *
         PLLODIV[(uint32_t) OSCCONbits.PLLODIV]);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief  Calculate the peripheral clock.
 * 
 * \return the peripheral clock value.
 */
uint32_t
pic32_clock_get_peripheral_clock(void)
{
  return pic32_clock_get_system_clock() / PBDIV[OSCCONbits.PBDIV];
}
/*---------------------------------------------------------------------------*/
uint32_t
pic32_clock_calculate_brg(uint32_t mul, uint32_t desired)
{
  uint32_t fp = pic32_clock_get_peripheral_clock();

  uint32_t brg[3];
  uint32_t obtained[3];
  int32_t err[3];

  uint32_t min;

  brg[0] = fp / (mul * desired);     // +1
  brg[1] = fp / (mul * desired) - 1; //  0
  brg[2] = fp / (mul * desired) - 2; // -1

  obtained[0] = fp / (mul * (brg[0] + 1));
  obtained[1] = fp / (mul * (brg[1] + 1));
  obtained[2] = fp / (mul * (brg[2] + 1));

  err[0] = obtained[0] - desired;
  err[1] = obtained[1] - desired;
  err[2] = obtained[2] - desired;

  err[0] = err[0] < 0 ? -err[0] : err[0];
  err[1] = err[1] < 0 ? -err[1] : err[1];
  err[2] = err[2] < 0 ? -err[2] : err[2];

  min = 0;
  min = err[1] < err[min] ? 1 : min;
  min = err[2] < err[min] ? 2 : min;

  return brg[min];
}
/** @} */
