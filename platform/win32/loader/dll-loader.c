/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>

#include "contiki.h"

/*---------------------------------------------------------------------------*/
int
dll_loader_load(char *name, char *arg)
{
  HMODULE handle;
  FARPROC p;

  /* Load and link the program. */
  handle = LoadLibrary(name);

  if(handle == NULL) {
    debug_printf("dll_loader_load: loading failed: %d\n", GetLastError());
    return LOADER_ERR_OPEN;
  } 

  /* Find the processes to be started from the loaded program. */
  p = GetProcAddress(handle, "autostart_processes");
  if(p == NULL) {
    debug_printf("dll_loader_load: could not find symbol 'autostart_processes'\n");
    return LOADER_ERR_READ;
  }

  /* Start the process. */
  debug_printf("Starting '%s'\n", (**(struct process ***)&p)->name);
  process_start(**(struct process ***)&p, (void *)arg);

  return LOADER_OK;
}
/*---------------------------------------------------------------------------*/
void
dll_loader_unload(void *addr)
{
  /* Avoid Access Violation Exception due to unloading code still being executed. */
  QueueUserAPC((PAPCFUNC)dll_loader_unload_dsc, GetCurrentThread(), (ULONG_PTR)addr);
}
/*---------------------------------------------------------------------------*/
struct dsc *
dll_loader_load_dsc(char *name)
{
  HMODULE handle;
  FARPROC d;
  char symbol[24];

  handle = LoadLibrary(name);

  if(handle == NULL) {
    debug_printf("dll_loader_load_dsc: loading failed: %d\n", GetLastError());
    return NULL;
  } 

  strcpy(symbol, name);
  *strchr(symbol, '.') = '_';

  d = GetProcAddress(handle, symbol);
  if(d == NULL) {
    debug_printf("dll_loader_load_dsc: could not find symbol '%s'\n", symbol);
    return NULL;
  }

  return *(struct dsc **)&d;
}
/*---------------------------------------------------------------------------*/
void __stdcall
dll_loader_unload_dsc(void *addr)
{
  HMODULE handle;

  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
		    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, addr, &handle);
  FreeLibrary(handle);
}
/*---------------------------------------------------------------------------*/
