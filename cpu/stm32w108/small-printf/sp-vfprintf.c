/**
 * \addtogroup stm32w-cpu
 *
 * @{
 */

/*
FUNCTION
<<vprintf>>, <<vfprintf>>, <<vsprintf>>---format argument list

INDEX
	vprintf
INDEX
	vfprintf
INDEX
	vsprintf
INDEX
	vsnprintf

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int vprintf(const char *<[fmt]>, va_list <[list]>);
	int vfprintf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsprintf(char *<[str]>, const char *<[fmt]>, va_list <[list]>);
	int vasprintf(char **<[strp]>, const char *<[fmt]>, va_list <[list]>);
	int vsnprintf(char *<[str]>, size_t <[size]>, const char *<[fmt]>, va_list <[list]>);

	int _vprintf_r(void *<[reent]>, const char *<[fmt]>,
                        va_list <[list]>);
	int _vfprintf_r(void *<[reent]>, FILE *<[fp]>, const char *<[fmt]>,
                        va_list <[list]>);
	int _vasprintf_r(void *<[reent]>, char **<[str]>, const char *<[fmt]>,
                        va_list <[list]>);
	int _vsprintf_r(void *<[reent]>, char *<[str]>, const char *<[fmt]>,
                        va_list <[list]>);
	int _vsnprintf_r(void *<[reent]>, char *<[str]>, size_t <[size]>, const char *<[fmt]>,
                        va_list <[list]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	#include <varargs.h>
	int vprintf( <[fmt]>, <[list]>)
	char *<[fmt]>;
	va_list <[list]>;

	int vfprintf(<[fp]>, <[fmt]>, <[list]>)
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vasprintf(<[strp]>, <[fmt]>, <[list]>)
	char **<[strp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vsprintf(<[str]>, <[fmt]>, <[list]>)
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vsnprintf(<[str]>, <[size]>, <[fmt]>, <[list]>)
	char *<[str]>;
        size_t <[size]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vprintf_r(<[reent]>, <[fmt]>, <[list]>)
	char *<[reent]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vfprintf_r(<[reent]>, <[fp]>, <[fmt]>, <[list]>)
	char *<[reent]>;
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vasprintf_r(<[reent]>, <[strp]>, <[fmt]>, <[list]>)
	char *<[reent]>;
	char **<[strp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vsprintf_r(<[reent]>, <[str]>, <[fmt]>, <[list]>)
	char *<[reent]>;
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vsnprintf_r(<[reent]>, <[str]>, <[size]>, <[fmt]>, <[list]>)
	char *<[reent]>;
	char *<[str]>;
        size_t <[size]>;
	char *<[fmt]>;
	va_list <[list]>;

DESCRIPTION
<<vprintf>>, <<vfprintf>>, <<vasprintf>>, <<vsprintf>> and <<vsnprintf>> are 
(respectively) variants of <<printf>>, <<fprintf>>, <<asprintf>>, <<sprintf>>,
and <<snprintf>>.  They differ only in allowing their caller to pass the 
variable argument list as a <<va_list>> object (initialized by <<va_start>>) 
rather than directly accepting a variable number of arguments.

RETURNS
The return values are consistent with the corresponding functions:
<<vasprintf>>/<<vsprintf>> returns the number of bytes in the output string,
save that the concluding <<NULL>> is not counted.
<<vprintf>> and <<vfprintf>> return the number of characters transmitted.
If an error occurs, <<vprintf>> and <<vfprintf>> return <<EOF>> and
<<vasprintf>> returns -1.  No error returns occur for <<vsprintf>>.

PORTABILITY
ANSI C requires all three functions.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char *sccsid = "from: @(#)vfprintf.c	5.50 (Berkeley) 12/16/92";
#endif /* LIBC_SCCS and not lint */

/*
 * Actual printf innards.
 *
 * This code is large and complicated...
 */

#ifdef INTEGER_ONLY
	#define VFPRINTF vfiprintf
	#define _VFPRINTF_R _vfiprintf_r
#else
	#define VFPRINTF vfprintf
	#define _VFPRINTF_R _vfprintf_r
	#ifndef NO_FLOATING_POINT
	#define FLOATING_POINT
	#endif
#endif

#define _NO_LONGLONG
#if defined WANT_PRINTF_LONG_LONG && defined __GNUC__
# undef _NO_LONGLONG
#endif

#define _NO_POS_ARGS 
#if defined WANT_IO_POS_ARGS
# undef _NO_POS_ARGS
#endif

#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <reent.h>
#include <wchar.h>
#include <string.h>
#include <sys/lock.h>

#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef _SMALL_PRINTF
	#include "local.h"
	#include "fvwrite.h"
#else
	#define MAXBUFLOC 80
#endif

	#include "vfieeefp.h"

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the _ldtoa_r code be
   preferred over _dtoa_r.  */
#define _NO_LONGDBL
#if defined WANT_IO_LONG_DBL && (LDBL_MANT_DIG > DBL_MANT_DIG)
#	undef _NO_LONGDBL
#endif


#ifndef _SMALL_PRINTF
/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 */
static int
__sprint(fp, uio)
	FILE *fp;
	register struct __suio *uio;
{
	register int err;

	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}
	err = __sfvwrite(fp, uio);
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return (err);
}

/*
 * Helper function for `fprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int
__sbprintf(fp, fmt, ap)
	register FILE *fp;
	const char *fmt;
	va_list ap;
{
	int ret;
	FILE fake;
	unsigned char buf[BUFSIZ];

	/* copy the important variables */
	fake._flags = fp->_flags & ~__SNBF;
	fake._file = fp->_file;
	fake._cookie = fp->_cookie;
	fake._write = fp->_write;

	/* set up the buffer */
	fake._bf._base = fake._p = buf;
	fake._bf._size = fake._w = sizeof(buf);
	fake._lbfsize = 0;	/* not actually used, but Just In Case */
#ifndef __SINGLE_THREAD__
	__lock_init_recursive (*(_LOCK_RECURSIVE_T *)&fake._lock);
#endif

	/* do the work, then copy any error status */
	ret = VFPRINTF(&fake, fmt, ap);
	if (ret >= 0 && fflush(&fake))
		ret = EOF;
	if (fake._flags & __SERR)
		fp->_flags |= __SERR;

