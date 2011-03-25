/*
FUNCTION
<<vscanf>>, <<vfscanf>>, <<vsscanf>>---format argument list

INDEX
	vscanf
INDEX
	vfscanf
INDEX
	vsscanf

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int vscanf(const char *<[fmt]>, va_list <[list]>);
	int vfscanf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsscanf(const char *<[str]>, const char *<[fmt]>, va_list <[list]>);

	int _vscanf_r(void *<[reent]>, const char *<[fmt]>, 
                       va_list <[list]>);
	int _vfscanf_r(void *<[reent]>, FILE *<[fp]>, const char *<[fmt]>, 
                       va_list <[list]>);
	int _vsscanf_r(void *<[reent]>, const char *<[str]>, const char *<[fmt]>, 
                       va_list <[list]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	#include <varargs.h>
	int vscanf( <[fmt]>, <[ist]>)
	char *<[fmt]>;
	va_list <[list]>;

	int vfscanf( <[fp]>, <[fmt]>, <[list]>)
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;
	
	int vsscanf( <[str]>, <[fmt]>, <[list]>)
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vscanf_r( <[reent]>, <[fmt]>, <[ist]>)
	char *<[reent]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vfscanf_r( <[reent]>, <[fp]>, <[fmt]>, <[list]>)
	char *<[reent]>;
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;
	
	int _vsscanf_r( <[reent]>, <[str]>, <[fmt]>, <[list]>)
	char *<[reent]>;
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

DESCRIPTION
<<vscanf>>, <<vfscanf>>, and <<vsscanf>> are (respectively) variants
of <<scanf>>, <<fscanf>>, and <<sscanf>>.  They differ only in 
allowing their caller to pass the variable argument list as a 
<<va_list>> object (initialized by <<va_start>>) rather than 
directly accepting a variable number of arguments.

RETURNS
The return values are consistent with the corresponding functions:
<<vscanf>> returns the number of input fields successfully scanned,
converted, and stored; the return value does not include scanned
fields which were not stored.  

If <<vscanf>> attempts to read at end-of-file, the return value 
is <<EOF>>.

If no fields were stored, the return value is <<0>>.

The routines <<_vscanf_r>>, <<_vfscanf_f>>, and <<_vsscanf_r>> are
reentrant versions which take an additional first parameter which points to the
reentrancy structure.

PORTABILITY
These are GNU extensions.

Supporting OS subroutines required:
*/

/*-
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
//#include <ctype.h>
#include <wctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <wchar.h>
#include <string.h>


#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef SMALL_SCANF
#include "local.h"
#endif


#ifndef	NO_FLOATING_POINT
#include <float.h>
#endif



#ifndef	NO_FLOATING_POINT
#define FLOATING_POINT
#endif

#ifdef FLOATING_POINT
#include <float.h>

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the _ldtoa_r code be
   preferred over _dtoa_r.  */
#define _NO_LONGDBL
#if defined WANT_IO_LONG_DBL && (LDBL_MANT_DIG > DBL_MANT_DIG)
#undef _NO_LONGDBL
extern _LONG_DOUBLE _strtold _PARAMS((char *s, char **sptr));
#endif

#define _NO_LONGLONG
#if defined WANT_PRINTF_LONG_LONG && defined __GNUC__
# undef _NO_LONGLONG
#endif

#include "floatio.h"

#if ((MAXEXP+MAXFRACT+3) > MB_LEN_MAX)
#  define BUF (MAXEXP+MAXFRACT+3)        /* 3 = sign + decimal point + NUL */
#else
#  define BUF MB_LEN_MAX
#endif

/* An upper bound for how long a long prints in decimal.  4 / 13 approximates
   log (2).  Add one char for roundoff compensation and one for the sign.  */
#define MAX_LONG_LEN ((CHAR_BIT * sizeof (long)  - 1) * 4 / 13 + 2)
#else
#define	BUF	40
#endif


/*
 * Flags used during conversion.
 */

#define	LONG		0x01	/* l: long or double */
#define	LONGDBL		0x02	/* L/ll: long double or long long */
#define	SHORT		0x04	/* h: short */
#define CHAR		0x08	/* hh: 8 bit integer */
#define	SUPPRESS	0x10	/* suppress assignment */
#define	POINTER		0x20	/* weird %p pointer (`fake hex') */
#define	NOSKIP		0x40	/* do not skip blanks */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */

#define	SIGNOK		0x80	/* +/- is (still) legal */
#define	NDIGITS		0x100	/* no digits detected */

