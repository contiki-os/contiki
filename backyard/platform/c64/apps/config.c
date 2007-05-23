/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * This file is part of the Contiki desktop OS.
 *
 * $Id: config.c,v 1.1 2007/05/23 23:11:24 oliverschmidt Exp $
 *
 */

#include "program-handler.h"
#include "contiki-net.h"
#include "cfs/cfs.h"

struct ptentry {
  char c;
  char * (* pfunc)(char *str);
};

/*-----------------------------------------------------------------------------------*/
static char *
parse(char *str, struct ptentry *t)
{
  struct ptentry *p;

  /* Loop over the parse table entries in t in order to find one that
     matches the first character in str. */
  for(p = t; p->c != 0; ++p) {
    if(*str == p->c) {
      /* Skip rest of the characters up to the first space. */
      while(*str != ' ') {
	++str;
      }

      /* Skip all spaces.*/
      while(*str == ' ') {
	++str;
      }

      /* Call parse table entry function and return. */
      return p->pfunc(str);
    }
  }

  /* Did not find matching entry in parse table. We just call the
     default handler supplied by the caller and return. */
  return p->pfunc(str);
}
/*-----------------------------------------------------------------------------------*/
static char *
skipnewline(char *str)
{
  /* Skip all characters until the newline. */
  while(*str != '\n') {
    ++str;
  }

  /* Return a pointer to the first character after the newline. */
  return str + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
nullterminate(char *str)
{
  char *nt;

  /* Nullterminate string. Start with finding newline character. */
  for(nt = str; *nt != '\r' &&
	*nt != '\n'; ++nt);

  /* Replace newline with a null char. */
  *nt = 0;

  /* Remove trailing spaces. */
  while(nt > str && *(nt - 1) == ' ') {
    *(nt - 1) = 0;
    --nt;
  }
  
  /* Return pointer to null char. */
  return nt;
}
/*-----------------------------------------------------------------------------------*/
static char *
loadfile(char *str)
{
  char *nt;

  nt = nullterminate(str);
  
  /* Call loader function. */
  program_handler_load(str, NULL);

  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
screensaverconf(char *str)
{
  char *nt;
  
  nt = nullterminate(str);
  program_handler_setscreensaver(str);

  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static u16_t addr[2];
static char *
ipaddrconf(char *str)
{
  char *nt;
  
  nt = nullterminate(str);
  if(uiplib_ipaddrconv(str, (unsigned char *)addr)) {
    uip_sethostaddr(addr);
  }

  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
netmaskconf(char *str)
{
  char *nt;
  
  nt = nullterminate(str);
  if(uiplib_ipaddrconv(str, (unsigned char *)addr)) {
    uip_setnetmask(addr);
  }

  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
drconf(char *str)
{
  char *nt;
  
  nt = nullterminate(str);
  if(uiplib_ipaddrconv(str, (unsigned char *)addr)) {
    uip_setdraddr(addr);
  }

  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static char *
dnsconf(char *str)
{
  char *nt;
  
  nt = nullterminate(str);
  if(uiplib_ipaddrconv(str, (unsigned char *)addr)) {
    resolv_conf(addr);
  }

  return nt + 1;
}
/*-----------------------------------------------------------------------------------*/
static struct ptentry configparsetab[] =
  {{'n', loadfile},
   {'t', loadfile},
   {'c', loadfile},   
   {'s', screensaverconf},   
   {'i', ipaddrconf},
   {'m', netmaskconf},
   {'r', drconf},
   {'d', dnsconf},
   {'#', skipnewline},

   /* Default action */
   {0, skipnewline}};
static void
configscript(void)
{
  static char line[40], *lineptr;
  /*  static struct c64_fs_file f;*/
  int f;

  if((f = cfs_open("config.cfg", 0)) == -1) {
    return;
  }

  line[0] = ' ';
  while(line[0] != '.' &&
	line[0] != 0) {
    lineptr = line;
    do {
      if(cfs_read(f, lineptr, 1) != 1) {
	cfs_close(f);
	return;
      }
      ++lineptr;
    } while(*(lineptr - 1) != '\n' &&
	    *(lineptr - 1) != '\r');

    *lineptr = 0;

    if(line[0] != '.' &&
       line[0] != 0) {
      parse(line, configparsetab);
    }
    
  }
  cfs_close(f);
  return;
}
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(config_init, arg)
{
  arg_free(arg);
  program_handler_setscreensaver(NULL);  
  configscript();
  LOADER_UNLOAD();
}
/*-----------------------------------------------------------------------------------*/
