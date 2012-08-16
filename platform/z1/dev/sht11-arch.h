/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark of Advancare,SL
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
 *	Architecture-specific definitions for the SHT11 sensor on Zolertia Z1
 *      when connected to the Ziglet port.
 * \author
 *      Enric M. Calvo <ecalvo@zolertia.com>, adapted from work by
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef SHT11_ARCH_H
#define SHT11_ARCH_H

// Architecture-specific definitions for the SHT11 sensor on Zolertia Z1
// when connected to the Ziglet port. 
// CAUTION: I2C needs to be disabled to use the bitbang protocol of SHT11

#define SHT11_ARCH_SDA	1	/* P5.1 */
#define SHT11_ARCH_SCL	2	/* P5.2 */
// SHT11_ARCH_PWR is not needed, but until and *-arch abstraction exists, this
// should fix it 
#define SHT11_ARCH_PWR	3	/* P5.3 -- unused pin */ 

#define	SHT11_PxDIR	P5DIR
#define SHT11_PxIN	P5IN
#define SHT11_PxOUT	P5OUT
#define SHT11_PxSEL	P5SEL
#define SHT11_PxREN     P5REN

#endif
