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
 */
/* for dladdr() */
#define _DARWIN_C_SOURCE
#define _GNU_SOURCE
#include <dlfcn.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "contiki.h"

/*---------------------------------------------------------------------------*/
int
dlloader_load(char *path, char *arg)
{
  void *handle;
  struct process **p;

  if(path == NULL || *path == '\0') {
    fprintf(stderr, "dlloader_load: bad name: %s\n", path);
    return LOADER_ERR_OPEN;
  }

  /* Load and link the program. */
  handle = dlopen(path, RTLD_NOW);

  fprintf(stderr, "Loading '%s'\n", path);

  if(handle == NULL) {
    fprintf(stderr, "dlloader_load: loading failed: %s\n", dlerror());
    return LOADER_ERR_FMT;
  }

  /* Find the processes to be started from the loaded program. */
  p = dlsym(handle, "autostart_processes");
  if(p == NULL) {
    fprintf(stderr,
            "dlloader_load: could not find symbol 'autostart_processes'\n");
    return LOADER_ERR_FMT;
  }

  /* Start the process. */
  fprintf(stderr, "Starting '%s'\n", (*p)->name);
  process_start(*p, arg);

  return LOADER_OK;
}
/*---------------------------------------------------------------------------*/
void
dlloader_unload(void *addr)
{
  /*XXX: cache handles ? */
}
/*---------------------------------------------------------------------------*/
struct dsc *
dlloader_load_dsc(char *name)
{
  void *handle;
  char symbol[24];
  void *d;

  if(name == NULL || *name == '\0') {
    fprintf(stderr, "dlloader_load_dsc: bad name: %s\n", name);
    return NULL;
  }

  /* Load and link the program. */
  handle = dlopen(name, RTLD_NOW);
  if(handle == NULL) {
    char path[PATH_MAX + 1];

    if(getwd(path)) {
      size_t l = strlen(path);

      //strlcat(path, "/", PATH_MAX);
      //strlcat(path, name, PATH_MAX);
      snprintf(path + l, PATH_MAX - l, "/%s", name);
      handle = dlopen(path, RTLD_NOW);
    }
  }

  fprintf(stderr, "Loading '%s'\n", name);

  if(handle == NULL) {
    fprintf(stderr, "dll_loader_load_dsc: loading failed: %s\n", dlerror());
    return NULL;
  }

  strcpy(symbol, name);
  if(*strchr(symbol, '.'))
    *strchr(symbol, '.') = '_';

  d = dlsym(handle, symbol);
  if(d == NULL) {
    fprintf(stderr, "dll_loader_load_dsc: could not find symbol '%s'\n",
            symbol);
    return NULL;
  }

  return *(struct dsc **)&d;
}
/*---------------------------------------------------------------------------*/
void
dlloader_unload_dsc(void *addr)
{
  /* TODO: we can't use dladdr() for this, use a linked list */
  Dl_info info;

  if(dladdr(addr, &info) != 0) {
    ;                           /*dlclose(info.); */
  }
}
/*---------------------------------------------------------------------------*/