#define	DPTOK		0x200	/* (float) decimal point is still legal */
#define	EXPOK		0x400	/* (float) exponent (e+3, etc) still legal */

#define	PFXOK		0x200	/* 0x prefix is (still) legal */
#define	NZDIGITS	0x400	/* no zero digits detected */

/*
 * Conversion types.
 */

#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* integer, i.e., strtol or strtoul */
#define	CT_FLOAT	4	/* floating, i.e., strtod */

#if 0
#define u_char unsigned char
#endif
#define u_char char
#define u_long unsigned long

#ifndef _NO_LONGLONG
typedef unsigned long long u_long_long;
#endif

/*static*/ u_char *__sccl ();

/*
 * vfscanf
 */

#ifndef SMALL_SCANF
#define BufferEmpty (fp->_r <= 0 && __srefill(fp))

#else // SMALL_SCANF

unsigned char buf_ungetc ;
int flag_buf_ungetc = 0  ;
#define buf_ungetc_empty 0
#define buf_ungetc_full  1


/*
	Redefinition of ungetc : store in a buffer a character 
	Only ONE ungetc is allowed before a getchar
   For each getchar, new_getchar() is called, then the buf is tested:
   	- if flag_buf_ungetc == 0 : new_getchar call __io_getchar() 
   	- if flag_buf_ungetc == 1 : new_getchar returns character in the buffer 
   	
 */
   



int __io_ungetc(int c){

if (flag_buf_ungetc == 0){ 
   flag_buf_ungetc = buf_ungetc_full; //flag to 1 to indicate that a caracter is in the buffer
}
buf_ungetc= (unsigned char)c ;

return (c);
}

int new_getchar() {
if (flag_buf_ungetc == 0){
  
  return __io_getchar();

}
else {
   
	flag_buf_ungetc = buf_ungetc_empty;		
	return buf_ungetc;

}

}

#endif // SMALL_SCANF

#ifndef _REENT_ONLY

int
_DEFUN (vfscanf, (fp, fmt, ap), 
    register FILE *fp _AND 
    _CONST char *fmt _AND 
    va_list ap)
{
  
  #ifndef SMALL_SCANF
  CHECK_INIT(fp);
  #endif
  
  return __svfscanf_r (_REENT, fp, fmt, ap);
}

int
__svfscanf (fp, fmt0, ap)
     register FILE *fp;
     char _CONST *fmt0;
     va_list ap;
{
  return __svfscanf_r (_REENT, fp, fmt0, ap);
}

#endif /* !_REENT_ONLY */

int
_DEFUN (_vfscanf_r, (data, fp, fmt, ap),
    struct _reent *data _AND 
    register FILE *fp _AND 
    _CONST char *fmt _AND 
    va_list ap)
{
  return __svfscanf_r (data, fp, fmt, ap);
}



/* 


For SMALL_SCANF :
The use of files has been removed so as to use directly __io_getchar()
Buffer_empty is not anymore tested since _io_getchar() is blocked until
a character is entered.
Generally each *fp->_p ++  which equal to read a character on the file 
has been replaced by a new_getchar() which call __io_getchar(); 


*/

