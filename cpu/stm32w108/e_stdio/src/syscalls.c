/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */


#if 0

/*You can comment this three lines if you want to use _gettimeofday and _times*/


#if defined(_SMALL_PRINTF) || defined(SMALL_SCANF)
#define NO_TIME
#endif

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>

#ifndef NO_TIME

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#endif

#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include "swi.h"

/* Forward prototypes.  */

int     _system     _PARAMS ((const char *));



int     _rename     _PARAMS ((const char *, const char *));
int     isatty		_PARAMS ((int));

#ifndef NO_TIME
clock_t _times		_PARAMS ((struct tms *));
int     _gettimeofday	_PARAMS ((struct timeval *, struct timezone *));
#endif


void    _raise 		_PARAMS ((void));

int     _unlink		_PARAMS ((void));
int     _link 		_PARAMS ((void));

int     _stat 		_PARAMS ((const char *, struct stat *));

int     _fstat 		_PARAMS ((int, struct stat *));

//caddr_t _sbrk		_PARAMS ((int));
int     _getpid		_PARAMS ((int));

int     _kill		_PARAMS ((int, int));
void    _exit		_PARAMS ((int));
int     _close		_PARAMS ((int));

int     _swiclose	_PARAMS ((int));
int     _open		_PARAMS ((const char *, int, ...));
int     _swiopen	_PARAMS ((const char *, int));
int     _write 		_PARAMS ((int, char *, int));
int     _swiwrite	_PARAMS ((int, char *, int));
int     _lseek		_PARAMS ((int, int, int));
int     _swilseek	_PARAMS ((int, int, int));
int     _read		_PARAMS ((int, char *, int));
int     _swiread	_PARAMS ((int, char *, int));
void    initialise_monitor_handles _PARAMS ((void));

static int	wrap		_PARAMS ((int));
static int	error		_PARAMS ((int));
static int	get_errno	_PARAMS ((void));
static int	remap_handle	_PARAMS ((int));
static int 	findslot	_PARAMS ((int));

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");


/* following is copied from libc/stdio/local.h to check std streams */
extern void   _EXFUN(__sinit,(struct _reent *));

#ifndef _SMALL_PRINTF
#define CHECK_INIT(fp) \
  do                                    \
    {                                   \
      if ((fp)->_data == 0)             \
        (fp)->_data = _REENT;           \
      if (!(fp)->_data->__sdidinit)     \
        __sinit ((fp)->_data);          \
    }                                   \
  while (0)
#endif
/* Adjust our internal handles to stay away from std* handles.  */
#define FILE_HANDLE_OFFSET (0x20)

static int std_files_checked;
static int monitor_stdin;
static int monitor_stdout;
static int monitor_stderr;

/* Struct used to keep track of the file position, just so we
   can implement fseek(fh,x,SEEK_CUR).  */
typedef struct
{
  int handle;
  int pos;
}
poslog;

#define MAX_OPEN_FILES 20
static poslog openfiles [MAX_OPEN_FILES];
/*
static int findslot (int fh)
{
  int i;
  for (i = 0; i < MAX_OPEN_FILES; i ++)
    if (openfiles[i].handle == fh)
      break;
  return i;
}
*/

/* Function to convert std(in|out|err) handles to internal versions.  */
/*
static int remap_handle (int fh)
{
  if (!std_files_checked)
    {
       CHECK_INIT(stdin);
       CHECK_INIT(stdout);
       CHECK_INIT(stderr);
       std_files_checked = 1;
    }
  if (fh == STDIN_FILENO)
    return monitor_stdin;
  if (fh == STDOUT_FILENO)
    return monitor_stdout;
  if (fh == STDERR_FILENO)
    return monitor_stderr;

  return fh - FILE_HANDLE_OFFSET;
}
*/

