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
 */

#include <_ansi.h>
#include <stdio.h>
#include "local.h"

#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef _REENT_ONLY

int
#ifdef _HAVE_STDC
scanf (const char *fmt, ...)
#else
scanf (fmt, va_alist)
     char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;

  //_REENT_SMALL_CHECK_INIT(_stdin_r (_REENT));
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = __svfscanf_r (_REENT, _stdin_r (_REENT), fmt, ap);
  va_end (ap);
  return ret;
}

#endif /* !_REENT_ONLY */

int
#ifdef _HAVE_STDC
_scanf_r (struct _reent *ptr, const char *fmt, ...)
#else
_scanf_r (ptr, fmt, va_alist)
     struct _reent *ptr;
     char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;

  //_REENT_SMALL_CHECK_INIT(_stdin_r (ptr));
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = __svfscanf_r (ptr, _stdin_r (ptr), fmt, ap);
  va_end (ap);
  return (ret);
}


