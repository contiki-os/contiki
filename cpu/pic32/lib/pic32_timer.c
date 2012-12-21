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
 * \file   pic32_timer.c
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

#define __TIMER_CODE_TEST__ 0

#if __TIMER_CODE_TEST__
#define __USE_TIMER__       1
#define __USE_TIMER_1__     1
#define __USE_TIMER_2__     1
#define __USE_TIMER_3__     1
#define __USE_TIMER_4__     1
#define __USE_TIMER_5__     1
#define __USE_TIMER_23__    1
#define __USE_TIMER_45__    1
#endif /* __TIMER_CODE_TEST__ */

#ifdef __USE_TIMER__

#include <pic32_timer.h>
#include <pic32_clock.h>
#include <pic32_irq.h>

#include <p32xxxx.h>

#include <stdint.h>
#include <stddef.h>

/*---------------------------------------------------------------------------*/
#define TIMERN_16(XX, TT, PP)                                                                         \
  void                                                                                                \
  pic32_timer##XX##_enable_irq(void)                                                                  \
  {                                                                                                   \
    IFS0CLR = _IFS0_T##XX##IF_MASK;                      /* Clean Timer IRQ Flag */                   \
    IEC0SET = _IEC0_T##XX##IE_MASK;                      /* Enable Timer IRQ */                       \
  }                                                                                                   \
                                                                                                      \
  void                                                                                                \
  pic32_timer##XX##_disable_irq(void)                                                                 \
  {                                                                                                   \
    IEC0CLR = _IEC0_T##XX##IE_MASK;                      /* Disable Timer IRQ */                      \
    IFS0CLR = _IFS0_T##XX##IF_MASK;                      /* Clean Timer IRQ Flag */                   \
  }                                                                                                   \
                                                                                                      \
  uint8_t                                                                                             \
  pic32_timer##XX##_init(uint32_t frequency)                                                          \
  {                                                                                                   \
    uint32_t prd = pic32_clock_get_peripheral_clock() / frequency;                                    \
    uint8_t tckps; /* Prescale */                                                                     \
    uint8_t ok = 0;                                                                                   \
                                                                                                      \
    if(prd <= UINT16_MAX) {                                                                           \
      tckps = TIMER_##TT##_PRESCALE_1;                                                                \
      ok = 1;                                                                                         \
    } else {                                                                                          \
      prd = prd / 8;                                                                                  \
    }                                                                                                 \
                                                                                                      \
    if(ok == 0) {                                                                                     \
      if(prd <= UINT16_MAX) {                                                                         \
        tckps = TIMER_##TT##_PRESCALE_8;                                                              \
        ok = 1;                                                                                       \
      } else {                                                                                        \
        prd = prd / 8;                                                                                \
      }                                                                                               \
    }                                                                                                 \
                                                                                                      \
    if(ok == 0) {                                                                                     \
      if(prd <= UINT16_MAX) {                                                                         \
        tckps = TIMER_##TT##_PRESCALE_64;                                                             \
        ok = 1;                                                                                       \
      } else {                                                                                        \
        prd = prd / 4;                                                                                \
      }                                                                                               \
    }                                                                                                 \
                                                                                                      \
    if(ok == 0) {                                                                                     \
      if(prd <= UINT16_MAX) {                                                                         \
        tckps = TIMER_##TT##_PRESCALE_256;                                                            \
      } else {                                                                                        \
        return -TIMER_ERR_BAD_ARGS;                                                                   \
      }                                                                                               \
    }                                                                                                 \
                                                                                                      \
    pic32_timer##XX##_disable_irq();                                                                  \
                                                                                                      \
    IPC##XX##CLR = _IPC##XX##_T##XX##IP_MASK | _IPC##XX##_T##XX##IS_MASK;                             \
    IPC##XX##SET = (7 << _IPC##XX##_T##XX##IP_POSITION) | (PP << _IPC##XX##_T##XX##IS_POSITION);      \
    T##XX##CON = 0;                                                                                   \
    T##XX##CONSET = tckps << _T##XX##CON_TCKPS_POSITION;                                              \
    PR##XX =  prd;                                                                                    \
    TMR##XX = 0;                                                                                      \
                                                                                                      \
    return TIMER_NO_ERRORS;                                                                           \
  }                                                                                                   \
                                                                                                      \
  void                                                                                                \
  pic32_timer##XX##_start(void)                                                                       \
  {                                                                                                   \
    T##XX##CONSET = _T##XX##CON_ON_MASK;                  /* Start Timer */                           \
  }                                                                                                   \
                                                                                                      \
  void                                                                                                \
  pic32_timer##XX##_stop(void)                                                                        \
  {                                                                                                   \
    T##XX##CONCLR = _T##XX##CON_ON_MASK;                  /* Stop Timer */                            \
  }                                                                                                   \
                                                                                                      \
  uint16_t                                                                                            \
  pic32_timer##XX##_get_val(void)                                                                     \
  {                                                                                                   \
      return TMR##XX;                                                                                 \
  }
