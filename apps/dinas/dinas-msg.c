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
 *         DINAS message structure
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */

#include "dinas-msg.h"

/*---------------------------------------------------------------------------*/
uint8_t dinas_msg_set_config(uint8_t direction, uint8_t type, uint8_t ttl)
{
  uint8_t config = 0;
  if (direction == 1) /* up */
  	config += 1;
  if (type == 1) /* request */
  	config += 2;
  else if (type == 2) /* reply */		
  	config += 4;
  ttl = ttl * 8; /* shift left by 3*/
  config += ttl;	
  return config;	
}

/*---------------------------------------------------------------------------*/
uint8_t dinas_msg_get_direction(uint8_t config)
{
	config = config & 1; 
	return config; /* return 1 if direction bit is 1, 0 else*/
}

/*---------------------------------------------------------------------------*/
uint8_t dinas_msg_get_type(uint8_t config)
{
	config = config & 6;
	config = config/2; 
	return config; /* return 0 if type is 00, 1 if type is 01, 2 if type is 10 */
}

/*---------------------------------------------------------------------------*/
uint8_t dinas_msg_get_ttl(uint8_t config)
{
	config = config & 248;
	config = config/8; 
	return config; /* return TTL */
}