#ifndef __SINGLE_THREAD__
	__lock_close_recursive (*(_LOCK_RECURSIVE_T *)&fake._lock);
#endif
	return (ret);
}
#endif



#ifdef FLOATING_POINT
#include <locale.h>
#include <math.h>
#include "floatio.h"

#if ((MAXEXP+MAXFRACT+1) > MB_LEN_MAX)
#  define BUF (MAXEXP+MAXFRACT+1) /* + decimal point */
#else 
#  define BUF MB_LEN_MAX
#endif

#define	DEFPREC		6

#ifdef _NO_LONGDBL
static char *cvt _PARAMS((struct _reent *, double, int, int, char *, int *, int, int *));
#else
static char *cvt _PARAMS((struct _reent *, _LONG_DOUBLE, int, int, char *, int *, int, int *));
extern int  _ldcheck _PARAMS((_LONG_DOUBLE *));
#endif

static int exponent _PARAMS((char *, int, int));

#else /* no FLOATING_POINT */

#define	BUF		40

#endif /* FLOATING_POINT */

#ifndef _NO_LONGLONG
#define quad_t long long
#define u_quad_t unsigned long long
#else
#define quad_t long
#define u_quad_t unsigned long
#endif

typedef quad_t * quad_ptr_t;
typedef void *   void_ptr_t;
typedef char *   char_ptr_t;
typedef long *   long_ptr_t;
typedef int  *   int_ptr_t;
typedef short *  short_ptr_t;

#ifndef _NO_POS_ARGS
#define MAX_POS_ARGS 32

union arg_val
{
  int val_int;
  u_int val_u_int;
  long val_long;
  u_long val_u_long;
  float val_float;
  double val_double;
  _LONG_DOUBLE val__LONG_DOUBLE;
  int_ptr_t val_int_ptr_t;
  short_ptr_t val_short_ptr_t;
  long_ptr_t val_long_ptr_t;
  char_ptr_t val_char_ptr_t;
  quad_ptr_t val_quad_ptr_t;
  void_ptr_t val_void_ptr_t;
  quad_t val_quad_t;
  u_quad_t val_u_quad_t;
  wint_t val_wint_t;
};

static union arg_val *get_arg (struct _reent *data, int n, char *fmt, 
                               va_list *ap, int *numargs, union arg_val *args, 
			       int *arg_type, char **last_fmt);
#endif /* !_NO_POS_ARGS */

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)
#define	to_char(n)	((n) + '0')

/*
 * Flags used during conversion.
 */
#define	ALT		0x001		/* alternate form */
#define	HEXPREFIX	0x002		/* add 0x or 0X prefix */
#define	LADJUST		0x004		/* left adjustment */
#define	LONGDBL		0x008		/* long double */
#define	LONGINT		0x010		/* long integer */
#ifndef _NO_LONGLONG
#define	QUADINT		0x020		/* quad integer */
#else /* ifdef _NO_LONGLONG, make QUADINT equivalent to LONGINT, so
	 that %lld behaves the same as %ld, not as %d, as expected if:
	 sizeof (long long) = sizeof long > sizeof int  */
#define	QUADINT		LONGINT
#endif
#define	SHORTINT	0x040		/* short integer */
#define	ZEROPAD		0x080		/* zero (as opposed to blank) pad */
#define FPT		0x100		/* Floating point number */


		int _EXFUN (_VFPRINTF_R, (struct _reent *, FILE *, _CONST char *, va_list));
		
		int 
		_DEFUN (VFPRINTF, (fp, fmt0, ap),
			FILE * fp _AND
			_CONST char *fmt0 _AND
			va_list ap)
		{
		  int result;
		  _flockfile(fp);
#ifndef _SMALL_PRINTF		  
		  CHECK_INIT (fp);
#endif
		  result = _VFPRINTF_R (_REENT, fp, fmt0, ap);
		  _funlockfile(fp);
		  return result;
		}



int 
_DEFUN (_VFPRINTF_R, (data, fp, fmt0, ap),
	struct _reent *data _AND
	FILE * fp _AND
	_CONST char *fmt0 _AND
	va_list ap)
{
	register char *fmt;	/* format string */
	register int ch;	/* character from fmt */
	register int n, m;	/* handy integers (short term usage) */
	register char *cp;	/* handy char pointer (short term usage) */
	register struct __siov *iovp;/* for PRINT macro */
	register int flags;	/* flags as above */
	char *fmt_anchor;       /* current format spec being processed */
	int N;                  /* arg number */
	int arg_index;          /* index into args processed directly */
#ifndef _NO_POS_ARGS
	int numargs;            /* number of varargs read */
	char *saved_fmt;        /* saved fmt pointer */
	union arg_val args[MAX_POS_ARGS];
	int arg_type[MAX_POS_ARGS];
	int is_pos_arg;         /* is current format positional? */
	int old_is_pos_arg;     /* is current format positional? */
#endif
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	char sign;		/* sign prefix (' ', '+', '-', or \0) */
#ifdef FLOATING_POINT
#ifdef _SMALL_PRINTF
		char *decimal_point = ".";
#else
		char *decimal_point = localeconv()->decimal_point;
#endif
	char softsign;		/* temporary negative sign for floats */
#ifdef _NO_LONGDBL
	union { int i; double d; } _double_ = {0};
	#define _fpvalue (_double_.d)
#else
	union { int i; _LONG_DOUBLE ld; } _long_double_ = {0};
	#define _fpvalue (_long_double_.ld)
	int tmp;  
#endif
	int expt;		/* integer value of exponent */
	int expsize = 0;	/* character count for expstr */
	int ndig;		/* actual number of digits returned by cvt */
	char expstr[7];		/* buffer for exponent string */
#endif
	u_quad_t _uquad;	/* integer arguments %[diouxX] */
	enum { OCT, DEC, HEX } base;/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int realsz;		/* field size expanded by dprec */
	int size;		/* size of converted field or string */
	char *xdigs = NULL;	/* digits for [xX] conversion */
#ifndef _SMALL_PRINTF
		#define NIOV 8
			struct __suio uio;	/* output information: summary */
			struct __siov iov[NIOV];/* ... and individual io vectors */
			char *malloc_buf = NULL;/* handy pointer for malloced buffers */
#else
	char malloc_buf [MAXBUFLOC]; /* local buffers */
#endif

	char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
	char ox[2];		/* space for 0x hex-prefix */
#ifdef MB_CAPABLE
	wchar_t wc;
	mbstate_t state;        /* mbtowc calls from library must not change state */
#endif


	/*
	 * Choose PADSIZE to trade efficiency vs. size.  If larger printf
	 * fields occur frequently, increase PADSIZE and make the initialisers
	 * below longer.
	 */
#define	PADSIZE	16		/* pad chunk size */
	static _CONST char blanks[PADSIZE] =
	 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	static _CONST char zeroes[PADSIZE] =
	 {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

#ifdef MB_CAPABLE
        memset (&state, '\0', sizeof (state));
#endif

#ifndef  _SMALL_PRINTF
			/*
			 * BEWARE, these `goto error' on error, and PAD uses `n'.
			 */
		#define	PRINT(ptr, len,f) { \
			iovp->iov_base = (ptr); \
			iovp->iov_len = (len); \
			uio.uio_resid += (len); \
			iovp++; \
			if (++uio.uio_iovcnt >= NIOV) { \
				if (__sprint(fp, &uio)) \
					goto error; \
				iovp = iov; \
			} \
		}
		#define	PAD(howmany, with,f) { \
			if ((n = (howmany)) > 0) { \
				while (n > PADSIZE) { \
					PRINT(with, PADSIZE,f); \
					n -= PADSIZE; \
				} \
				PRINT(with, n,f); \
			} \
		}
		#define	FLUSH() { \
			if (uio.uio_resid && __sprint(fp, &uio)) \
				goto error; \
			uio.uio_iovcnt = 0; \
			iovp = iov; \
		}
