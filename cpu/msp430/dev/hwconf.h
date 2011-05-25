/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: hwconf.h,v 1.3 2010/01/14 20:05:58 nifi Exp $
 */
#ifndef __HWCONF_H__
#define __HWCONF_H__

#include "sys/cc.h"

#ifdef __GNUC__
#include <io.h>
#include <signal.h>
#endif

#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#endif 

#define HWCONF_PIN(name, port, bit)                                           \
static CC_INLINE void name##_SELECT() {P##port##SEL &= ~(1 << bit);}          \
static CC_INLINE void name##_SELECT_IO() {P##port##SEL &= ~(1 << bit);}       \
static CC_INLINE void name##_SELECT_PM() {P##port##SEL |= 1 << bit;}          \
static CC_INLINE void name##_SET() {P##port##OUT |= 1 << bit;}                \
static CC_INLINE void name##_CLEAR() {P##port##OUT &= ~(1 << bit);}           \
static CC_INLINE int  name##_READ() {return (P##port##IN & (1 << bit));}      \
static CC_INLINE void name##_MAKE_OUTPUT() {P##port##DIR |= 1 << bit;}        \
static CC_INLINE void name##_MAKE_INPUT() {P##port##DIR &= ~(1 << bit);}

#define HWCONF_IRQ(name, port, bit)                                           \
static CC_INLINE void name##_ENABLE_IRQ() {P##port##IE |= 1 << bit;}          \
static CC_INLINE void name##_DISABLE_IRQ() {P##port##IE &= ~(1 << bit);}      \
static CC_INLINE int  name##_IRQ_ENABLED() {return P##port##IE & (1 << bit);} \
static CC_INLINE void name##_IRQ_EDGE_SELECTD() {P##port##IES |= 1 << bit;}   \
static CC_INLINE void name##_IRQ_EDGE_SELECTU() {P##port##IES &= ~(1 << bit);}\
static CC_INLINE int  name##_CHECK_IRQ() {return P##port##IFG & (1 << bit);} \
static CC_INLINE int  name##_IRQ_PORT() {return port;}

#endif /* __HWCONF_H__ */
