#ifdef INTEGER_ONLY
	#define 	_vfprintf_r  	_vfiprintf_r 
	#define 	_vfprintf  		_vfiprintf
	#define 	vfprintf 		vfiprintf
#endif

#include <_ansi.h>
#include <stdio.h>

#ifndef _SMALL_PRINTF
	#include "local.h"
#endif

#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef _SMALL_PRINTF

	#ifdef _HAVE_STDC
	int
	_printf_r (struct _reent *ptr, const char *fmt, ...)
	#else
	int
	_printf_r (ptr, fmt, va_alist)
	     struct _reent *ptr;
	     char *fmt;
	     va_dcl
	#endif
	{
	  int ret;
	  va_list ap;
	
	  //_REENT_SMALL_CHECK_INIT(_stdout_r (ptr));
	#ifdef _HAVE_STDC
	  va_start (ap, fmt);
	#else
	  va_start (ap);
	#endif
	  ret = _vfprintf_r (ptr, _stdout_r (ptr), fmt, ap);
	  va_end (ap);
	  return ret;
	}
#endif

#ifndef _REENT_ONLY

#ifdef _HAVE_STDC
int
printf (const char *fmt, ...)
#else
int
printf (fmt, va_alist)
     char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;

  //_REENT_SMALL_CHECK_INIT(_stdout_r (_REENT));
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
#ifndef _SMALL_PRINTF
  ret = vfprintf (_stdout_r (_REENT), fmt, ap);
#else
  ret = vfprintf (0, fmt, ap);
#endif
  va_end (ap);
  return ret;
}

#endif /* ! _REENT_ONLY */
