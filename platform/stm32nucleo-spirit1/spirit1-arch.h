/*
 * Copyright (c) 2012, STMicroelectronics.
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


#ifndef __SPIRIT1_ARCH_H__
#define __SPIRIT1_ARCH_H__

#include "string.h"
#include "SPIRIT_Management.h"
#include "radio_gpio.h"
#include "radio_spi.h"



#define IRQ_ENABLE()             RadioGpioInterruptCmd(RADIO_GPIO_IRQ,0x0F,0x0F,ENABLE);
#define IRQ_DISABLE()            RadioGpioInterruptCmd(RADIO_GPIO_IRQ,0x0F,0x0F,DISABLE); 
#define SDK_EVAL_GPIO_INIT()              RadioGpioInit(RADIO_GPIO_IRQ,M2S_MODE_EXTI_IN);
#define SDK_EVAL_SDN_GPIO_INIT()          RadioGpioInit(RADIO_GPIO_SDN, M2S_MODE_GPIO_OUT);
#define RadioSpiCSLow()        HAL_GPIO_WritePin(RADIO_SPI_CS_PORT, RADIO_SPI_CS_PIN, GPIO_PIN_RESET)
#define RadioSpiCSHigh()       HAL_GPIO_WritePin(RADIO_SPI_CS_PORT, RADIO_SPI_CS_PIN, GPIO_PIN_SET)
#define SpiritSPIBusy()        (!(RADIO_SPI_CS_PORT->IDR & RADIO_SPI_CS_PIN))
#define SPIRIT1_STATUS()       (spirit1_arch_refresh_status() & SPIRIT1_STATE_STATEBITS)

uint16_t spirit1_arch_refresh_status(void);



#endif /* __SPIRIT1_ARCH_H__ */
