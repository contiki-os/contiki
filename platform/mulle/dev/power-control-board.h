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
 *         Power control pins for the on board power switches on the Mulle board.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 *
 */

#ifndef POWER_CONTROL_BOARD_H_
#define POWER_CONTROL_BOARD_H_

#include "K60.h"

/*
 * Hardware dependent defines below.
 */
/* Update these if the hardware pins are swapped or moved */

/* The number of the GPIO function on the pins in the IO mux */
/* See Pin multiplexing in K60 reference manual */
#define POWER_CONTROL_GPIO_MUX_NUMBER 1

/* Pin numbers within the port */
#define POWER_CONTROL_VPERIPH_PIN_NUMBER 7
#define POWER_CONTROL_VSEC_PIN_NUMBER 16
#define POWER_CONTROL_AVDD_PIN_NUMBER 17

/* Port Control Registers (PCR in the K60 reference manual) */
#define POWER_CONTROL_VPERIPH_PCR PORTD_PCR7
#define POWER_CONTROL_VSEC_PCR PORTB_PCR16
#define POWER_CONTROL_AVDD_PCR PORTD_PCR17

/* Clock gating bit masks (SIM_SCGC5 in the K60 reference manual) */
#define POWER_CONTROL_VPERIPH_SIM_SCGC5_MASK SIM_SCGC5_PORTD_MASK
#define POWER_CONTROL_VSEC_SIM_SCGC5_MASK SIM_SCGC5_PORTB_MASK
#define POWER_CONTROL_AVDD_SIM_SCGC5_MASK SIM_SCGC5_PORTB_MASK

/* GPIO data direction registers */
#define POWER_CONTROL_VPERIPH_GPIO_PDDR GPIOD_PDDR
#define POWER_CONTROL_VSEC_GPIO_PDDR GPIOB_PDDR
#define POWER_CONTROL_AVDD_GPIO_PDDR GPIOB_PDDR

/* GPIO Data, Set and Clear registers */
#define POWER_CONTROL_VPERIPH_GPIO_PDOR GPIOC_PDOR
#define POWER_CONTROL_VPERIPH_GPIO_PSOR GPIOC_PSOR
#define POWER_CONTROL_VPERIPH_GPIO_PCOR GPIOC_PCOR
#define POWER_CONTROL_VPERIPH_GPIO_PTOR GPIOC_PTOR
#define POWER_CONTROL_VSEC_GPIO_PDOR GPIOB_PDOR
#define POWER_CONTROL_VSEC_GPIO_PSOR GPIOB_PSOR
#define POWER_CONTROL_VSEC_GPIO_PCOR GPIOB_PCOR
#define POWER_CONTROL_VSEC_GPIO_PTOR GPIOB_PTOR
#define POWER_CONTROL_AVDD_GPIO_PDOR GPIOB_PDOR
#define POWER_CONTROL_AVDD_GPIO_PSOR GPIOB_PSOR
#define POWER_CONTROL_AVDD_GPIO_PCOR GPIOB_PCOR
#define POWER_CONTROL_AVDD_GPIO_PTOR GPIOB_PTOR

#endif
