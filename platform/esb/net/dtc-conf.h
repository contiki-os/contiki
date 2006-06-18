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
 * @(#)$Id: dtc-conf.h,v 1.1 2006/06/18 07:49:33 adamdunkels Exp $
 */
/**
 * \file
 * Configuration options for the distributed TCP caching module.
 * \author Adam Dunkels <adam@sics.se>
 */

#ifndef __DTC_CONF_H__
#define __DTC_CONF_H__

#include "uipopt.h"

/**
 * The maximum size of a TCP segment (excluding headers) that can be
 * cached by the distributed TCP caching module.
 *
 * The distributed TCP caching module statically allocates memory for
 * holding cached TCP segments. This configuration parameter sets the
 * size of those blocks. The DTC_CONF_NUM_SEGMENTS configuration
 * parameter sets the number of packet caches that are allocated.
 *
 * \hideinitializer
 */
#define DTC_CONF_MAX_SEGSIZE UIP_TCP_MSS

/**
 * The number of TCP segments that the distributed TCP caching module can hold.
 *
 * This configuration parameter affects how much memory is allocated
 * to the TCP segment cache. The size of each cache block is set with
 * the DTC_CONF_MAX_SEGSIZE parameter.
 *
 * \hideinitializer
 */
#define DTC_CONF_NUM_SEGMENTS 1

/**
 * The number of simulataneous TCP connections that the distributed TCP caching can handle simulataneously.
 *
 * This configuration parameter specifies the maximum amount of
 * simultaneous TCP connections that can be handled by the distributed
 * TCP caching module. Note that this parameter does not limit the
 * amount of TCP connections that can go through the node, only how
 * many connections that will benefit from the help of the distributed
 * TCP caching mechanism.
 *
 * \hideinitializer
 */
#define DTC_CONF_NUM_CONNECTIONS 1

/**
 * Type definition of the native unsigned 32-bit datatype.
 *
 * This configuration option defines the native 32-bit datatype for
 * the platform on which the distributed TCP caching mechanism is to
 * be run. Normally, the unsigned 32-bit datatype is "unsigned long",
 * but check the manual for the C compiler to be sure.
 */
typedef unsigned long u32_t;

/**
 * Type definition of the native signed 32-bit datatype.
 *
 * This configuration option defines the native 32-bit datatype for
 * the platform on which the distributed TCP caching mechanism is to
 * be run. Normally, the signed 32-bit datatype is "signed long", but
 * check the manual for the C compiler to be sure.
 */
typedef signed long s32_t;

#endif /* __DTC_CONF_H__ */
