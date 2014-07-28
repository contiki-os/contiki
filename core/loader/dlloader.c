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
 */
#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>

#include "contiki.h"

/*---------------------------------------------------------------------------*/
int
dlloader_load(char *path, char *arg)
{
  void *handle;
  struct process **p;

  /* Load and link the program. */
  handle = dlopen(path, RTLD_NOW);

  printf("Loading '%s'\n", path);
  
  if(handle == NULL) {
    printf("dlloader_load: loading failed: %s\n", dlerror());
    return LOADER_ERR_FMT;
  } 

  /* Find the processes to be started from the loaded program. */
  p = dlsym(handle, "autostart_processes");
  if(p == NULL) {
    printf("dlloader_load: could not find symbol 'autostart_processes'\n");
    return LOADER_ERR_FMT;
  }

  /* Start the process. */
 
  printf("Starting '%s'\n", PROCESS_NAME_STRING(*p));
  process_start(*p, (void *)arg);

  return LOADER_OK;
}
/*---------------------------------------------------------------------------*/