#else
	//Macros for _SMALL_PRINTF
   	void _SMALL_PRINTF_puts(const char *ptr, int len, FILE *f);
		#define	PRINT(ptr, len, f) {_SMALL_PRINTF_puts(ptr,len,f);}
		#define	PAD(howmany, with, f) { \
			if ((n = (howmany)) > 0) { \
				while (n > PADSIZE) { \
					PRINT(with, PADSIZE,f); \
					n -= PADSIZE; \
				} \
				PRINT(with, n, f); \
			} \
		}
		#define	FLUSH() { ; }
#endif


	/* Macros to support positional arguments */
#ifndef _NO_POS_ARGS
#define GET_ARG(n, ap, type) \
  ( is_pos_arg \
      ? n < numargs \
         ? args[n].val_##type \
         : get_arg (data, n, fmt_anchor, &ap, &numargs, args, arg_type, &saved_fmt)->val_##type \
      : arg_index++ < numargs \
         ? args[n].val_##type \
         : numargs < MAX_POS_ARGS \
           ? args[numargs++].val_##type = va_arg(ap, type) \
           : va_arg(ap, type) \
  )
#else
#define GET_ARG(n, ap, type) (va_arg(ap, type))
#endif
    
	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#ifndef _NO_LONGLONG
#define	SARG() \
	(flags&QUADINT ? GET_ARG(N, ap, quad_t) : \
	    flags&LONGINT ? GET_ARG(N, ap, long) : \
	    flags&SHORTINT ? (long)(short)GET_ARG(N, ap, int) : \
	    (long)GET_ARG(N, ap, int))