int
__svfscanf_r (rptr, fp, fmt0, ap)
     struct _reent *rptr;
     register FILE *fp;
     char _CONST *fmt0;
     va_list ap;
{
  
  register char * bufread;
  register u_char *fmt = (u_char *) fmt0;
  register int c;		/* character from format, or conversion */
  register int car;		
  register size_t width;	/* field width, or 0 */
  register char *p;		/* points into all kinds of strings */
  register int n;		/* handy integer */
  register int flags;		/* flags as defined above */
  register char *p0;		/* saves original value of p when necessary */
  int nassigned;		/* number of fields assigned */
  int nread;			/* number of characters consumed from fp */
  int base = 0;			/* base argument to strtol/strtoul */
  int nbytes = 1;               /* number of bytes read from fmt string */
  wchar_t wc;                   /* wchar to use to read format string */
  wchar_t *wcp;                 /* handy wide character pointer */
  size_t mbslen;                /* length of converted multibyte sequence */
  mbstate_t state;              /* value to keep track of multibyte state */

  u_long (*ccfn) () = 0;	/* conversion function (strtol/strtoul) */
  char ccltab[256];		/* character class table for %[...] */
  char buf[BUF];		/* buffer for numeric conversions */
  char *lptr;                   /* literal pointer */

  char *cp;
  short *sp;
  int *ip;
  float *flp;
  _LONG_DOUBLE *ldp;
  double *dp;
  long *lp;
#ifndef _NO_LONGLONG
  long long *llp;
#endif

  /* `basefix' is used to avoid `if' tests in the integer scanner */
  static _CONST short basefix[17] =
    {10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  nassigned = 0;
  nread = 0;
  for (;;)
    {
#ifndef MB_CAPABLE
      wc = *fmt;
#else
      memset (&state, '\0', sizeof (state));
      nbytes = _mbtowc_r (rptr, &wc, fmt, MB_CUR_MAX, &state);
#endif
      fmt += nbytes;
      if (wc == 0)
	return nassigned;
      if (nbytes == 1 && isspace (wc))
	{
	  for (;;)
	    {
			#ifndef SMALL_SCANF
	      if (BufferEmpty || !isspace (*fp->_p))
		break;
	      nread++, fp->_r--, fp->_p++;
	     
	      #else            
	      if (!isspace (*fp->_p)) break;
	      nread++, fp->_r--;
	    	*fp->_p = new_getchar();  
	
	    	#endif
	    }
	    	
	  continue;
	}
      if (wc != '%')
	goto literal;
      width = 0;
      flags = 0;

      /*
       * switch on the format.  continue if done; break once format
       * type is derived.
       */

    again:
      c = *fmt++;
      switch (c)
	{
	case '%':
	literal:
          lptr = fmt - nbytes;
          for (n = 0; n < nbytes; ++n)
            {
         #ifndef SMALL_SCANF   
	      if (BufferEmpty)
	        goto input_failure;
	      #else
	        *fp->_p = new_getchar();
	      #endif
	      
	      if (*fp->_p != *lptr)
	        goto match_failure;
	       #ifndef SMALL_SCANF 
	      fp->_r--, fp->_p++;
	      nread++;
	      #else
	         fp->_r--;
	         *fp->_p = new_getchar();
	         nread++;
	      #endif
              ++lptr;
            }
	  continue;

	case '*':
	  flags |= SUPPRESS;
	  goto again;
	case 'l':
	  if (*fmt == 'l')	/* Check for 'll' = long long (SUSv3) */
	    {
	      ++fmt;
	      flags |= LONGDBL;
	    }
	  else
	    flags |= LONG;
	  goto again;
	case 'L':
	  flags |= LONGDBL;
	  goto again;
	case 'h':
	  if (*fmt == 'h')	/* Check for 'hh' = char int (SUSv3) */
	    {
	      ++fmt;
	      flags |= CHAR;
	    }
	  else
	    flags |= SHORT;
	  goto again;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  width = width * 10 + c - '0';
	  goto again;

	  /*
	   * Conversions. Those marked `compat' are for
	   * 4.[123]BSD compatibility.
	   *
	   * (According to ANSI, E and X formats are supposed to
	   * the same as e and x.  Sorry about that.)
	   */

	case 'D':		/* compat */
	  flags |= LONG;
	  /* FALLTHROUGH */
	case 'd':
	  c = CT_INT;
	  ccfn = (u_long (*)())_strtol_r;
	  base = 10;
	  break;

	case 'i':
	  c = CT_INT;
	  ccfn = (u_long (*)())_strtol_r;
	  base = 0;
	  break;

	case 'O':		/* compat */
	  flags |= LONG;
	  /* FALLTHROUGH */
	case 'o':
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 8;
	  break;

	case 'u':
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 10;
	  break;

	case 'X':		/* compat   XXX */
	case 'x':
	  flags |= PFXOK;	/* enable 0x prefixing */
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 16;
	  break;

#ifdef FLOATING_POINT
	case 'E':		/* compat   XXX */
	case 'G':		/* compat   XXX */
/* ANSI says that E,G and X behave the same way as e,g,x */
	  /* FALLTHROUGH */
	case 'e':
	case 'f':
	case 'g':
	  c = CT_FLOAT;
	  break;
#endif
        case 'S':
          flags |= LONG;
          /* FALLTHROUGH */

	case 's':
	  c = CT_STRING;
	  break;

	case '[':
	  fmt = __sccl (ccltab, fmt);
	  flags |= NOSKIP;
	  c = CT_CCL;
	  break;

        case 'C':
          flags |= LONG;
          /* FALLTHROUGH */

	case 'c':
	  flags |= NOSKIP;
	  c = CT_CHAR;
	  break;

	case 'p':		/* pointer format is like hex */
	  flags |= POINTER | PFXOK;
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 16;
	  break;

	case 'n':
	  if (flags & SUPPRESS)	/* ??? */
	    continue;
	  if (flags & CHAR)
	    {
	      cp = va_arg (ap, char *);
	      *cp = nread;
	    }
	  else if (flags & SHORT)
	    {
	      sp = va_arg (ap, short *);
	      *sp = nread;
	    }
	  else if (flags & LONG)
	    {
	      lp = va_arg (ap, long *);
	      *lp = nread;
	    }
#ifndef _NO_LONGLONG
	  else if (flags & LONGDBL)
	    {
	      llp = va_arg (ap, long long*);
	      *llp = nread;
	    }
#endif
	  else
	    {
	      ip = va_arg (ap, int *);
	      *ip = nread;
	    }
	  continue;

	  /*
	   * Disgusting backwards compatibility hacks.	XXX
	   */
	case '\0':		/* compat */
	  return EOF;

	default:		/* compat */
	  if (isupper (c))
	    flags |= LONG;
	  c = CT_INT;
	  ccfn = (u_long (*)())_strtol_r;
	  base = 10;
	  break;
	}

      /*
       * We have a conversion that requires input.
       */
   #ifndef SMALL_SCANF
      if (BufferEmpty)
   	goto input_failure;
   #else
      *fp->_p = new_getchar();      
   #endif

      /*
       * Consume leading white space, except for formats that
       * suppress this.
       */

      if ((flags & NOSKIP) == 0)
	{
	  while (isspace (*fp->_p))
	    {
	      #ifndef SMALL_SCANF 
	    	nread++;
	      if (--fp->_r > 0)
		     fp->_p++;
	      else
	      if (__srefill (fp))
		   goto input_failure;
		   #else
     			 *fp->_p = new_getchar();      
   		#endif
	    }
	  /*
	   * Note that there is at least one character in the
	   * buffer, so conversions that do not set NOSKIP ca
	   * no longer result in an input failure.
	   */
	}

      /*
       * Do the conversion.
       */
      switch (c)
	{

	case CT_CHAR:
	  /* scan arbitrary characters (sets NOSKIP) */
	  if (width == 0)
	    width = 1;
          if (flags & LONG) 
            {
              if ((flags & SUPPRESS) == 0)
                wcp = va_arg(ap, wchar_t *);
              else
                wcp = NULL;
              n = 0;
              while (width != 0) 
                {
                  
                  if (n == MB_CUR_MAX)
                    goto input_failure;
                  #ifndef SMALL_SCANF
                  buf[n++] = *fp->_p;
                  fp->_r -= 1;
                  fp->_p += 1;
                  memset((void *)&state, '\0', sizeof(mbstate_t));
                  if ((mbslen = _mbrtowc_r(rptr, wcp, buf, n, &state)) 
                                                         == (size_t)-1)
                    goto input_failure; /* Invalid sequence */
                  #else
                     buf[n++] = *fp->_p;
                  	*fp->_p = new_getchar();
							 						 
						#endif                 
                  
                  if (mbslen == 0 && !(flags & SUPPRESS))
                    *wcp = L'\0';
                  if (mbslen != (size_t)-2) /* Incomplete sequence */
                    {
                      nread += n;
                      width -= 1;
                      if (!(flags & SUPPRESS))
                        wcp += 1;
                      n = 0;
                    }
                  #ifndef SMALL_SCANF
                  if (BufferEmpty) 
	            {
                      if (n != 0) 
                        goto input_failure;
                      break;
                    }
                  #endif
                }
              if (!(flags & SUPPRESS))
                nassigned++;
            } 
          else if (flags & SUPPRESS) 
	    {
	      size_t sum = 0;
	      for (;;)
		{
		  if ((n = fp->_r) < (int)width)
		    {
		      sum += n;
		      width -= n;	
		      #ifndef SMALL_SCANF          
		      fp->_p += n;		      
		      if (__srefill (fp))
			{
			  if (sum == 0)
			    goto input_failure;
			  break;
			}
			  #else
			  *fp->_p = new_getchar();			  
			  #endif
			 
		    }
		  else
		    {
		      sum += width;
		      fp->_r -= width;
		      #ifndef SMALL_SCANF
		      fp->_p += width;
		      #else
		      *fp->_p = new_getchar();
		      #endif
		      
		      break;
		    }
		}
	      nread += sum;
	    }
	  else
	    {
		
			#ifndef SMALL_SCANF 
	      size_t r = fread ((_PTR) va_arg (ap, char *), 1, width, fp);
	      if (r == 0)
		   goto input_failure;
	      nread += r;
	      nassigned++;
	     	      
	      #else
	      bufread=(_PTR)va_arg(ap,char *);
	      int r;
	      for (r=0;r<width;r++){	      	
	      	*bufread++= *fp->_p;
	      	if ( r+1 < width){
	      	*fp->_p = new_getchar();
	      	}
	     	 }
	     	#endif          	  	  
	    }
	  break;

	case CT_CCL:
	  /* scan a (nonempty) character class (sets NOSKIP) */
	  if (width == 0)
	    width = ~0;		/* `infinity' */
	  /* take only those things in the class */
	  if (flags & SUPPRESS)
	    {
	      n = 0;
	      while (ccltab[*fp->_p])
		{
		  #ifndef SMALL_SCANF
		  n++, fp->_r--, fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    {
		      if (n == 0)
			goto input_failure;
		      break;
		    }
		   #else
		    n++;
		    fp->_r++;
		    *fp->_p = new_getchar();
		   #endif
		   
		}
	      if (n == 0)
		goto match_failure;
	    }
	  else
	    {
	      p0 = p = va_arg (ap, char *);
	      while (ccltab[*fp->_p])
		{
		  fp->_r--;
		  #ifndef SMALL_SCANF
		  *p++ = *fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    {
		      if (p == p0)
			goto input_failure;
		      break;
		    }
		  #else
		   *p++ = *fp->_p;
		  	*fp->_p= new_getchar();
		  	if (--width == 0)
		    break;
		  #endif
		  
		}
	      n = p - p0;
	      if (n == 0)
		goto match_failure;
	      *p = 0;
	      nassigned++;
	    }
	  nread += n;
	  break;

	case CT_STRING:
	  /* like CCL, but zero-length string OK, & no NOSKIP */
	  
	  if (width == 0)
            width = (size_t)~0;
          if (flags & LONG) 
            {
              /* Process %S and %ls placeholders */
              if ((flags & SUPPRESS) == 0)
                wcp = va_arg(ap, wchar_t *);
              else
                wcp = &wc;
              n = 0;
              while (!isspace(*fp->_p) && width != 0) 
                {
                  if (n == MB_CUR_MAX)
                    goto input_failure;
                  buf[n++] = *fp->_p;
                  fp->_r -= 1;
                  #ifndef SMALL_SCANF
                  fp->_p += 1;
                  memset((void *)&state, '\0', sizeof(mbstate_t));
                  if ((mbslen = _mbrtowc_r(rptr, wcp, buf, n, &state)) 
                                                        == (size_t)-1)
                    goto input_failure;
                  #else
                  *fp->_p = new_getchar();
                  #endif
                  
                  if (mbslen == 0)
                    *wcp = L'\0';
                  
                  if (mbslen != (size_t)-2) /* Incomplete sequence */
                    {
                      if (iswspace(*wcp)) 
                        {
                          
                          while (n != 0)
                            #ifndef SMALL_SCANF
                            ungetc(buf[--n], fp);
                            #else
                            __io_ungetc(buf[--n]);
                            #endif
                          break;
                         
                        }
                        
                      nread += n;
                      width -= 1;
                      if ((flags & SUPPRESS) == 0)
                        wcp += 1;
                      n = 0;
                    }
       				#ifndef SMALL_SCANF
                  if (BufferEmpty) 
                    {
                      if (n != 0)
                        goto input_failure;
                      break;
                    }
                 #endif
                
                }
              if (!(flags & SUPPRESS)) 
                {
                  *wcp = L'\0';
                  nassigned++;
                }
            }
          else if (flags & SUPPRESS) 
	    {
	      n = 0;
	      while (!isspace (*fp->_p))
		{
		  #ifndef SMALL_SCANF
		  n++, fp->_r--, fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    break;
		 #else
		    n++;
		    *fp->_p = new_getchar();		    
		    if (*fp->_p == '\0') break;
		  #endif
		  
		}
	      nread += n;
	    }
	  else
	    {
	      p0 = p = va_arg (ap, char *);
	      while (!isspace (*fp->_p))
		{
		  #ifndef SMALL_SCANF
		  fp->_r--;
		  *p++ = *fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    break;
		  #else
		    *p++=*fp->_p;
		    *fp->_p = new_getchar();		    
		    if (*fp->_p == '\0') break;
		  #endif
	 
		  
		}
	      *p = 0;
	      nread += p - p0;
	      nassigned++;
	    }
	  continue;

	case CT_INT:
	  /* scan an integer as if by strtol/strtoul */
#ifdef hardway
	  if (width == 0 || width > sizeof (buf) - 1)
	    width = sizeof (buf) - 1;
#else
	  /* size_t is unsigned, hence this optimisation */
	  if (--width > sizeof (buf) - 2)
	    width = sizeof (buf) - 2;
	  width++;
#endif
	  flags |= SIGNOK | NDIGITS | NZDIGITS;
	  for (p = buf; width; width--)
	    {     
	      c = *fp->_p;
	      /*
	       * Switch on the character; `goto ok' if we
	       * accept it as a part of number.
	       */
	      switch (c)
		{
		  /*
		   * The digit 0 is always legal, but is special.
		   * For %i conversions, if no digits (zero or nonzero)
		   * have been scanned (only signs), we will have base==0.
		   * In that case, we should set it to 8 and enable 0x
		   * prefixing. Also, if we have not scanned zero digits
		   * before this, do not turn off prefixing (someone else
		   * will turn it off if we have scanned any nonzero digits).
		   */
		case '0':
		  if (base == 0)
		    {
		      base = 8;
		      flags |= PFXOK;
		    }
		  if (flags & NZDIGITS)
		    flags &= ~(SIGNOK | NZDIGITS | NDIGITS);
		  else
		    flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* 1 through 7 always legal */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		  base = basefix[base];
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* digits 8 and 9 ok iff decimal or hex */
		case '8':
		case '9':
		  base = basefix[base];
		  if (base <= 8)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* letters ok iff hex */
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		  /* no need to fix base here */
		  if (base <= 10)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* sign ok only as first character */
		case '+':
		case '-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      goto ok;
		    }
		  break;

		  /* x ok iff flag still set & 2nd char */
		case 'x':
		case 'X':
		  if (flags & PFXOK && p == buf + 1)
		    {
		      base = 16;/* if %i */
		      flags &= ~PFXOK;
		      goto ok;
		    }		  
		  break;
		}
	      /*
	       * If we got here, c is not a legal character
	       * for a number.  Stop accumulating digits.
	       */
		
	      break;
	    ok:
	      /*
	       * c is legal: store it and look at the next.
	       */
	      *p++ = c;
	      #ifndef SMALL_SCANF
       	 if (--fp->_r > 0)
				fp->_p++;
	       else	     
	     		if (__srefill (fp))
		     	break;		/* EOF */
		   #else
		   	   
		    *fp->_p = new_getchar();	
		         		   
		   #endif
		   	     
     	}
	  /*
	   * If we had only a sign, it is no good; push back the sign.
	   * If the number ends in `x', it was [sign] '0' 'x', so push back
	   * the x and treat it as [sign] '0'.
	   */
	  
	  if (flags & NDIGITS)
	    {
	   if (p > buf)
	       #ifndef SMALL_SCANF
				_CAST_VOID ungetc (*(u_char *)-- p, fp);
			 #else
				_CAST_VOID __io_ungetc (*(u_char *)-- p);
			 #endif
	      goto match_failure;

	    }
	 
	  c = ((u_char *) p)[-1];
	  if (c == 'x' || c == 'X')
	    {
	      --p;
	      #ifndef SMALL_SCANF
	      /*(void)*/ ungetc (c, fp);
	      #else 
	      	__io_ungetc (c);
	      #endif
	  
	    }
	  if ((flags & SUPPRESS) == 0)
	    {
	      u_long res;

	      *p = 0;
	      res = (*ccfn) (rptr, buf, (char **) NULL, base);
	      if (flags & POINTER)
		*(va_arg (ap, _PTR *)) = (_PTR) (unsigned _POINTER_INT) res;
	      else if (flags & CHAR)
		{
		  cp = va_arg (ap, char *);
		  *cp = res;
		}
	      else if (flags & SHORT)
		{
		  sp = va_arg (ap, short *);
		  *sp = res;
		}
	      else if (flags & LONG)
		{
		  lp = va_arg (ap, long *);
		  *lp = res;
		}
#ifndef _NO_LONGLONG
	      else if (flags & LONGDBL)
		{
		  u_long_long resll;
		  if (ccfn == _strtoul_r)
		    resll = _strtoull_r (rptr, buf, (char **) NULL, base);
		  else
		    resll = _strtoll_r (rptr, buf, (char **) NULL, base);
		  llp = va_arg (ap, long long*);
		  *llp = resll;
		}
#endif
	      else
		{
		  ip = va_arg (ap, int *);
		  *ip = res;
		}
	      nassigned++;
	    }
	  nread += p - buf;
	  break;

#ifdef FLOATING_POINT
	case CT_FLOAT:
	{
	  /* scan a floating point number as if by strtod */
	  /* This code used to assume that the number of digits is reasonable.
	     However, ANSI / ISO C makes no such stipulation; we have to get
	     exact results even when there is an unreasonable amount of
	     leading zeroes.  */
	  long leading_zeroes = 0;
	  long zeroes, exp_adjust;
	  char *exp_start = NULL;
#ifdef hardway
	  if (width == 0 || width > sizeof (buf) - 1)
	    width = sizeof (buf) - 1;
#else
	  /* size_t is unsigned, hence this optimisation */
	  if (--width > sizeof (buf) - 2)
	    width = sizeof (buf) - 2;
	  width++;
#endif
	  flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
	  zeroes = 0;
	  exp_adjust = 0;
	  for (p = buf; width; )
	    {
	      c = *fp->_p;
	      /*
	       * This code mimicks the integer conversion
	       * code, but is much simpler.
	       */
	      switch (c)
		{

		case '0':
		  if (flags & NDIGITS)
		    {
		      flags &= ~SIGNOK;
		      zeroes++;
		      goto fskip;
		    }
		  /* Fall through.  */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  flags &= ~(SIGNOK | NDIGITS);
		  goto fok;

		case '+':
		case '-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      goto fok;
		    }
		  break;
		case '.':
		  if (flags & DPTOK)
		    {
		      flags &= ~(SIGNOK | DPTOK);
		      leading_zeroes = zeroes;
		      goto fok;
		    }
		  break;
		case 'e':
		case 'E':
		  /* no exponent without some digits */
		  if ((flags & (NDIGITS | EXPOK)) == EXPOK
		      || ((flags & EXPOK) && zeroes))
		    {
		      if (! (flags & DPTOK))
			{
			  exp_adjust = zeroes - leading_zeroes;
			  exp_start = p;
			}
		      flags =
			(flags & ~(EXPOK | DPTOK)) |
			SIGNOK | NDIGITS;
		      zeroes = 0;
		      goto fok;
		    }
		  break;
		}
	      break;
	    fok:
	      *p++ = c;
	    fskip:
	      width--;
              ++nread;
         #ifndef SMALL_SCANF
	      if (--fp->_r > 0)
		fp->_p++;
	      else
	      if (__srefill (fp))
		break;		/* EOF */
		  #else
		     *fp->_p = new_getchar();
		    if ( (47<*fp->_p<58) && (64<*fp->_p < 71) && (96<*fp->_p<103) ){
		       ;
		    }
		    else
		    {
		       break;
		    }
		  #endif 
	    }
	  if (zeroes)
	    flags &= ~NDIGITS;
	  /*
	   * If no digits, might be missing exponent digits
	   * (just give back the exponent) or might be missing
	   * regular digits, but had sign and/or decimal point.
	   */
	  if (flags & NDIGITS)
	    {
	      if (flags & EXPOK)
		{
		  /* no digits at all */
		
		  while (p > buf)
        {
            #ifndef SMALL_SCANF
		      ungetc (*(u_char *)-- p, fp);
		      #else
		      __io_ungetc(*(u_char *)-- p);
		      #endif
            --nread;
         }
                    
		  goto match_failure;

		}
		  
	      /* just a bad exponent (e and maybe sign) */
	      c = *(u_char *)-- p;
              --nread;
	      if (c != 'e' && c != 'E')
		{
		#ifndef SMALL_SCANF
		  _CAST_VOID ungetc (c, fp);	/* sign */
		#else
		  _CAST_VOID __io_ungetc (c);
		#endif
		  c = *(u_char *)-- p;
                  --nread;
		}
		#ifndef SMALL_SCANF
	      _CAST_VOID ungetc (c, fp);
	   #else
	      _CAST_VOID __io_ungetc (c);
	   #endif
	  
	    }
	  if ((flags & SUPPRESS) == 0)
	    {
	      double res = 0;
#ifdef _NO_LONGDBL
#define QUAD_RES res;
#else  /* !_NO_LONG_DBL */
	      long double qres = 0;
#define QUAD_RES qres;
#endif /* !_NO_LONG_DBL */
	      long new_exp = 0;

	      *p = 0;
	      if ((flags & (DPTOK | EXPOK)) == EXPOK)
		{
		  exp_adjust = zeroes - leading_zeroes;
		  new_exp = -exp_adjust;
		  exp_start = p;
		}
	      else if (exp_adjust)
                new_exp = _strtol_r (rptr, (exp_start + 1), NULL, 10) - exp_adjust;
	      if (exp_adjust)
		{

		  /* If there might not be enough space for the new exponent,
		     truncate some trailing digits to make room.  */
		  if (exp_start >= buf + sizeof (buf) - MAX_LONG_LEN)
		    exp_start = buf + sizeof (buf) - MAX_LONG_LEN - 1;
           sprintf (exp_start, "e%ld", new_exp);
		}

	      /* Current _strtold routine is markedly slower than 
	         _strtod_r.  Only use it if we have a long double
	         result.  */
#ifndef _NO_LONGDBL /* !_NO_LONGDBL */
	      if (flags & LONGDBL)
	      	qres = _strtold (buf, NULL);
	      else
#endif
	        res = _strtod_r (rptr, buf, NULL);
	      if (flags & LONG)
		{
		  dp = va_arg (ap, double *);
		  *dp = res;
		}
	      else if (flags & LONGDBL)
		{
		  ldp = va_arg (ap, _LONG_DOUBLE *);
		  *ldp = QUAD_RES;
		}
	      else
		{
		  flp = va_arg (ap, float *);
		  *flp = res;
		}
	      nassigned++;
	    }
	  break;
	}
#endif /* FLOATING_POINT */

	}
    }
