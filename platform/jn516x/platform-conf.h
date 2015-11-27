/*
 * Copyright (c) 2015, SICS Swedish ICT.
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

#ifndef PLATFORM_CONF_H
#define PLATFORM_CONF_H

#include <inttypes.h>
#include <jendefs.h>

#undef putchar

/* Delay between GO signal and SFD
 * Measured 153us between GO and preamble. Add 5 bytes (preamble + SFD) air time: 153+5*32 = 313 */
#define RADIO_DELAY_BEFORE_TX ((unsigned)US_TO_RTIMERTICKS(313))
/* Delay between GO signal and start listening
 * Measured 104us: between GO signal and start listening */
#define RADIO_DELAY_BEFORE_RX ((unsigned)US_TO_RTIMERTICKS(104))

/* Micromac configuration */

#ifndef MIRCOMAC_CONF_BUF_NUM
#define MIRCOMAC_CONF_BUF_NUM 2
#endif

#ifndef MICROMAC_CONF_CHANNEL
#define MICROMAC_CONF_CHANNEL 26
#endif

#ifdef RF_CHANNEL
#define MICROMAC_CONF_CHANNEL RF_CHANNEL
#endif

/* Timer conversion
 * RTIMER 16M = 256 * 62500(RADIO)  == 2^8 * 62500 */
#define RADIO_TO_RTIMER(X)                      ((rtimer_clock_t)((X) << (int32_t)8L))

#define DR_11744_DIO2 12
#define DR_11744_DIO3 13
#define DR_11744_DIO4 14
#define DR_11744_DIO5 15
#define DR_11744_DIO6 16
#define DR_11744_DIO7 17

#define TSCH_DEBUG 0

#if TSCH_DEBUG
#define TSCH_DEBUG_INIT() do { \
    vAHI_DioSetDirection(0, (1 << DR_11744_DIO2) | (1 << DR_11744_DIO3) | (1 << DR_11744_DIO4) | (1 << DR_11744_DIO5) | (1 << DR_11744_DIO6) | (1 << DR_11744_DIO7)); \
    vAHI_DioSetOutput(0, (1 << DR_11744_DIO2) | (1 << DR_11744_DIO3) | (1 << DR_11744_DIO4) | (1 << DR_11744_DIO5) | (1 << DR_11744_DIO6) | (1 << DR_11744_DIO7)); } while(0);
#define TSCH_DEBUG_INTERRUPT() do { \
    static dio_state = 0; \
    dio_state = !dio_state; \
    if(dio_state) { \
      vAHI_DioSetOutput((1 << DR_11744_DIO2), 0); \
    } else { \
      vAHI_DioSetOutput(0, (1 << DR_11744_DIO2)); \
    } \
} while(0);
#define TSCH_DEBUG_RX_EVENT() do { \
    static dio_state = 0; \
    dio_state = !dio_state; \
    if(dio_state) { \
      vAHI_DioSetOutput((1 << DR_11744_DIO4), 0); \
    } else { \
      vAHI_DioSetOutput(0, (1 << DR_11744_DIO4)); \
    } \
} while(0);
#define TSCH_DEBUG_TX_EVENT() do { \
    static dio_state = 0; \
    dio_state = !dio_state; \
    if(dio_state) { \
      vAHI_DioSetOutput((1 << DR_11744_DIO5), 0); \
    } else { \
      vAHI_DioSetOutput(0, (1 << DR_11744_DIO5)); \
    } \
} while(0);
#define TSCH_DEBUG_SLOT_START() do { \
    static dio_state = 0; \
    dio_state = !dio_state; \
    if(dio_state) { \
      vAHI_DioSetOutput((1 << DR_11744_DIO3), 0); \
    } else { \
      vAHI_DioSetOutput(0, (1 << DR_11744_DIO3)); \
    } \
} while(0);
#define TSCH_DEBUG_SLOT_END()
#endif /* TSCH_DEBUG */

#ifndef BAUD2UBR
#define BAUD2UBR(X) (X)
#endif /* BAUD2UBR */

/* UART baud rates */
#define UART_RATE_4800          0
#define UART_RATE_9600          1
#define UART_RATE_19200         2
#define UART_RATE_38400         3
#define UART_RATE_76800         4
#define UART_RATE_115200        5
#define UART_RATE_230400        6
#define UART_RATE_460800        7
#define UART_RATE_500000        8
#define UART_RATE_576000        9
#define UART_RATE_921600        10
#define UART_RATE_1000000       11