#define	UARG() \
	(flags&QUADINT ? GET_ARG(N, ap, u_quad_t) : \
	    flags&LONGINT ? GET_ARG(N, ap, u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GET_ARG(N, ap, int) : \
	    (u_long)GET_ARG(N, ap, u_int))
#else
#define	SARG() \
	(flags&LONGINT ? GET_ARG(N, ap, long) : \
	    flags&SHORTINT ? (long)(short)GET_ARG(N, ap, int) : \
	    (long)GET_ARG(N, ap, int))
#define	UARG() \
	(flags&LONGINT ? GET_ARG(N, ap, u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GET_ARG(N, ap, int) : \
	    (u_long)GET_ARG(N, ap, u_int))
#endif

#ifndef _SMALL_PRINTF
			/* sorry, fprintf(read_only_file, "") returns EOF, not 0 */
			if (cantwrite(fp))
				return (EOF);
	
			/* optimise fprintf(stderr) (and other unbuffered Unix files) */
			if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
			    fp->_file >= 0)
				return (__sbprintf(fp, fmt0, ap));
		
			uio.uio_iov = iovp = iov;
			uio.uio_resid = 0;
			uio.uio_iovcnt = 0;
#endif
	fmt = (char *)fmt0;

	ret = 0;
	arg_index = 0;

#ifndef _NO_POS_ARGS
	saved_fmt = NULL;
	arg_type[0] = -1;
	numargs = 0;
	is_pos_arg = 0;
#endif

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
	        cp = fmt;
#ifdef MB_CAPABLE
	        while ((n = _mbtowc_r(data, &wc, fmt, MB_CUR_MAX, &state)) > 0) {
                    if (wc == '%')
                        break;
                    fmt += n;
		}
#else
                while (*fmt != '\0' && *fmt != '%')
                    fmt += 1;
#endif
		if ((m = fmt - cp) != 0) {
			PRINT(cp, m, fp);
			ret += m;
		}
#ifdef MB_CAPABLE
		if (n <= 0)
                    goto done;
#else
                if (*fmt == '\0')
                    goto done;
#endif
		fmt_anchor = fmt;
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		sign = '\0';
		N = arg_index;
#ifndef _NO_POS_ARGS
		is_pos_arg = 0;
#endif

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
		case ' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
		case '*':
			n = N;
#ifndef _NO_POS_ARGS
			/* we must check for positional arg used for dynamic width */
			old_is_pos_arg = is_pos_arg;
			is_pos_arg = 0;
			if (is_digit(*fmt)) {
				char *old_fmt = fmt;

				n = 0;
				ch = *fmt++;
				do {
					n = 10 * n + to_digit(ch);
					ch = *fmt++;
				} while (is_digit(ch));

				if (ch == '$') {
					if (n <= MAX_POS_ARGS) {
						n -= 1;
						is_pos_arg = 1;
					}
					else
						goto error;
				}
				else {
					fmt = old_fmt;
					goto rflag;
				}
			}
#endif /* !_NO_POS_ARGS */

			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			width = GET_ARG(n, ap, int);
#ifndef _NO_POS_ARGS
			is_pos_arg = old_is_pos_arg;
#endif
			if (width >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '.':
			if ((ch = *fmt++) == '*') {
				n = N;
#ifndef _NO_POS_ARGS
				/* we must check for positional arg used for dynamic width */
				old_is_pos_arg = is_pos_arg;
				is_pos_arg = 0;
				if (is_digit(*fmt)) {
					char *old_fmt = fmt;

					n = 0;
					ch = *fmt++;
					do {
						n = 10 * n + to_digit(ch);
						ch = *fmt++;
					} while (is_digit(ch));

					if (ch == '$') {
						if (n <= MAX_POS_ARGS) {
							n -= 1;
							is_pos_arg = 1;
						}
						else
							goto error;
					}
					else {
						fmt = old_fmt;
						goto rflag;
					}
				}
#endif /* !_NO_POS_ARGS */
				prec = GET_ARG(n, ap, int);
#ifndef _NO_POS_ARGS
				is_pos_arg = old_is_pos_arg;
#endif
				if (prec < 0)
					prec = -1;
				goto rflag;
			}
			n = 0;
			while (is_digit(ch)) {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case '0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			} while (is_digit(ch));
#ifndef _NO_POS_ARGS
			if (ch == '$') {
				if (n <= MAX_POS_ARGS) {
					N = n - 1;
					is_pos_arg = 1;
					goto rflag;
				}
				else
					goto error;
			}
#endif /* !_NO_POS_ARGS */
			width = n;
			goto reswitch;
#ifdef FLOATING_POINT
		case 'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case 'h':
			flags |= SHORTINT;
			goto rflag;
		case 'l':
			if (*fmt == 'l') {
				fmt++;
				flags |= QUADINT;
			} else {
				flags |= LONGINT;
			}
			goto rflag;
		case 'q':
			flags |= QUADINT;
			goto rflag;
		case 'c':
		case 'C':
			cp = buf;
			if (ch == 'C' || (flags & LONGINT)) {
				mbstate_t ps;

				memset((void *)&ps, '\0', sizeof(mbstate_t));
				if ((size = (int)_wcrtomb_r(data, cp, 
				    	       (wchar_t)GET_ARG(N, ap, wint_t), 
					        &ps)) == -1)
					goto error; 
			}
			else {
				*cp = GET_ARG(N, ap, int);
				size = 1;
			}
			sign = '\0';
			break;
		case 'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
			_uquad = SARG();
#ifndef _NO_LONGLONG
			if ((quad_t)_uquad < 0)
#else
			if ((long) _uquad < 0)
#endif
			{

				_uquad = -_uquad;
				sign = '-';
			}
			base = DEC;
			goto number;
#ifdef FLOATING_POINT
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			if (prec == -1) {
				prec = DEFPREC;
			} else if ((ch == 'g' || ch == 'G') && prec == 0) {
				prec = 1;
			}

#ifdef _NO_LONGDBL
			if (flags & LONGDBL) {
				_fpvalue = (double) GET_ARG(N, ap, _LONG_DOUBLE);
			} else {
				_fpvalue = GET_ARG(N, ap, double);
			}

			/* do this before tricky precision changes */
			if (isinf(_fpvalue)) {
				if (_fpvalue < 0)
					sign = '-';
				cp = "Inf";
				size = 3;
				break;
			}
			if (isnan(_fpvalue)) {
				cp = "NaN";
				size = 3;
				break;
			}

#else /* !_NO_LONGDBL */
			
			if (flags & LONGDBL) {
				_fpvalue = GET_ARG(N, ap, _LONG_DOUBLE);
			} else {
				_fpvalue = (_LONG_DOUBLE)GET_ARG(N, ap, double);
			}

			/* do this before tricky precision changes */
			tmp = _ldcheck (&_fpvalue);
			if (tmp == 2) {
				if (_fpvalue < 0)
					sign = '-';
				cp = "Inf";
				size = 3;
				break;
			}
			if (tmp == 1) {
				cp = "NaN";
				size = 3;
				break;
			}
#endif /* !_NO_LONGDBL */

			flags |= FPT;

			cp = cvt(data, _fpvalue, prec, flags, &softsign,
				&expt, ch, &ndig);

			if (ch == 'g' || ch == 'G') {
				if (expt <= -4 || expt > prec)
					ch = (ch == 'g') ? 'e' : 'E';
				else
					ch = 'g';
			} 
			if (ch <= 'e') {	/* 'e' or 'E' fmt */
				--expt;
				expsize = exponent(expstr, expt, ch);
				size = expsize + ndig;
				if (ndig > 1 || flags & ALT)
					++size;
			} else if (ch == 'f') {		/* f fmt */
				if (expt > 0) {
					size = expt;
					if (prec || flags & ALT)
						size += prec + 1;
				} else	/* "0.X" */
					size = (prec || flags & ALT)
						  ? prec + 2
						  : 1;
			} else if (expt >= ndig) {	/* fixed g fmt */
				size = expt;
				if (flags & ALT)
					++size;
			} else
				size = ndig + (expt > 0 ?
					1 : 2 - expt);

			if (softsign)
				sign = '-';
			break;
#endif /* FLOATING_POINT */
		case 'n':
#ifndef _NO_LONGLONG
			if (flags & QUADINT)
				*GET_ARG(N, ap, quad_ptr_t) = ret;
			else 
#endif
			if (flags & LONGINT)
				*GET_ARG(N, ap, long_ptr_t) = ret;
			else if (flags & SHORTINT)
				*GET_ARG(N, ap, short_ptr_t) = ret;
			else
				*GET_ARG(N, ap, int_ptr_t) = ret;
			continue;	/* no output */
		case 'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
			_uquad = UARG();
			base = OCT;
			goto nosign;
		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			/* NOSTRICT */
			_uquad = (u_long)(unsigned _POINTER_INT)GET_ARG(N, ap, void_ptr_t);
			base = HEX;
			xdigs = "0123456789abcdef";
			flags |= HEXPREFIX;
			ch = 'x';
			goto nosign;
		case 's':
		case 'S':
			sign = '\0';
			if ((cp = GET_ARG(N, ap, char_ptr_t)) == NULL) {
				cp = "(null)";
				size = 6;
			}
			else if (ch == 'S' || (flags & LONGINT)) {
				mbstate_t ps;
				_CONST wchar_t *wcp;
 
				wcp = (_CONST wchar_t *)cp;
				size = m = 0;
				memset((void *)&ps, '\0', sizeof(mbstate_t));
 
				/* Count number of bytes needed for multibyte
				   string that will be produced from widechar
				   string.  */
  				if (prec >= 0) {
					while (1) {
						if (wcp[m] == L'\0')
							break;
						if ((n = (int)_wcrtomb_r(data, 
                                                     buf, wcp[m], &ps)) == -1)
							goto error;
						if (n + size > prec)
							break;
						m += 1;
						size += n;
						if (size == prec)
							break;
					}
				}
				else {
					if ((size = (int)_wcsrtombs_r(data, 
                                                   NULL, &wcp, 0, &ps)) == -1)
						goto error; 
					wcp = (_CONST wchar_t *)cp;
				}
 
				if (size == 0)
					break;
 
				#ifndef _SMALL_PRINTF
				if ((malloc_buf = 
				    (char *)_malloc_r(data, size + 1)) == NULL)
					goto error;
				#endif
                             
				/* Convert widechar string to multibyte string. */
				memset((void *)&ps, '\0', sizeof(mbstate_t));
				if (_wcsrtombs_r(data, malloc_buf, &wcp, size, &ps) != size)
					goto error;
				cp = malloc_buf;
				cp[size] = '\0';
			}
			else if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
				char *p = memchr(cp, 0, prec);

				if (p != NULL) {
					size = p - cp;
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = strlen(cp);

			break;
		case 'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
			_uquad = UARG();
			base = DEC;
			goto nosign;
		case 'X':
			xdigs = "0123456789ABCDEF";
			goto hex;
		case 'x':
			xdigs = "0123456789abcdef";
hex:			_uquad = UARG();
			base = HEX;
			/* leading 0x/X only if non-zero */
			if (flags & ALT && _uquad != 0)
				flags |= HEXPREFIX;

			/* unsigned conversions */
nosign:			sign = '\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
			if (_uquad != 0 || prec != 0) {
				/*
				 * Unsigned mod is hard, and unsigned mod
				 * by a constant is easier than that by
				 * a variable; hence this switch.
				 */
				switch (base) {
				case OCT:
					do {
						*--cp = to_char(_uquad & 7);
						_uquad >>= 3;
					} while (_uquad);
					/* handle octal leading 0 */
					if (flags & ALT && *cp != '0')
						*--cp = '0';
					break;

				case DEC:
					/* many numbers are 1 digit */
					while (_uquad >= 10) {
						*--cp = to_char(_uquad % 10);
						_uquad /= 10;
					}
					*--cp = to_char(_uquad);
					break;

				case HEX:
					do {
						*--cp = xdigs[_uquad & 15];
						_uquad >>= 4;
					} while (_uquad);
					break;

				default:
					cp = "bug in vfprintf: bad base";
					size = strlen(cp);
					goto skipsize;
				}
			}
                       /*
			* ...result is to be converted to an 'alternate form'.
			* For o conversion, it increases the precision to force
			* the first digit of the result to be a zero."
			*     -- ANSI X3J11
			*
			* To demonstrate this case, compile and run:
                        *    printf ("%#.0o",0);
			*/
                       else if (base == OCT && (flags & ALT))
                         *--cp = '0';

			size = buf + BUF - cp;
		skipsize:
			break;
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == '\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = '\0';
			break;
		}

		/*
		 * All reasonable formats wind up here.  At this point, `cp'
		 * points to a string which (if not flags&LADJUST) should be
		 * padded out to `width' places.  If flags&ZEROPAD, it should
		 * first be prefixed by any sign or other prefix; otherwise,
		 * it should be blank padded before the prefix is emitted.
		 * After any left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print the
		 * string proper, then emit zeroes required by any leftover
		 * floating precision; finally, if LADJUST, pad with blanks.
		 *
		 * Compute actual size, so we know how much to pad.
		 * size excludes decimal prec; realsz includes it.
		 */
		realsz = dprec > size ? dprec : size;
		if (sign)
			realsz++;
		else if (flags & HEXPREFIX)
			realsz+= 2;

		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			PAD(width - realsz, blanks, fp);

		/* prefix */
		if (sign) {
			PRINT(&sign, 1, fp);
		} else if (flags & HEXPREFIX) {
			ox[0] = '0';
			ox[1] = ch;
			PRINT(ox, 2 ,fp);
		}

		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			PAD(width - realsz, zeroes, fp);

		/* leading zeroes from decimal precision */
		PAD(dprec - size, zeroes, fp);

		/* the string or number proper */
#ifdef FLOATING_POINT
		if ((flags & FPT) == 0) {
			PRINT(cp, size, fp);
		} else {	/* glue together f_p fragments */
			if (ch >= 'f') {	/* 'f' or 'g' */
				if (_fpvalue == 0) {
					/* kludge for __dtoa irregularity */
					PRINT("0", 1, fp);
					if (expt < ndig || (flags & ALT) != 0) {
						PRINT(decimal_point, 1, fp);
						PAD(ndig - 1, zeroes, fp);
					}
				} else if (expt <= 0) {
					PRINT("0", 1, fp);
					if(expt || ndig) {
						PRINT(decimal_point, 1, fp);
						PAD(-expt, zeroes, fp);
						PRINT(cp, ndig, fp);
					}
				} else if (expt >= ndig) {
					PRINT(cp, ndig, fp);
					PAD(expt - ndig, zeroes, fp);
					if (flags & ALT)
						PRINT(".", 1, fp);
				} else {
					PRINT(cp, expt, fp);
					cp += expt;
					PRINT(".", 1, fp);
					PRINT(cp, ndig-expt, fp);
				}
			} else {	/* 'e' or 'E' */
				if (ndig > 1 || flags & ALT) {
					ox[0] = *cp++;
					ox[1] = '.';
					PRINT(ox, 2, fp);
                                       if (_fpvalue) {
						PRINT(cp, ndig-1, fp);
					} else	/* 0.[0..] */
						/* __dtoa irregularity */
						PAD(ndig - 1, zeroes, fp);
				} else	/* XeYYY */
					PRINT(cp, 1, fp);
				PRINT(expstr, expsize, fp);
			}
		}
#else
		PRINT(cp, size, fp);
#endif
		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD(width - realsz, blanks, fp);

		/* finally, adjust ret */
		ret += width > realsz ? width : realsz;

		FLUSH();	/* copy out the I/O vectors */

#ifndef _SMALL_PRINTF
                if (malloc_buf != NULL) {
			free(malloc_buf);
			malloc_buf = NULL;
		}              
#endif
	}
done:
	FLUSH();
error:

#ifndef _SMALL_PRINTF
	if (malloc_buf != NULL)
		free(malloc_buf);
	return (__sferror(fp) ? EOF : ret);
#else
	return ret;
#endif
	/* NOTREACHED */
}

#ifdef FLOATING_POINT

#ifdef _NO_LONGDBL
extern char *_dtoa_r _PARAMS((struct _reent *, double, int,
			      int, int *, int *, char **));
#else
extern char *_ldtoa_r _PARAMS((struct _reent *, _LONG_DOUBLE, int,
			      int, int *, int *, char **));
#undef word0
#define word0(x) ldword0(x)
#endif

static char *
cvt(data, value, ndigits, flags, sign, decpt, ch, length)
	struct _reent *data;
#ifdef _NO_LONGDBL
	double value;
#else
	_LONG_DOUBLE value;
#endif
	int ndigits, flags, *decpt, ch, *length;
	char *sign;
{
	int mode, dsgn;
	char *digits, *bp, *rve;
#ifdef _NO_LONGDBL
        union double_union tmp;
#else
        struct ldieee *ldptr;
#endif

	if (ch == 'f') {
		mode = 3;		/* ndigits after the decimal point */
	} else {
		/* To obtain ndigits after the decimal point for the 'e' 
		 * and 'E' formats, round to ndigits + 1 significant 
		 * figures.
		 */
		if (ch == 'e' || ch == 'E') {
			ndigits++;
		}
		mode = 2;		/* ndigits significant digits */
	}

#ifdef _NO_LONGDBL
        tmp.d = value;

	if (word0(tmp) & Sign_bit) { /* this will check for < 0 and -0.0 */
		value = -value;
		*sign = '-';
        } else
		*sign = '\000';

	digits = _dtoa_r(data, value, mode, ndigits, decpt, &dsgn, &rve);
#else /* !_NO_LONGDBL */
	ldptr = (struct ldieee *)&value;
	if (ldptr->sign) { /* this will check for < 0 and -0.0 */
		value = -value;
		*sign = '-';
        } else
		*sign = '\000';

	digits = _ldtoa_r(data, value, mode, ndigits, decpt, &dsgn, &rve);
#endif /* !_NO_LONGDBL */

	if ((ch != 'g' && ch != 'G') || flags & ALT) {	/* Print trailing zeros */
		bp = digits + ndigits;
		if (ch == 'f') {
			if (*digits == '0' && value)
				*decpt = -ndigits + 1;
			bp += *decpt;
		}
		if (value == 0)	/* kludge for __dtoa irregularity */
			rve = bp;
		while (rve < bp)
			*rve++ = '0';
	}
	*length = rve - digits;
	return (digits);
}

static int
exponent(p0, exp, fmtch)
	char *p0;
	int exp, fmtch;
{
	register char *p, *t;
	char expbuf[40];

	p = p0;
	*p++ = fmtch;
	if (exp < 0) {
		exp = -exp;
		*p++ = '-';
	}
	else
		*p++ = '+';
	t = expbuf + 40;
	if (exp > 9) {
		do {
			*--t = to_char(exp % 10);
		} while ((exp /= 10) > 9);
		*--t = to_char(exp);
		for (; t < expbuf + 40; *p++ = *t++);
	}
	else {
		*p++ = '0';
		*p++ = to_char(exp);
	}
	return (p - p0);
}
#endif /* FLOATING_POINT */


#ifndef _NO_POS_ARGS

/* Positional argument support.
   Written by Jeff Johnston

   Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
      
      The name of Red Hat Incorporated may not be used to endorse
      or promote products derived from this software without specific
      prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

typedef enum {
  ZERO,   /* '0' */
  DIGIT,  /* '1-9' */
  DOLLAR, /* '$' */
  MODFR,  /* spec modifier */
  SPEC,   /* format specifier */
  DOT,    /* '.' */
  STAR,   /* '*' */
  FLAG,   /* format flag */
  OTHER,  /* all other chars */ 
  MAX_CH_CLASS /* place-holder */
} CH_CLASS;

