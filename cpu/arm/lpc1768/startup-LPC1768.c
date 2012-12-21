/*
 * Copyright (c) 2012, KTH, Royal Institute of Technology(Stockholm, Sweden)
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 * This is work by the CSD master project. Fall 2012. Microgrid team.
 * Author: Javier Lara Peinado <javierlp@kth.se>
 *
 */

#include <stdint.h>
#include "system_LPC17xx.h"
#include "lpc17xx.h"

extern int
main(void);

typedef void
(*ISR_func)(void);

#define SECTION(x) __attribute__ ((section(#x)))
#define ISR_VECTOR_SECTION SECTION(.isr_vector)
static void
sys_reset(void) __attribute__((naked));
void
NMI_handler(void) __attribute__((interrupt));
void
HardFault_handler(void) __attribute__((interrupt));
void
MemManage_handler(void) __attribute__((interrupt));
void
BusFault_handler(void) __attribute__((interrupt));
void
UsageFault_handler(void) __attribute__((interrupt));

static void
unhandled_int(void) __attribute__((interrupt));

#define UNHANDLED_ALIAS __attribute__((weak, alias("unhandled_int")));
void
Main_Stack_End(void);
void
HardFault_handler(void)__attribute__((weak, alias("dHardFault_handler")));
void
MemManage_handler(void)__attribute__((weak, alias("dMemManage_handler")));
void
BusFault_handler(void) __attribute__((weak, alias("dBusFault_handler")));
void
UsageFault_handler(void)__attribute__((weak, alias("dUsageFault_handler")));
void
Reserved_handler(void) UNHANDLED_ALIAS
;
void
SVC_handler(void) UNHANDLED_ALIAS
;
void
DebugMon_handler(void) UNHANDLED_ALIAS
;
void
PendSV_handler(void) UNHANDLED_ALIAS
;
void
SysTick_handler(void) UNHANDLED_ALIAS
;
void
WDT_IRQHandler(void) UNHANDLED_ALIAS
;
void
TIMER0_IRQHandler(void) UNHANDLED_ALIAS
;
void
TIMER1_IRQHandler(void) UNHANDLED_ALIAS
;
void
TIMER2_IRQHandler(void) UNHANDLED_ALIAS
;
void
TIMER3_IRQHandler(void) UNHANDLED_ALIAS
;
void
UART0_IRQHandler(void) UNHANDLED_ALIAS
;
void
UART1_IRQHandler(void) UNHANDLED_ALIAS
;
void
UART2_IRQHandler(void) UNHANDLED_ALIAS
;
void
UART3_IRQHandler(void) UNHANDLED_ALIAS
;
void
PWM1_IRQHandler(void) UNHANDLED_ALIAS
;
void
I2C0_IRQHandler(void) UNHANDLED_ALIAS
;
void
I2C1_IRQHandler(void) UNHANDLED_ALIAS
;
void
I2C2_IRQHandler(void) UNHANDLED_ALIAS
;
void
SPI_IRQHandler(void) UNHANDLED_ALIAS
;
void
SSP0_IRQHandler(void) UNHANDLED_ALIAS
;
void
SSP1_IRQHandler(void) UNHANDLED_ALIAS
;
void
PLL0_IRQHandler(void) UNHANDLED_ALIAS
;
void
RTC_IRQHandler(void) UNHANDLED_ALIAS
;
void
EINT0_IRQHandler(void) UNHANDLED_ALIAS
;
void
EINT1_IRQHandler(void) UNHANDLED_ALIAS
;
void
EINT2_IRQHandler(void) UNHANDLED_ALIAS
;
void
EINT3_IRQHandler(void) UNHANDLED_ALIAS
;
void
ADC_IRQHandler(void) UNHANDLED_ALIAS
;
void
BOD_IRQHandler(void) UNHANDLED_ALIAS
;
void
USB_IRQHandler(void) UNHANDLED_ALIAS
;
void
CAN_IRQHandler(void) UNHANDLED_ALIAS
;
void
DMA_IRQHandler(void) UNHANDLED_ALIAS
;
void
I2S_IRQHandler(void) UNHANDLED_ALIAS
;
void
ENET_IRQHandler(void) UNHANDLED_ALIAS
;
void
RIT_IRQHandler(void) UNHANDLED_ALIAS
;
void
MCPWM_IRQHandler(void) UNHANDLED_ALIAS
;
void
QEI_IRQHandler(void) UNHANDLED_ALIAS
;
void
PLL1_IRQHandler(void) UNHANDLED_ALIAS
;
void
USBActivity_IRQHandler(void) UNHANDLED_ALIAS
;
void
CANActivity_IRQHandler(void) UNHANDLED_ALIAS
;

