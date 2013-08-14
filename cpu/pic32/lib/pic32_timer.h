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
 * \file   pic32_timer.h
 * \brief  TIMER interface for PIC32MX (pic32mx795f512l)
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-26
 */

/*
 * PIC32MX795F512L - Specific Functions
 * 
 * All the functions in this part of the file are specific for the
 * pic32mx795f512l that is characterized by registers' name that differ from
 * the 3xx and 4xx families of the pic32mx.
 */

#ifndef __INCLUDE_PIC32_TIMER_H__
#define __INCLUDE_PIC32_TIMER_H__

#ifdef __USE_TIMER__

#include <pic32_irq.h>

#include <p32xxxx.h>

#include <stdint.h>

#define TIMER_NO_ERRORS             0
#define TIMER_ERR_BAD_ARGS          1

#define TIMER_A_PRESCALE_1          0b00
#define TIMER_A_PRESCALE_8          0b01
#define TIMER_A_PRESCALE_64         0b10
#define TIMER_A_PRESCALE_256        0b11

#define TIMER_B_PRESCALE_1          0b000
#define TIMER_B_PRESCALE_2          0b001
#define TIMER_B_PRESCALE_4          0b010
#define TIMER_B_PRESCALE_8          0b011
#define TIMER_B_PRESCALE_16         0b100
#define TIMER_B_PRESCALE_32         0b101
#define TIMER_B_PRESCALE_64         0b110
#define TIMER_B_PRESCALE_256        0b111

#define TIMERN_16_DEF(X)                             \
  uint8_t pic32_timer##X##_init(uint32_t frequency); \
  void pic32_timer##X##_enable_irq(void);            \
  void pic32_timer##X##_disable_irq(void);           \
  void pic32_timer##X##_start(void);                 \
  void pic32_timer##X##_stop(void);                  \
  uint16_t pic32_timer##X##_get_val(void);

#define TIMERN_32_DEF(XY)                            \
  uint8_t pic32_timer##XY##_init(uint32_t frequency);\
  void pic32_timer##XY##_enable_irq(void);           \
  void pic32_timer##XY##_disable_irq(void);          \
  void pic32_timer##XY##_start(void);                \
  void pic32_timer##XY##_stop(void);                 \
  uint32_t pic32_timer##XY##_get_val(void);

#define TIMER_INTERRUPT(XX, CALLBACK)                \
  TIMER_ISR(_TIMER_##XX##_VECTOR)                    \
  {                                                  \
    ENERGEST_ON(ENERGEST_TYPE_IRQ);                  \
    CALLBACK();                                      \
    ENERGEST_OFF(ENERGEST_TYPE_IRQ);                 \
    IFS0CLR = _IFS0_T##XX##IF_MASK;                  \
  }

#ifdef __USE_TIMER_1__
TIMERN_16_DEF(1)
#endif /* __USE_TIMER_1__ */

#if defined(__USE_TIMER_2__) || defined(__USE_TIMER_23__)
TIMERN_16_DEF(2)
#endif /* __USE_TIMER_2__ */

#if defined(__USE_TIMER_3__) || defined(__USE_TIMER_23__)
TIMERN_16_DEF(3)
#endif /* __USE_TIMER_3__ */

#if defined(__USE_TIMER_4__) || defined(__USE_TIMER_45__)
TIMERN_16_DEF(4)
#endif /* __USE_TIMER_4__ */

#if defined(__USE_TIMER_5__) || defined(__USE_TIMER_45__)
TIMERN_16_DEF(5)
#endif /* __USE_TIMER_5__ */

#ifdef __USE_TIMER_23__
TIMERN_32_DEF(23)
#endif /* __USE_TIMER_23__ */

#ifdef __USE_TIMER_45__
TIMERN_32_DEF(45)
#endif /* __USE_TIMER_45__ */

#endif /* __USE_TIMER__ */

#endif /* __INCLUDE_PIC32_TIMER_H__ */

/** @} */
