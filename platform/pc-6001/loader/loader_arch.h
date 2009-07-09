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
 * $Id: loader_arch.h,v 1.2 2009/07/09 18:24:56 matsutsuka Exp $
 *
 */
 
/*
 * \file
 * 	loader-arch.h
 * 	Definitions of machine-dependent loader for PC-6001 family.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __LOADER_ARCH_H__
#define __LOADER_ARCH_H__

#ifdef WITH_LOADER_ARCH
#define LOADER_LOAD(name, arg) loader_arch_load(name, arg)
#define LOADER_UNLOAD()        loader_arch_unload()
#define LOADER_LOAD_DSC(name)  loader_arch_load_dsc(name)
#define LOADER_UNLOAD_DSC(dsc) loader_arch_unload_dsc(dsc)



struct dsc *loader_arch_load_dsc(char *name);
unsigned char loader_arch_load(char *name, char *arg);
void loader_arch_unload();
void loader_arch_unload_dsc(struct dsc *dsc);
#endif /* WITH_LOADER_ARCH */

#endif /* __LOADER_ARCH_H__ */