typedef enum { 
  START,  /* start */
  SFLAG,  /* seen a flag */
  WDIG,   /* seen digits in width area */
  WIDTH,  /* processed width */
  SMOD,   /* seen spec modifier */
  SDOT,   /* seen dot */ 
  VARW,   /* have variable width specifier */
  VARP,   /* have variable precision specifier */
  PREC,   /* processed precision */
  VWDIG,  /* have digits in variable width specification */
  VPDIG,  /* have digits in variable precision specification */
  DONE,   /* done */   
  MAX_STATE, /* place-holder */ 
} STATE;

typedef enum {
  NOOP,  /* do nothing */
  NUMBER, /* build a number from digits */
  SKIPNUM, /* skip over digits */
  GETMOD,  /* get and process format modifier */
  GETARG,  /* get and process argument */
  GETPW,   /* get variable precision or width */
  GETPWB,  /* get variable precision or width and pushback fmt char */
  GETPOS,  /* get positional parameter value */
  PWPOS,   /* get positional parameter value for variable width or precision */
} ACTION;

const static CH_CLASS chclass[256] = {
  /* 00-07 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 08-0f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 10-17 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 18-1f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 20-27 */  FLAG,    OTHER,   OTHER,   FLAG,    DOLLAR,  OTHER,   OTHER,   OTHER,
  /* 28-2f */  OTHER,   OTHER,   STAR,    FLAG,    OTHER,   FLAG,    DOT,     OTHER,
  /* 30-37 */  ZERO,    DIGIT,   DIGIT,   DIGIT,   DIGIT,   DIGIT,   DIGIT,   DIGIT,
  /* 38-3f */  DIGIT,   DIGIT,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 40-47 */  OTHER,   OTHER,   OTHER,   SPEC,    SPEC,    SPEC,    OTHER,   SPEC, 
  /* 48-4f */  OTHER,   OTHER,   OTHER,   OTHER,   MODFR,   OTHER,   OTHER,   SPEC, 
  /* 50-57 */  OTHER,   OTHER,   OTHER,   SPEC,    OTHER,   SPEC,    OTHER,   SPEC, 
  /* 58-5f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 60-67 */  OTHER,   OTHER,   OTHER,   SPEC,    SPEC,    SPEC,    SPEC,    SPEC, 
  /* 68-6f */  MODFR,   SPEC,    OTHER,   OTHER,   MODFR,   OTHER,   OTHER,   SPEC, 
  /* 70-77 */  SPEC,    MODFR,   OTHER,   SPEC,    OTHER,   SPEC,    OTHER,   OTHER,
  /* 78-7f */  SPEC,    OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 80-87 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 88-8f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 90-97 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 98-9f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* a0-a7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* a8-af */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* b0-b7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* b8-bf */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* c0-c7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* c8-cf */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* d0-d7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* d8-df */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* e0-e7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* e8-ef */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* f0-f7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* f8-ff */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
};

