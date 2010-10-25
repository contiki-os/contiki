/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	%W% (UofMD/Berkeley) %G%
 */

/*
 * Information local to this implementation of stdio,
 * in particular, macros and private variables.
 */

#include <_ansi.h>
#include <stdarg.h>
#include <reent.h>
#include <unistd.h>

extern int    _EXFUN(__svfscanf_r,(struct _reent *,FILE *, _CONST char *,va_list));
extern FILE  *_EXFUN(__sfp,(struct _reent *));
extern int    _EXFUN(__sflags,(struct _reent *,_CONST char*, int*));
extern int    _EXFUN(__srefill,(FILE *));
extern _READ_WRITE_RETURN_TYPE _EXFUN(__sread,(void *, char *, int));
extern _READ_WRITE_RETURN_TYPE _EXFUN(__swrite,(void *, char const *, int));
extern _fpos_t _EXFUN(__sseek,(void *, _fpos_t, int));
extern int    _EXFUN(__sclose,(void *));
extern int    _EXFUN(__stextmode,(int));
extern void   _EXFUN(__sinit,(struct _reent *));
extern void   _EXFUN(_cleanup_r,(struct _reent *));
extern void   _EXFUN(__smakebuf,(FILE *));
extern int    _EXFUN(_fwalk,(struct _reent *, int (*)(FILE *)));
struct _glue * _EXFUN(__sfmoreglue,(struct _reent *,int n));
extern int   _EXFUN(__srefill,(FILE *fp));

/* Called by the main entry point fns to ensure stdio has been initialized.  */

#define CHECK_INIT(fp) \
  do					\
    {					\
      if (!_REENT->__sdidinit)		\
	__sinit (_REENT);		\
    }					\
  while (0)

/* Return true iff the given FILE cannot be written now.  */

#define	cantwrite(fp) \
  ((((fp)->_flags & __SWR) == 0 || (fp)->_bf._base == NULL) && \
   __swsetup(fp))

/* Test whether the given stdio file has an active ungetc buffer;
   release such a buffer, without restoring ordinary unread data.  */

#define	HASUB(fp) ((fp)->_ub._base != NULL)
#define	FREEUB(fp) { \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		_free_r(_REENT, (char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

/* Test for an fgetline() buffer.  */

#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(fp) { _free_r(_REENT,(char *)(fp)->_lb._base); (fp)->_lb._base = NULL; }

/* WARNING: _dcvt is defined in the stdlib directory, not here!  */

char *_EXFUN(_dcvt,(struct _reent *, char *, double, int, int, char, int));
char *_EXFUN(_sicvt,(char *, short, char));
char *_EXFUN(_icvt,(char *, int, char));
char *_EXFUN(_licvt,(char *, long, char));
#ifdef __GNUC__
char *_EXFUN(_llicvt,(char *, long long, char));
#endif

#define CVT_BUF_SIZE 128

#define	NDYNAMIC 4	/* add four more whenever necessary */
