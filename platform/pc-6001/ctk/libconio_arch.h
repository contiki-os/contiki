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
 *
 */

/*
 * \file
 * 	This is architecture-depend contiki toolkit for PC-6001 family.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __LIBCONIO_ARCH_H__
#define __LIBCONIO_ARCH_H__

/* The following functions are used by assembler only */
#if LIBCONIO_EXPORT
void cputc_arch(char c);
void cputs_arch(char *str);
void cputsn_arch(char *str, unsigned char len);
void chline_arch(unsigned char length);
void cvline_arch(unsigned char length);
void clip_arch(unsigned char clip1, unsigned char clip2);
void clearto_arch(unsigned char to);
void revers_arch(unsigned char reversed);
void gotoxy_arch(unsigned char x, unsigned char y);
#endif /* LIBCONIO_CONF_EXPORT */
unsigned char wherex_arch();
unsigned char wherey_arch();

#endif /* __LIBCONIO_ARCH_H__ */
