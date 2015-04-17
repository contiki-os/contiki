/* Copyright (c) 2015, Michele Amoretti.
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
*/

/**
 * \file
 *         Configuration file for demo4
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */

#define UIP_CONF_TCP 0
#define PROCESS_CONF_NO_PROCESS_NAMES 1

#define MAX_NUM_MSG 21
#define NOTIFICATION_COUNTER 10 /* the pattern is N R R R R .. */
#define NUM_ROOMS 100  /* N = number of nodes */
#define TTL 3 /* this is D */
#define PERIOD 120
#define RANDWAIT (PERIOD)

/* overwrite params defined in bloom.h */
#define BLOOM_SIZE_PARAM 40
#define BLOOM_NFUNCS_PARAM 7

/* overwrite params defined in proximity-cache.h */
#define CACHE_SIZE_PARAM 8 /* this is C */
#define T1_PARAM 30
#define T2_PARAM 90

/* overwrite params defined in dinas-updown.h */
#define FLOODING_PARAM 0