/*
void
initialise_monitor_handles (void)
{
  int i;
  
  int fh;
  const char * name;

  name = ":tt";
  asm ("mov r0,%2; mov r1, #0; swi %a1; mov %0, r0"
       : "=r"(fh)
       : "i" (SWI_Open),"r"(name)
       : "r0","r1");
  monitor_stdin = fh;

  name = ":tt";
  asm ("mov r0,%2; mov r1, #4; swi %a1; mov %0, r0"
       : "=r"(fh)
       : "i" (SWI_Open),"r"(name)
       : "r0","r1");
  monitor_stdout = monitor_stderr = fh;

  for (i = 0; i < MAX_OPEN_FILES; i ++)
    openfiles[i].handle = -1;

  openfiles[0].handle = monitor_stdin;
  openfiles[0].pos = 0;
  openfiles[1].handle = monitor_stdout;
  openfiles[1].pos = 0;
}
*/




static int get_errno (void)
{
  asm ("swi %a0" :: "i" (SWI_GetErrno));
}

static int error (int result)
{
  errno = get_errno ();
  return result;
}



static int wrap (int result)
{
  if (result == -1)
    return error (-1);
  return result;
}

#ifndef NO_FILE
int _read (int file,
       char * ptr,
       int len)
{
  return 0;
}

int _lseek (int file,
	int ptr,
	int dir)
{
  return 0;
}

extern void __io_putchar( char c );

int _write (int    file,
	char * ptr,
	int    len)
{
   int todo;
  
   for (todo = 0; todo < len; todo++)
   {
      __io_putchar( *ptr++ );
   }

   return len;
}

int _open (const char * path,
       int          flags,
       ...)
{
  return -1;
}

int _close (int file)
{
  return -1;
}

void _exit (int n)
{
  /* FIXME: return code is thrown away.  */
  while(1);
}



int _kill (int n, int m)
{
   return -1;
}


#if 0 //VC090825: moved to independent lib std_sbrk.lib
caddr_t _sbrk (int incr)
{
 extern char end;		/* Defined by the linker */
  static char *heap_end;
  char *prev_heap_end;
 
  if (heap_end == 0) {
    heap_end = &end;
  }
  prev_heap_end = heap_end;
  if (heap_end + incr > stack_ptr)
    {
      _write (1, "Heap and stack collision\n", 25);
      abort ();
    }

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}
#endif //if 0 //VC090825: moved to independent lib std_sbrk.lib
 	
#endif

#include <sys/stat.h>

#ifndef NO_FILE
int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}


#endif

int _stat (const char *fname, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}


#ifndef NO_FILE
int _link (void)
{
  return -1;
}

int _unlink (void)
{
  return -1;
}
#endif

void _raise (void)
{
  return;
}

#ifndef NO_TIME

int _gettimeofday (struct timeval * tp, struct timezone * tzp)
{

  if (tp)
    {
    /* Ask the host for the seconds since the Unix epoch.  */
      {
        int value;
        asm ("swi %a1; mov %0, r0" : "=r" (value): "i" (SWI_Time) : "r0");
        tp->tv_sec = value;
      }
      tp->tv_usec = 0;
    }

  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}



/* Return a clock that ticks at 100Hz.  */
clock_t _times (struct tms * tp)
{
  clock_t timeval;

  asm ("swi %a1; mov %0, r0" : "=r" (timeval): "i" (SWI_Clock) : "r0");

  if (tp)
    {
      tp->tms_utime  = timeval;	/* user time */
      tp->tms_stime  = 0;	/* system time */
      tp->tms_cutime = 0;	/* user time, children */
      tp->tms_cstime = 0;	/* system time, children */
    }
  
  return timeval;
};

#endif


int isatty (int fd)
{
  return 1;
  fd = fd;
}



int _system (const char *s)
{
  if (s == NULL)
    return 0;
  errno = ENOSYS;
  return -1;
}


#ifndef NO_FILE

int _rename (const char * oldpath, const char * newpath)
{
  errno = ENOSYS;
  return -1;
}

#endif

#endif
 