const ISR_func isr_vector[51] ISR_VECTOR_SECTION =
  { Main_Stack_End, /* Top of Stack                 */
  sys_reset, /* Reset Handler                */
  NMI_handler, /* NMI Handler                  */
  HardFault_handler, /* Hard Fault Handler           */
  MemManage_handler, /* MPU Fault Handler            */
  BusFault_handler, /* Bus Fault Handler            */
  UsageFault_handler, /* Usage Fault Handler          */
  Reserved_handler, /* Reserved                     */
  Reserved_handler, /* Reserved                     */
  Reserved_handler, /* Reserved                     */
  Reserved_handler, /* Reserved                     */
  SVC_handler,  /* SVCall Handler    */
  DebugMon_handler, /* Debug Monitor Handler */
  Reserved_handler, /* Reserved */
  PendSV_handler, /* PendSV Handler               */
  SysTick_handler, /* SysTick Handler              */

  /* External Interrupts */
  WDT_IRQHandler, /* 16: Watchdog Timer               */
  TIMER0_IRQHandler, /* 17: Timer0                       */
  TIMER1_IRQHandler, /* 18: Timer1                       */
  TIMER2_IRQHandler, /* 19: Timer2                       */
  TIMER3_IRQHandler, /* 20: Timer3                       */
  UART0_IRQHandler, /* 21: UART0                        */
  UART1_IRQHandler, /* 22: UART1                        */
  UART2_IRQHandler, /* 23: UART2                        */
  UART3_IRQHandler, /* 24: UART3                        */
  PWM1_IRQHandler, /* 25: PWM1                         */
  I2C0_IRQHandler, /* 26: I2C0                         */
  I2C1_IRQHandler, /* 27: I2C1                         */
  I2C2_IRQHandler, /* 28: I2C2                         */
  SPI_IRQHandler, /* 29: SPI                          */
  SSP0_IRQHandler, /* 30: SSP0                         */
  SSP1_IRQHandler, /* 31: SSP1                         */
  PLL0_IRQHandler, /* 32: PLL0 Lock (Main PLL)         */
  RTC_IRQHandler, /* 33: Real Time Clock              */
  EINT0_IRQHandler, /* 34: External Interrupt 0         */
  EINT1_IRQHandler, /* 35: External Interrupt 1         */
  EINT2_IRQHandler, /* 36: External Interrupt 2         */
  EINT3_IRQHandler, /* 37: External Interrupt 3         */
  ADC_IRQHandler, /* 38: A/D Converter                */
  BOD_IRQHandler, /* 39: Brown-Out Detect             */
  USB_IRQHandler, /* 40: USB                          */
  CAN_IRQHandler, /* 41: CAN                          */
  DMA_IRQHandler, /* 42: General Purpose DMA          */
  I2S_IRQHandler, /* 43: I2S                          */
  ENET_IRQHandler, /* 44: Ethernet                     */
  RIT_IRQHandler, /* 45: Repetitive Interrupt Timer   */
  MCPWM_IRQHandler, /* 46: Motor Control PWM            */
  QEI_IRQHandler, /* 47: Quadrature Encoder Interface */
  PLL1_IRQHandler, /* 48: PLL1 Lock (USB PLL)          */
  USBActivity_IRQHandler, /* 49: USB Activity                 */
  CANActivity_IRQHandler /* 50: CAN Activity                 */
  };

extern uint8_t _data[];
extern uint8_t _etext[];
extern uint8_t _edata[];

static void
copy_initialized(void)
{
  uint8_t *ram = _data;
  uint8_t *rom = _etext;
  while (ram < _edata)
    {
      *ram++ = *rom++;
    }
}

extern uint8_t __bss_start[];
extern uint8_t __bss_end[];

static void
clear_bss(void)
{
  uint8_t *m = __bss_start;
  while (m < __bss_end)
    {
      *m++ = 0;
    }
}

static void
sys_reset(void)
{
  copy_initialized();
  clear_bss();
  SystemInit();
  main();
  while (1)
    ;

}

void
NMI_handler(void)
{
  while (1)
    ;
}

static void
unhandled_int(void)
{
  while (1)
    ;
}

static void
dHardFault_handler(void)
{
  while (1)
    ;
}

static void
dUsageFault_handler(void)
{
  while (1)
    ;
}

static void
dMemManage_handler(void)
{
  while (1)
    ;
}

static void
dBusFault_handler(void)
{
  while (1)
    ;
}

