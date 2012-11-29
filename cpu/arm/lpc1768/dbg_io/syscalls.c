/*
 * syscalls.c
 *
 *  Created on: Nov 29, 2012
 *      Author: cazulu
 */

#include <sys/stat.h>
#include "debug_frmwrk.h"

#define CR     0x0D

int
_close(int file)
{
  return -1;
}

int
_fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

int
_isatty(int file)
{
  return 1;
}

int
_lseek(int file, int ptr, int dir)
{
  return 0;
}

int
_open(const char *name, int flags, int mode)
{
  return -1;
}

int
_read(int file, char *ptr, int len)
{
  int todo;
  if (len == 0)
    return 0;

  for (todo = 0; todo < len; todo++)
    {
      *ptr++ = _DG;
    }
  return todo;
}

/*
 * sbrk -- changes heap size size. Get nbytes more
 * RAM. We just increment a pointer in what’s
 * left of memory on the board.
 */
char * heap_end = 0;
caddr_t
_sbrk(int incr)
{
  extern char __heap_start__; /* Defined by the linker */
  extern char __heap_end__; /* Defined by the linker */
  char *prev_heap_end;

  if (heap_end == 0)
    {
      heap_end = &__heap_start__;
    }
  prev_heap_end = heap_end;

  if (heap_end + incr > &__heap_end__)
    {
      /* Heap and stack collision */
      return (caddr_t) 0;
    }

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int
_write(int file, char *ptr, int len)
{
  int todo;
  char ch;

  for (todo = 0; todo < len; todo++)
    {
      ch=*ptr++;
      _DBC(ch);
      if(ch=='\n')
        _DBC(CR);
    }
  return len;
}
