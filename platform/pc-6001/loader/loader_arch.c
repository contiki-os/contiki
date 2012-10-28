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
 * 	loader-arch.c
 * 	A machine-dependent implementation of program loader for PC-6001 family.
 * \author
 * 	Takahide Matsutsuka <markn@markn.org>
 */

#ifdef WITH_LOADER_ARCH
#include "contiki.h"
#include "loader.h"
#include "loader/mef.h"
#include "loader/loader_arch.h"

unsigned char load_byte() { return 0; }

struct dsc*
loader_arch_load_dsc(char *name)
{
  mef_load((unsigned char *) 0xc800);
  log_message("loader_arch_load_dsc", name);
}
unsigned char
loader_arch_load(char *name, char *arg)
{
  log_message("loader_arch_load", name);
  return LOADER_ERR_NOLOADER;
}
void
loader_arch_unload()
{
  log_message("loader_arch_unload", name);
}
void
loader_arch_unload_dsc(struct dsc *dsc)
{
  log_message("loader_arch_unload_dsc", name);
}

#endif /* WITH_LOADER_ARCH */
