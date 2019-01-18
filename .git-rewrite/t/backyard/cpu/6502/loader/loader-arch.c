/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * This file is part of the Contiki desktop OS
 *
 * $Id: loader-arch.c,v 1.1 2007/05/23 23:50:02 oliverschmidt Exp $
 *
 */

#include <stdlib.h>
#include <modload.h>
#include <fcntl.h>
#include <unistd.h>

#include "loader.h"

struct mod_ctrl ctrl = {
  read            /* Read from disk */
};

struct loader_arch_hdr {
  char arch[8];
  char version[8];

  char initfunc[1];
};

/*-----------------------------------------------------------------------------------*/
/* load(name)
 *
 * Loads a program from disk and executes it. Code originally written by
 * Ullrich von Bassewitz.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
load(const char *name)
{
  unsigned char res;
  
  /* Now open the file */
  ctrl.callerdata = open(name, O_RDONLY);
  if(ctrl.callerdata < 0) {
    /* Could not open the file, display an error and return */
    /* ### */
    return LOADER_ERR_OPEN;
  }

  /* Load the module */
  res = mod_load(&ctrl);
  
  /* Close the input file */
  close(ctrl.callerdata);
  
  /* Check the return code */
  if(res != MLOAD_OK) {
    /* Wrong module, out of memory or whatever. Print an error
     * message and return.
     */
    /* ### */
    return res;
  }
  
  /* We've successfully loaded the module. */
  
  return LOADER_OK;
}
/*-----------------------------------------------------------------------------------*/
unsigned char
loader_arch_load(const char *name, char *arg)
{
  unsigned char r;
  struct loader_arch_hdr *hdr;
  
  r = load(name);
  if(r != MLOAD_OK) {
    return r;
  }
  hdr = (struct loader_arch_hdr *)ctrl.module;
  
  /* Check the program header and see that version and architecture
     matches. */
  
  /* Call the init function. */

  if(*hdr->initfunc == 0x4c) {
    ((void (*)(char *))hdr->initfunc)(arg);
  }

  return LOADER_OK;
}
/*-----------------------------------------------------------------------------------*/
