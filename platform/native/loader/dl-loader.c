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
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "contiki.h"
#include "lib/list.h"

struct dl_object {
  struct dl_object *next;
  void *handle;
  void *sym;
};

LIST(dl_objects);

PROCESS(dlcloser_process, "dlcloser");

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dlcloser_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit);

  PROCESS_BEGIN();
  fprintf(stderr, "dlcloser_process starting\n");

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_MSG) {
      fprintf(stderr, "doing defered dlclose(%p)\n", data);
      dlclose((void *)data);
    }
  }

exit:
  fprintf(stderr, "dlcloser_process exiting\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
dlloader_load(char *path, char *arg)
{
  void *handle;
  struct process **p;
  struct dl_object *o;
  static bool initialized = false;
  int flags = RTLD_NOW;

#ifdef RTLD_DEEPBIND
  /* work around autostart_processes being imported from main */
  flags |= RTLD_DEEPBIND;
#endif

  if(!initialized) {
    process_start(&dlcloser_process, NULL);
    initialized = true;
  }

  if(path == NULL || *path == '\0') {
    fprintf(stderr, "dlloader_load: bad name: %s\n", path);
    return LOADER_ERR_OPEN;
  }

  o = malloc(sizeof(struct dl_object));
  if(o == NULL) {
    fprintf(stderr, "dlloader_load: bad name: No memory\n");
    return LOADER_ERR_MEM;
  }

  /* Load and link the program. */
  handle = dlopen(path, flags);

  fprintf(stderr, "Loading '%s'\n", path);

  if(handle == NULL) {
    fprintf(stderr, "dlloader_load: loading failed: %s\n", dlerror());
    free(o);
    return LOADER_ERR_FMT;
  }

  /* Find the processes to be started from the loaded program. */
  p = dlsym(handle, "autostart_processes");
  if(p == NULL) {
    fprintf(stderr,
            "dlloader_load: could not find symbol 'autostart_processes'\n");
    free(o);
    return LOADER_ERR_FMT;
  }

  o->handle = handle;
  o->sym = p;
  list_add(dl_objects, o);

  /* Start the process. */
  fprintf(stderr, "Starting '%s'\n", (*p)->name);
  process_start(*p, arg);

  return LOADER_OK;
}
/*---------------------------------------------------------------------------*/
void
dlloader_unload(void *addr)
{
  struct dl_object *o;

  for(o = list_head(dl_objects); o != NULL; o = o->next) {
    if(o->sym == addr) {
      /* can't dlclose() yet, we're called from it */
      process_post(&dlcloser_process, PROCESS_EVENT_MSG, o->handle);
      list_remove(dl_objects, o);
      free(o);
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
struct dsc *
dlloader_load_dsc(char *name)
{
  void *handle;
  char symbol[24];
  void *d;
  struct dl_object *o;

  if(name == NULL || *name == '\0') {
    fprintf(stderr, "dlloader_load_dsc: bad name: %s\n", name);
    return NULL;
  }

  o = malloc(sizeof(struct dl_object));
  if(o == NULL) {
    fprintf(stderr, "dlloader_load: bad name: No memory\n");
    return NULL;
  }

  /* Load and link the program. */
  handle = dlopen(name, RTLD_NOW);
  if(handle == NULL) {
    char path[PATH_MAX];

    if(getcwd(path, sizeof(path))) {
      size_t l = strlen(path);

      /* strlcat(path, "/", PATH_MAX); */
      /* strlcat(path, name, PATH_MAX); */
      snprintf(path + l, PATH_MAX - l, "/%s", name);
      handle = dlopen(path, RTLD_NOW);
    }
  }

  fprintf(stderr, "Loading '%s'\n", name);

  if(handle == NULL) {
    fprintf(stderr, "dll_loader_load_dsc: loading failed: %s\n", dlerror());
    free(o);
    return NULL;
  }

  strcpy(symbol, name);
  if(*strchr(symbol, '.'))
    *strchr(symbol, '.') = '_';

  d = dlsym(handle, symbol);
  if(d == NULL) {
    fprintf(stderr, "dll_loader_load_dsc: could not find symbol '%s'\n",
            symbol);
    free(o);
    return NULL;
  }

  o->handle = handle;
  o->sym = d;
  list_add(dl_objects, o);

  return *(struct dsc **)&d;
}
/*---------------------------------------------------------------------------*/
void
dlloader_unload_dsc(void *addr)
{
  struct dl_object *o;

  for(o = list_head(dl_objects); o != NULL; o = o->next) {
    if(o->sym == addr) {
      dlclose(o->handle);
      list_remove(dl_objects, o);
      free(o);
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
