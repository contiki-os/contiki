/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 *
 */

/**
 * \file
 *	ADC functions.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "contiki-msb430.h"

void
adc_init(void)
{
  ADC12CTL0 = SHT0_15 | SHT1_15 | MSC;

  /*
   * SHP: sampling timer
   * CONSEQ3: repeat sequence of channels
   * CSTARTADD: conversion start address 0
   */
  ADC12CTL1 = SHP | CONSEQ_3 | CSTARTADD_0;

  /* P60: A0 */
  ADC12MCTL0 = INCH_0 | SREF_0;
  ADC12MCTL1 = INCH_1 | SREF_0;
  ADC12MCTL2 = INCH_2 | SREF_0;

  /* P61: A1 */
  ADC12MCTL3 = ADC12MCTL4 = ADC12MCTL5 = INCH_1 | SREF_0;

  /* P62: A2 */
  ADC12MCTL6 = ADC12MCTL7 = ADC12MCTL8 = ADC12MCTL9 = INCH_2 | SREF_0;

  /* P63: A3 */
  ADC12MCTL10 = INCH_3 | SREF_0;

  /* P64: A4 */
  ADC12MCTL11 = INCH_4 | SREF_0;

  /* P65: A5 */
  ADC12MCTL12 = INCH_5 | SREF_0;

  /* INCH10: Temperature sensor. */
  ADC12MCTL13 = INCH_10 | SREF_0 | EOS;
}

void
adc_on(void)
{
  ADC12CTL0 |= ADC12ON;
  clock_delay(20000);
  ADC12CTL0 |= ENC;
  ADC12CTL0 |= ADC12SC;
}

void
adc_off(void)
{
  ADC12CTL0 &= ~ENC;
  clock_delay(20000);
  ADC12CTL0 &= ~ADC12ON;
}