const static STATE state_table[MAX_STATE][MAX_CH_CLASS] = {
  /*             '0'     '1-9'     '$'     MODFR    SPEC    '.'     '*'    FLAG    OTHER */ 
  /* START */  { SFLAG,   WDIG,    DONE,   SMOD,    DONE,   SDOT,  VARW,   SFLAG,  DONE },
  /* SFLAG */  { SFLAG,   WDIG,    DONE,   SMOD,    DONE,   SDOT,  VARW,   SFLAG,  DONE },
  /* WDIG  */  { DONE,    DONE,    WIDTH,  SMOD,    DONE,   SDOT,  DONE,   DONE,   DONE },
  /* WIDTH */  { DONE,    DONE,    DONE,   SMOD,    DONE,   SDOT,  DONE,   DONE,   DONE },
  /* SMOD  */  { DONE,    DONE,    DONE,   DONE,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* SDOT  */  { SDOT,    PREC,    DONE,   SMOD,    DONE,   DONE,  VARP,   DONE,   DONE },
  /* VARW  */  { DONE,    VWDIG,   DONE,   SMOD,    DONE,   SDOT,  DONE,   DONE,   DONE },
  /* VARP  */  { DONE,    VPDIG,   DONE,   SMOD,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* PREC  */  { DONE,    DONE,    DONE,   SMOD,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* VWDIG */  { DONE,    DONE,    WIDTH,  DONE,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* VPDIG */  { DONE,    DONE,    PREC,   DONE,    DONE,   DONE,  DONE,   DONE,   DONE },
};

const static ACTION action_table[MAX_STATE][MAX_CH_CLASS] = {
  /*             '0'     '1-9'     '$'     MODFR    SPEC    '.'     '*'    FLAG    OTHER */ 
  /* START */  { NOOP,    NUMBER,  NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* SFLAG */  { NOOP,    NUMBER,  NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* WDIG  */  { NOOP,    NOOP,    GETPOS, GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* WIDTH */  { NOOP,    NOOP,    NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* SMOD  */  { NOOP,    NOOP,    NOOP,   NOOP,    GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* SDOT  */  { NOOP,    SKIPNUM, NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* VARW  */  { NOOP,    NUMBER,  NOOP,   GETPW,   GETPWB, GETPW, NOOP,   NOOP,   NOOP },
  /* VARP  */  { NOOP,    NUMBER,  NOOP,   GETPW,   GETPWB, NOOP,  NOOP,   NOOP,   NOOP },
  /* PREC  */  { NOOP,    NOOP,    NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* VWDIG */  { NOOP,    NOOP,    PWPOS,  NOOP,    NOOP,   NOOP,  NOOP,   NOOP,   NOOP },
  /* VPDIG */  { NOOP,    NOOP,    PWPOS,  NOOP,    NOOP,   NOOP,  NOOP,   NOOP,   NOOP },
};

/* function to get positional parameter N where n = N - 1 */
static union arg_val *
get_arg (struct _reent *data, int n, char *fmt, va_list *ap, 
         int *numargs_p, union arg_val *args, 
	 int *arg_type, char **last_fmt) 
{
  int ch;
  int number, flags;
  int spec_type;
  int numargs = *numargs_p;
  CH_CLASS chtype;
  STATE state, next_state;
  ACTION action;
  int pos, last_arg;
  int max_pos_arg = n;
  enum types { INT, LONG_INT, SHORT_INT, QUAD_INT, CHAR, CHAR_PTR, DOUBLE, LONG_DOUBLE, WIDE_CHAR };
#ifdef MB_CAPABLE
  wchar_t wc;
  mbstate_t wc_state;
  int nbytes; 
#endif
    
  /* if this isn't the first call, pick up where we left off last time */
  if (*last_fmt != NULL)
    fmt = *last_fmt;

#ifdef MB_CAPABLE
  memset (&wc_state, '\0', sizeof (wc_state));
#endif

  /* we need to process either to end of fmt string or until we have actually
     read the desired parameter from the vararg list. */
  while (*fmt && n >= numargs)
    {
#ifdef MB_CAPABLE
      while ((nbytes = _mbtowc_r(data, &wc, fmt, MB_CUR_MAX, &wc_state)) > 0) 
	{
	  fmt += nbytes;
	  if (wc == '%') 
	    break;
	}

      if (nbytes <= 0)
	break;
#else
      while (*fmt != '\0' && *fmt != '%')
	fmt += 1;

      if (*fmt == '\0')
	break;
#endif
      state = START;
      flags = 0;
      pos = -1;
      number = 0;
      spec_type = INT;

      /* Use state/action table to process format specifiers.  We ignore invalid
         formats and we are only interested in information that tells us how to
         read the vararg list. */
      while (state != DONE)
	{
	  ch = *fmt++;
	  chtype = chclass[ch];
	  next_state = state_table[state][chtype];
	  action = action_table[state][chtype];
	  state = next_state;
	  
	  switch (action)
	    {
	    case GETMOD:  /* we have format modifier */
	      switch (ch)
		{
		case 'h':
		  flags |= SHORTINT;
		  break;
		case 'L':
		  flags |= LONGDBL;
		  break;
		case 'q':
		  flags |= QUADINT;
		  break;
		case 'l':
		default:
		  if (*fmt == 'l')
		    {
		      flags |= QUADINT;
		      ++fmt;
		    }
		  else
		    flags |= LONGINT;
		  break;
		}
	      break;
	    case GETARG: /* we have format specifier */
	      {
		numargs &= (MAX_POS_ARGS - 1);
		/* process the specifier and translate it to a type to fetch from varargs */
		switch (ch)
		  {
		  case 'd':
		  case 'i':
		  case 'o':
		  case 'x':
		  case 'X':
		  case 'u':
		    if (flags & LONGINT)
		      spec_type = LONG_INT;
		    else if (flags & SHORTINT)
		      spec_type = SHORT_INT;
#ifndef _NO_LONGLONG
		    else if (flags & QUADINT)
		      spec_type = QUAD_INT;
#endif
		    else
		      spec_type = INT;
		    break;
		  case 'D':
		  case 'U':
		  case 'O':
		    spec_type = LONG_INT;
		    break;
		  case 'f':
		  case 'g':
		  case 'G':
		  case 'E':
		  case 'e':
#ifndef _NO_LONGDBL
		    if (flags & LONGDBL)
		      spec_type = LONG_DOUBLE;
		    else
#endif
		      spec_type = DOUBLE;
		    break;
		  case 's':
		  case 'S':
		  case 'p':
		    spec_type = CHAR_PTR;
		    break;
		  case 'c':
		    spec_type = CHAR;
		    break;
		  case 'C':
		    spec_type = WIDE_CHAR;
		    break;
		  }

		/* if we have a positional parameter, just store the type, otherwise
		   fetch the parameter from the vararg list */
		if (pos != -1)
		  arg_type[pos] = spec_type;
		else
		  {
		    switch (spec_type)
		      {
		      case LONG_INT:
			args[numargs++].val_long = va_arg(*ap, long);
			break;
		      case QUAD_INT:
			args[numargs++].val_quad_t = va_arg(*ap, quad_t);
			break;
		      case WIDE_CHAR:
			args[numargs++].val_wint_t = va_arg(*ap, wint_t);
			break;
		      case CHAR:
		      case SHORT_INT:
		      case INT:
			args[numargs++].val_int = va_arg(*ap, int);
			break;
		      case CHAR_PTR:
			args[numargs++].val_char_ptr_t = va_arg(*ap, char *);
			break;
		      case DOUBLE:
			args[numargs++].val_double = va_arg(*ap, double);
			break;
		      case LONG_DOUBLE:
			args[numargs++].val__LONG_DOUBLE = va_arg(*ap, _LONG_DOUBLE);
			break;
		      }
		  }
	      }
	      break;
	    case GETPOS: /* we have positional specifier */
	      if (arg_type[0] == -1)
		memset (arg_type, 0, sizeof(int) * MAX_POS_ARGS);
	      pos = number - 1;
	      max_pos_arg = (max_pos_arg > pos ? max_pos_arg : pos);
	      break;
	    case PWPOS:  /* we have positional specifier for width or precision */
	      if (arg_type[0] == -1)
		memset (arg_type, 0, sizeof(int) * MAX_POS_ARGS);
	      number -= 1;
	      arg_type[number] = INT;
	      max_pos_arg = (max_pos_arg > number ? max_pos_arg : number);
	      break;
	    case GETPWB: /* we require format pushback */
	      --fmt;
	      /* fallthrough */
	    case GETPW:  /* we have a variable precision or width to acquire */
	      args[numargs++].val_int = va_arg(*ap, int);
	      break;
	    case NUMBER: /* we have a number to process */
	      number = (ch - '0');
	      while ((ch = *fmt) != '\0' && is_digit(ch))
		{
		  number = number * 10 + (ch - '0');
		  ++fmt;
		}
	      break;
	    case SKIPNUM: /* we have a number to skip */
	      while ((ch = *fmt) != '\0' && is_digit(ch))
		++fmt;
	      break;
	    case NOOP:
	    default:
	      break; /* do nothing */
	    }
	}
    }

  /* process all arguments up to at least the one we are looking for and if we
     have seen the end of the string, then process up to the max argument needed */
  if (*fmt == '\0')
    last_arg = max_pos_arg;
  else
    last_arg = n;

  while (numargs <= last_arg)
    {
      switch (arg_type[numargs])
	{
	case LONG_INT:
	  args[numargs++].val_long = va_arg(*ap, long);
	  break;
	case QUAD_INT:
	  args[numargs++].val_quad_t = va_arg(*ap, quad_t);
	  break;
	case CHAR_PTR:
	  args[numargs++].val_char_ptr_t = va_arg(*ap, char *);
	  break;
	case DOUBLE:
	  args[numargs++].val_double = va_arg(*ap, double);
	  break;
	case LONG_DOUBLE:
	  args[numargs++].val__LONG_DOUBLE = va_arg(*ap, _LONG_DOUBLE);
	  break;
	case WIDE_CHAR:
	  args[numargs++].val_wint_t = va_arg(*ap, wint_t);
	  break;
	case INT:
	case SHORT_INT:
	case CHAR:
	default:
	  args[numargs++].val_int = va_arg(*ap, int);
	  break;
	}
    }

  /*
   * alter the global numargs value and keep a reference to the last bit of the
   * fmt string we processed here because the caller will continue processing
   * where we started
   */
  *numargs_p = numargs;
  *last_fmt = fmt;
  return &args[n];
}
#endif /* !_NO_POS_ARGS */
/** @} */
