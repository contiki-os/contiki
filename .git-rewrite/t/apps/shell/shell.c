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
 * $Id: shell.c,v 1.5 2007/08/30 14:39:16 matsutsuka Exp $
 *
 */

#include "program-handler.h"
#include "contiki-net.h"
#include "cfs/cfs.h"

#include "shell.h"

#include <string.h>

static char showingdir = 0;
static struct cfs_dir dir;
static unsigned int totsize;

struct ptentry {
  char c1;
  char c2;
  void (* pfunc)(char *str);
};

/*-----------------------------------------------------------------------------------*/
static void
parse(CC_REGISTER_ARG char *str, struct ptentry *t)
{
  register struct ptentry *p;
  char *sstr;

  sstr = str;
  
  /* Loop over the parse table entries in t in order to find one that
     matches the first character in str. */
  for(p = t; p->c1 != 0; ++p) {
    if(*str == p->c1 || *str == p->c2) {
      /* Skip rest of the characters up to the first space. */
      while(*str != ' ') {
	++str;
      }

      /* Skip all spaces.*/
      while(*str == ' ') {
	++str;
      }

      /* Call parse table entry function and return. */
      p->pfunc(str);
      return;
    }
  }

  /* Did not find matching entry in parse table. We just call the
     default handler supplied by the caller and return. */
  p->pfunc(str);
}
/*-----------------------------------------------------------------------------------*/
static void
inttostr(CC_REGISTER_ARG char *str, unsigned int i)
{
  str[0] = '0' + i / 100;
  if(str[0] == '0') {
    str[0] = ' ';
  }
  str[1] = '0' + (i / 10) % 10;
  if(str[0] == ' ' && str[1] == '0') {
    str[1] = ' ';
  }
  str[2] = '0' + i % 10;
  str[3] = ' ';
  str[4] = 0;
}
/*-----------------------------------------------------------------------------------*/
static void
processes(char *str)
{
  struct process *p;

  shell_output("Processes:", "");
  for(p = PROCESS_LIST(); p != NULL; p = p->next) {
    shell_output((char *)p->name, "");
  }
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
#if SHELL_CONF_WITH_PROGRAM_HANDLER
static void
runfile(char *str)
{
  nullterminate(str);

  if(strlen(str) > 0) {
    /* Call loader function. */
    program_handler_load(str, NULL);
    shell_output("Starting program ", str);  
  } else {
    shell_output("Must supply a program name", "");  
  }
}
/*-----------------------------------------------------------------------------------*/
static void
execfile(char *str)
{
  runfile(str);
  shell_quit(NULL);
}
#endif /* SHELL_CONF_WITH_PROGRAM_HANDLER */
/*-----------------------------------------------------------------------------------*/
static void
killproc(char *str)
{
  struct process *p;

  nullterminate(str);
  
  for(p = PROCESS_LIST(); p != NULL; p = p->next) {
    if(strcasecmp(p->name, str) == 0) {
      break;
    }
  }

  if(p != NULL) {
    shell_output("Killing process ", (char *)p->name);
    process_post(p, PROCESS_EVENT_EXIT, NULL);
  } else {
    shell_output("Invalid process name", "");
  }
}
/*-----------------------------------------------------------------------------------*/
static void
help(char *str)
{
  shell_output("Available commands:", "");
#if SHELL_CONF_WITH_PROGRAM_HANDLER
  shell_output("run  - start program", "");
  shell_output("exec - start program & exit shell", "");
#endif
  shell_output("ps   - show processes", "");
  shell_output("kill - kill process", "");
  shell_output("ls   - display directory", "");
  shell_output("quit - quit shell", "");
  shell_output("?    - show this help", "");      
}
/*-----------------------------------------------------------------------------------*/
static void
directory(char *str)
{
  if(cfs_opendir(&dir, ".") != 0) {
    shell_output("Cannot open directory", "");
    showingdir = 0;
  } else {
    shell_output("Disk directory:", "");
    showingdir = 1;
    totsize = 0;
    process_post(PROCESS_CURRENT(), PROCESS_EVENT_CONTINUE, NULL);
  }
  
}
/*-----------------------------------------------------------------------------------*/
static void
none(char *str)
{
}
/*-----------------------------------------------------------------------------------*/
static struct ptentry configparsetab[] =
  {
#if SHELL_CONF_WITH_PROGRAM_HANDLER
   {'e', 'E', execfile},
   {'r', 'R', runfile},
#endif
   {'k', 'K', killproc},   
   {'p', 'P', processes},
   {'l', 'L', directory},
   {'q', 'Q', shell_quit},
   {'h', '?', help},

   /* Default action */
   {0,   0,   none}};
/*-----------------------------------------------------------------------------------*/
void
shell_init(void)
{
}
/*-----------------------------------------------------------------------------------*/
void
shell_start(void)
{
  showingdir = 0;
  shell_output("Contiki command shell", "");
  shell_output("Type '?' and return for help", "");  
  shell_prompt("contiki> "); 
}
/*-----------------------------------------------------------------------------------*/
void
shell_input(char *cmd)
{
  if(showingdir != 0) {
    showingdir = 0;
    shell_output("Directory stopped", "");
    cfs_closedir(&dir);
  }
  parse(cmd, configparsetab);
  if(showingdir == 0) {
    shell_prompt("contiki> ");
  }
}
/*-----------------------------------------------------------------------------------*/
void
shell_eventhandler(process_event_t ev, process_data_t data)
{
  static struct cfs_dirent dirent;
  static char size[10];

  if(ev == PROCESS_EVENT_CONTINUE) {
    if(showingdir != 0) {
      if(cfs_readdir(&dir, &dirent) != 0) {
	cfs_closedir(&dir);
	showingdir = 0;
	inttostr(size, totsize);
	shell_output("Total number of blocks: ", size);
	shell_prompt("contiki> ");
      } else {
	totsize += dirent.size;
	inttostr(size, dirent.size);
	shell_output(size, dirent.name);
	process_post(PROCESS_CURRENT(), PROCESS_EVENT_CONTINUE, NULL);
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