input_failure:
  return nassigned ? nassigned : -1;
match_failure:
  return nassigned;
}

/*
 * Fill in the given table from the scanset at the given format
 * (just after `[').  Return a pointer to the character past the
 * closing `]'.  The table has a 1 wherever characters should be
 * considered part of the scanset.
 */

/*static*/
u_char *
__sccl (tab, fmt)
     register char *tab;
     register u_char *fmt;
{
  register int c, n, v;

  /* first `clear' the whole table */
  c = *fmt++;			/* first char hat => negated scanset */
  if (c == '^')
    {
      v = 1;			/* default => accept */
      c = *fmt++;		/* get new first char */
    }
  else
    v = 0;			/* default => reject */
  /* should probably use memset here */
  for (n = 0; n < 256; n++)
    tab[n] = v;
  if (c == 0)
    return fmt - 1;		/* format ended before closing ] */

  /*
   * Now set the entries corresponding to the actual scanset to the
   * opposite of the above.
   *
   * The first character may be ']' (or '-') without being special; the
   * last character may be '-'.
   */

  v = 1 - v;
  for (;;)
    {
      tab[c] = v;		/* take character c */
    doswitch:
      n = *fmt++;		/* and examine the next */
      switch (n)
	{

	case 0:		/* format ended too soon */
	  return fmt - 1;

	case '-':
	  /*
	   * A scanset of the form [01+-] is defined as `the digit 0, the
	   * digit 1, the character +, the character -', but the effect of a
	   * scanset such as [a-zA-Z0-9] is implementation defined.  The V7
	   * Unix scanf treats `a-z' as `the letters a through z', but treats
	   * `a-a' as `the letter a, the character -, and the letter a'.
	   *
	   * For compatibility, the `-' is not considerd to define a range if
	   * the character following it is either a close bracket (required by
	   * ANSI) or is not numerically greater than the character we just
	   * stored in the table (c).
	   */
	  n = *fmt;
	  if (n == ']' || n < c)
	    {
	      c = '-';
	      break;		/* resume the for(;;) */
	    }
	  fmt++;
	  do
	    {			/* fill in the range */
	      tab[++c] = v;
	    }
	  while (c < n);
#if 1			/* XXX another disgusting compatibility hack */
	  /*
	   * Alas, the V7 Unix scanf also treats formats such
	   * as [a-c-e] as `the letters a through e'. This too
	   * is permitted by the standard....
	   */
	  goto doswitch;
#else
	  c = *fmt++;
	  if (c == 0)
	    return fmt - 1;
	  if (c == ']')
	    return fmt;
#endif

	  break;


	case ']':		/* end of scanset */
	  return fmt;

	default:		/* just another character */
	  c = n;
	  break;
	}
    }
  /* NOTREACHED */
}