#define PLATFORM_HAS_LEDS    1
#define PLATFORM_HAS_BUTTON  (SENSOR_BOARD_DR1174 == 1)
#define PLATFORM_HAS_LIGHT   (SENSOR_BOARD_DR1175 == 1)
#define PLATFORM_HAS_HT      (SENSOR_BOARD_DR1175 == 1)
#define PLATFORM_HAS_POT     (SENSOR_BOARD_DR1199 == 1)
#define PLATFORM_HAS_BATTERY 0 /* sensor driver not implemented */
#define PLATFORM_HAS_SHT11   0
#define PLATFORM_HAS_RADIO   1

/* CPU target speed in Hz
 * RTIMER and peripherals clock is F_CPU/2 */
#define F_CPU 32000000UL

/* LED ports */
/*
   #define LEDS_PxDIR P5DIR
   #define LEDS_PxOUT P5OUT
   #define LEDS_CONF_RED    0x10
   #define LEDS_CONF_GREEN  0x20
   #define LEDS_CONF_YELLOW 0x40
   #define JENNIC_CONF_BUTTON_PIN (IRQ_DIO9|IRQ_DIO10)
 */

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE                 inline

#define CCIF
#define CLIF

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#ifndef uint8_t
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef   signed char int8_t;
typedef          short int16_t;
typedef          long int32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;
#endif
#endif /* !HAVE_STDINT_H */

/* Types for clocks and uip_stats */
typedef uint16_t uip_stats_t;
typedef uint32_t clock_time_t;

/* Core rtimer.h defaults to 16 bit timer unless RTIMER_CLOCK_LT is defined */
typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_LT(a, b)     ((int32_t)((a) - (b)) < 0)
/* 10ms timer tick */
#define CLOCK_CONF_SECOND 100

/* Shall we calibrate the DCO periodically? */
#define DCOSYNCH_CONF_ENABLED 1

/* How often shall we attempt to calibrate DCO?
 * PS: It should be calibrated upon temperature changes,
 * but the naive approach of periodic calibration is fine too */
#ifndef DCOSYNCH_PERIOD
#define DCOSYNCH_PERIOD (5 * 60)
#endif /* VCO_CALIBRATION_INTERVAL */

/* Disable UART HW flow control */
#ifndef UART_HW_FLOW_CTRL
#define UART_HW_FLOW_CTRL 0
#endif /* UART_HW_FLOW_CTRL */

/* Disable UART SW flow control */
#ifndef UART_XONXOFF_FLOW_CTRL
#define UART_XONXOFF_FLOW_CTRL 1
#endif /* UART_XONXOFF_FLOW_CTRL */

#ifndef UART_BAUD_RATE
#define UART_BAUD_RATE UART_RATE_1000000
#endif /* UART_BAUD_RATE */

#ifndef UART1_BAUD_RATE
#define UART1_BAUD_RATE UART_RATE_1000000
#endif
#define ENABLE_ADVANCED_BAUD_SELECTION (UART_BAUD_RATE > UART_RATE_115200)

/* Set this to zero only if we are using SLIP */
#ifndef SLIP_BRIDGE_CONF_NO_PUTCHAR
#define SLIP_BRIDGE_CONF_NO_PUTCHAR 1
#endif /* SLIP_BRIDGE_CONF_NO_PUTCHAR */

/* Enable this to get the 32.768kHz oscillator */
#ifndef USE_EXTERNAL_OSCILLATOR
#define USE_EXTERNAL_OSCILLATOR 0
#endif /* USE_EXTERNAL_OSCILLATOR */

/* Extension of LED definitions from leds.h for various JN516x dev boards 
JN516x Dongle:
    LEDS_RED        Red LED on dongle
    LEDS_GREEN      Green LED on dongle
    Note: Only one LED can be switch on at the same time
      
DR1174-only:
    LEDS_GP0        LED D3 on DR1174
    LEDS_GP1        LED D6 on DR1174
    
DR1174+DR1199:
    LEDS_RED        LED D1 on DR1199                      
    LEDS_GREEN      LED D2 on DR1199
    LEDS_BLUE       LED D3 on DR1199
    LEDS_GP0        LED D3 on DR1174
    LEDS_GP1        LED D6 on DR1174
    
DR1174+DR1175:
    LEDS_RED        Red led in RGB-led with level control on DR1175    
    LEDS_GREEN      Green led in RGB-led with level control on DR1175    
    LEDS_BLUE       Blue led in RGB-led with level control on DR1175    
    LEDS_WHITE      White power led with level control on DR1175
    LEDS_GP0        LEDS D3 on DR1174
    LEDS_GP1        LEDS D6 on DR1174
*/
#define LEDS_WHITE    8
#define LEDS_GP0      16
#define LEDS_GP1      32 
#define LEDS_GP2      64
#define LEDS_GP3      128
#define LEDS_CONF_ALL 255
#endif /* PLATFORM_CONF_H */