/*---------------------------------------------------------------------------*/
#define TIMERN_32(XX, YY, PP)                                                                         \
                                                                                                      \
  void                                                                                                \
  pic32_timer##XX##YY##_enable_irq(void)                                                              \
  {                                                                                                   \
    pic32_timer##YY##_enable_irq();                                                                   \
  }                                                                                                   \
                                                                                                      \
  void                                                                                                \
  pic32_timer##XX##YY##_disable_irq(void)                                                             \
  {                                                                                                   \
    pic32_timer##YY##_disable_irq();                                                                  \
  }                                                                                                   \
                                                                                                      \
  uint8_t                                                                                             \
  pic32_timer##XX##YY##_init(uint32_t frequency)                                                      \
  {                                                                                                   \
    uint32_t prd = pic32_clock_get_peripheral_clock() / frequency;                                    \
    uint8_t tckps; /* Prescale */                                                                     \
    uint8_t ok = 0;                                                                                   \
                                                                                                      \
    if(prd <= UINT16_MAX) {                                                                           \
      tckps = TIMER_B_PRESCALE_1;                                                                     \
      ok = 1;                                                                                         \
    } else {                                                                                          \
      prd = prd / 8;                                                                                  \
    }                                                                                                 \
                                                                                                      \
    if(ok == 0) {                                                                                     \
      if(prd <= UINT16_MAX) {                                                                         \
        tckps = TIMER_B_PRESCALE_8;                                                                   \
        ok = 1;                                                                                       \
      } else {                                                                                        \
        prd = prd / 8;                                                                                \
      }                                                                                               \
    }                                                                                                 \
                                                                                                      \
    if(ok == 0) {                                                                                     \
      if(prd <= UINT16_MAX) {                                                                         \
        tckps = TIMER_B_PRESCALE_64;                                                                  \
        ok = 1;                                                                                       \
      } else {                                                                                        \
        prd = prd / 4;                                                                                \
      }                                                                                               \
    }                                                                                                 \
                                                                                                      \
    if(ok == 0) {                                                                                     \
      if(prd <= UINT16_MAX) {                                                                         \
        tckps = TIMER_B_PRESCALE_256;                                                                 \
      } else {                                                                                        \
        return -TIMER_ERR_BAD_ARGS;                                                                   \
      }                                                                                               \
    }                                                                                                 \
                                                                                                      \
    pic32_timer##XX##_disable_irq();                                                                  \
                                                                                                      \
    IPC##YY##CLR = _IPC##YY##_T##YY##IP_MASK | _IPC##YY##_T##YY##IS_MASK;                             \
    IPC##YY##SET = (7 << _IPC##YY##_T##YY##IP_POSITION) | (PP << _IPC##YY##_T##YY##IS_POSITION);      \
    T##XX##CON = 0;                                                                                   \
    T##XX##CONSET = _T##XX##CON_T32_MASK | (tckps << _T##XX##CON_TCKPS_POSITION);                     \
    PR##XX =  prd;                                                                                    \
    TMR##XX = 0;                                                                                      \
                                                                                                      \
    return TIMER_NO_ERRORS;                                                                           \
  }                                                                                                   \
                                                                                                      \
  void                                                                                                \
  pic32_timer##XX##YY##_start(void)                                                                   \
  {                                                                                                   \
    T##XX##CONSET = _T##XX##CON_ON_MASK;                  /* Start Timer */                           \
  }                                                                                                   \
                                                                                                      \
  void                                                                                                \
  pic32_timer##XX##YY##_stop(void)                                                                    \
  {                                                                                                   \
    T##XX##CONCLR = _T##XX##CON_ON_MASK;                  /* Stop Timer */                            \
  }                                                                                                   \
                                                                                                      \
  uint32_t                                                                                            \
  pic32_timer##XX##YY##_get_val(void)                                                                 \
  {                                                                                                   \
    return TMR##XX;                                                                                   \
  }
/*---------------------------------------------------------------------------*/

#ifdef __USE_TIMER_1__
TIMERN_16(1, A, 3)
#endif /* __USE_TIMER_1__ */

#if defined(__USE_TIMER_2__) || defined(__USE_TIMER_23__)
TIMERN_16(2, B, 2)
#endif /* __USE_TIMER_2__ */

#if defined(__USE_TIMER_3__) || defined(__USE_TIMER_23__)
TIMERN_16(3, B, 1)
#endif /* __USE_TIMER_3__ */

#if defined(__USE_TIMER_4__) || defined(__USE_TIMER_45__)
TIMERN_16(4, B, 1)
#endif /* __USE_TIMER_4__ */

#if defined(__USE_TIMER_5__) || defined(__USE_TIMER_45__)
TIMERN_16(5, B, 1)
#endif /* __USE_TIMER_5__ */

#ifdef __USE_TIMER_23__
TIMERN_32(2, 3, 3)
#endif /* __USE_TIMER_23__ */

#ifdef __USE_TIMER_45__
TIMERN_32(4, 5, 2)
#endif /* __USE_TIMER_45__ */

#endif /* __USE_TIMER__ */

/** @} */
