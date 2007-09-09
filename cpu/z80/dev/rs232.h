/*
 * Copyright (c) 2007, Takahide Matsutsuka.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * $Id: rs232.h,v 1.1 2007/09/09 13:41:15 matsutsuka Exp $
 *
 */
/*
 * \file
 * 	This is RS-232C process based on polling.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __RS_232C_H__
#define __RS_232C_H__

/*
 * Implement the following methods for each platform.
 */

/*
 * An architecture-depend implementation of RS-232C initialization.
 */
void rs232_arch_init(void);

/*
 * An architecture-depend implementation of RS-232C polling.
 * @return character, zero if no input.
 */
unsigned char rs232_arch_poll(void);

/*
 * An architecture-depend implementation of RS-232C writing a byte.
 */
void rs232_arch_writeb(u8_t ch);

PROCESS_NAME(rs232_process);

#endif /* __RS_232C_H__ */
