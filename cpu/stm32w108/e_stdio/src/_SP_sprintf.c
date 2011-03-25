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

/*

FUNCTION
        <<printf>>, <<fprintf>>, <<asprintf>>, <<sprintf>>, <<snprintf>>---format output
INDEX
	fprintf
INDEX
	printf
INDEX
	asprintf
INDEX
	sprintf
INDEX
	snprintf

ANSI_SYNOPSIS
        #include <stdio.h>

        int printf(const char *<[format]> [, <[arg]>, ...]);
        int fprintf(FILE *<[fd]>, const char *<[format]> [, <[arg]>, ...]);
        int sprintf(char *<[str]>, const char *<[format]> [, <[arg]>, ...]);
        int asprintf(char **<[strp]>, const char *<[format]> [, <[arg]>, ...]);
        int snprintf(char *<[str]>, size_t <[size]>, const char *<[format]> [, <[arg]>, ...]);

TRAD_SYNOPSIS
	#include <stdio.h>

	int printf(<[format]> [, <[arg]>, ...])
	char *<[format]>;

	int fprintf(<[fd]>, <[format]> [, <[arg]>, ...]);
	FILE *<[fd]>;
	char *<[format]>;

	int asprintf(<[strp]>, <[format]> [, <[arg]>, ...]);
	char **<[strp]>;
	char *<[format]>;

	int sprintf(<[str]>, <[format]> [, <[arg]>, ...]);
	char *<[str]>;
	char *<[format]>;

	int snprintf(<[str]>, size_t <[size]>, <[format]> [, <[arg]>, ...]);
	char *<[str]>;
        size_t <[size]>;
	char *<[format]>;

DESCRIPTION
        <<printf>> accepts a series of arguments, applies to each a
        format specifier from <<*<[format]>>>, and writes the
        formatted data to <<stdout>>, terminated with a null character.
        The behavior of <<printf>> is undefined if there are not enough
        arguments for the format.
        <<printf>> returns when it reaches the end of the format string.
        If there are more arguments than the format requires, excess
        arguments are ignored.

        <<fprintf>>, <<asprintf>>, <<sprintf>> and <<snprintf>> are identical 
	to <<printf>>, other than the destination of the formatted output: 
	<<fprintf>> sends the output to a specified file <[fd]>, while 
	<<asprintf>> stores the output in a dynamically allocated buffer,
	while <<sprintf>> stores the output in the specified char array 
	<[str]> and <<snprintf>> limits number of characters written to 
	<[str]> to at most <[size]> (including terminating <<0>>).  For 
	<<sprintf>> and <<snprintf>>, the behavior is undefined if the 
	output <<*<[str]>>> overlaps with one of the arguments. For
	<<asprintf>>, <[strp]> points to a pointer to char which is filled
	in with the dynamically allocated buffer.  <[format]> is a pointer 
	to a charater string containing two types of objects: ordinary 
	characters (other than <<%>>), which are copied unchanged to the 
	output, and conversion specifications, each of which is introduced 
	by <<%>>. (To include <<%>> in the output, use <<%%>> in the format 
	string.) A conversion specification has the following form:

.       %[<[flags]>][<[width]>][.<[prec]>][<[size]>][<[type]>]

        The fields of the conversion specification have the following meanings:

        O+
	o <[flags]>

	an optional sequence of characters which control
	output justification, numeric signs, decimal points,
	trailing zeroes, and octal and hex prefixes.
	The flag characters are minus (<<->>), plus (<<+>>),
	space ( ), zero (<<0>>), and sharp (<<#>>).  They can
	appear in any combination.

	o+
    	o -
		The result of the conversion is left justified, and the right is
		padded with blanks.  If you do not use this flag, the result is right
		justified, and padded on the left.

        o +
		The result of a signed conversion (as determined by <[type]>)
		will always begin with a plus or minus sign.  (If you do not use
        this flag, positive values do not begin with a plus sign.)

        o " " (space)
		If the first character of a signed conversion specification
        is not a sign, or if a signed conversion results in no
		characters, the result will begin with a space.  If the
        space ( ) flag and the plus (<<+>>) flag both appear,
		the space flag is ignored.

        o 0
		If the <[type]> character is <<d>>, <<i>>, <<o>>, <<u>>,
		<<x>>, <<X>>, <<e>>, <<E>>, <<f>>, <<g>>, or <<G>>: leading zeroes,
		are used to pad the field width (following any indication of sign or
		base); no spaces are used for padding.  If the zero (<<0>>) and
		minus (<<->>) flags both appear, the zero (<<0>>) flag will
		be ignored.  For <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, and <<X>>
		conversions, if a precision <[prec]> is specified, the zero (<<0>>)
        flag is ignored.
		
		Note that <<0>> is interpreted as a flag, not as the beginning
        of a field width.

        o #
		The result is to be converted to an alternative form, according
		to the next character:

	    o+
		    o 0
			increases precision to force the first digit
                        of the result to be a zero.

			o x
			a non-zero result will have a <<0x>> prefix.

			o X
			a non-zero result will have a <<0X>> prefix.

			o e, E or f
			The result will always contain a decimal point
		        even if no digits follow the point.
                        (Normally, a decimal point appears only if a
			digit follows it.)  Trailing zeroes are removed.

			o g or G
			same as <<e>> or <<E>>, but trailing zeroes
                        are not removed.

			o all others
			undefined.

			o-
      o-

      o <[width]>

	  <[width]> is an optional minimum field width.  You can either
	  specify it directly as a decimal integer, or indirectly by
          using instead an asterisk (<<*>>), in which case an <<int>>
          argument is used as the field width.  Negative field widths
          are not supported; if you attempt to specify a negative field
          width, it is interpreted as a minus (<<->>) flag followed by a
          positive field width.

      o <[prec]>

	  an optional field; if present, it is introduced with `<<.>>'
	  (a period). This field gives the maximum number of
	  characters to print in a conversion; the minimum number of
	  digits of an integer to print, for conversions with <[type]>
	  <<d>>, <<i>>, <<o>>, <<u>>, <<x>>, and <<X>>; the maximum number of
	  significant digits, for the <<g>> and <<G>> conversions;
	  or the number of digits to print after the decimal
	  point, for <<e>>, <<E>>, and <<f>> conversions.  You can specify
	  the precision either directly as a decimal integer or
	  indirectly by using an asterisk (<<*>>), in which case
	  an <<int>> argument is used as the precision.  Supplying a negative
      precision is equivalent to omitting the precision.
	  If only a period is specified the precision is zero.
	  If a precision appears with any other conversion <[type]>
	  than those listed here, the behavior is undefined.

      o  <[size]>

		<<h>>, <<l>>, and <<L>> are optional size characters which
		override the default way that <<printf>> interprets the
		data type of the corresponding argument.  <<h>> forces
		the following <<d>>, <<i>>, <<o>>, <<u>>, <<x>> or <<X>> conversion
		<[type]> to apply to a <<short>> or <<unsigned short>>. <<h>> also
		forces a following <<n>> <[type]> to apply to
		a pointer to a <<short>>. Similarily, an
		<<l>> forces the following <<d>>, <<i>>, <<o>>, <<u>>,
		<<x>> or <<X>> conversion <[type]> to apply to a <<long>> or
		<<unsigned long>>.  <<l>> also forces a following <<n>> <[type]> to
		apply to a pointer to a <<long>>.  <<l>> with <<c>>, <<s>> is
		equivalent to <<C>>, <<S>> respectively.  If an <<h>>
		or an <<l>> appears with another conversion
		specifier, the behavior is undefined.  <<L>> forces a
		following <<e>>, <<E>>, <<f>>, <<g>> or <<G>> conversion <[type]> to
		apply to a <<long double>> argument.  If <<L>> appears with
		any other conversion <[type]>, the behavior is undefined.

     o   <[type]>

		<[type]> specifies what kind of conversion <<printf>> performs.
		Here is a table of these:

	o+
		o %
		prints the percent character (<<%>>)

		o c
		prints <[arg]> as single character
		
		o C
		prints wchar_t <[arg]> as single multibyte character
		
		o s
		prints characters until precision is reached or a null terminator
		is encountered; takes a string pointer

		o S
		converts wchar_t characters to multibyte output characters until
		precision is reached or a null wchar_t terminator
		is encountered; takes a wchar_t pointer

		o d
		prints a signed decimal integer; takes an <<int>> (same as <<i>>)

		o i
		prints a signed decimal integer; takes an <<int>> (same as <<d>>)

		o o
		prints a signed octal integer; takes an <<int>>

		o u
		prints an unsigned decimal integer; takes an <<int>>

		o x
		prints an unsigned hexadecimal integer (using <<abcdef>> as
		digits beyond <<9>>); takes an <<int>>

		o X
		prints an unsigned hexadecimal integer (using <<ABCDEF>> as
		digits beyond <<9>>); takes an <<int>>

		o f
		prints a signed value of the form <<[-]9999.9999>>; takes
		a floating-point number
	
		o e
		prints a signed	value of the form <<[-]9.9999e[+|-]999>>; takes a
		floating-point number

		o E
		prints the same way as <<e>>, but using <<E>> to introduce the
		exponent; takes a floating-point number

		o g
		prints a signed value in either <<f>> or <<e>> form, based on given
		value and precision---trailing zeros and the decimal point are
		printed only if necessary; takes a floating-point number
	
		o G
		prints the same way as <<g>>, but using <<E>> for the exponent if an
		exponent is needed; takes a floating-point number

		o n
		stores (in the same object) a count of the characters written;
		takes a pointer to <<int>>

		o p
		prints a pointer in an implementation-defined format.
		This implementation treats the pointer as an
		<<unsigned long>> (same as <<Lu>>).
	o-
O-


RETURNS
<<sprintf>> and <<asprintf>> return the number of bytes in the output string,
save that the concluding <<NULL>> is not counted.
<<printf>> and <<fprintf>> return the number of characters transmitted.
If an error occurs, <<printf>> and <<fprintf>> return <<EOF>> and
<<asprintf>> returns -1.  No error returns occur for <<sprintf>>.

PORTABILITY
        The  ANSI C standard specifies that implementations must
        support at least formatted output of up to 509 characters.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/


#include <stdio.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <limits.h>
#include <_ansi.h>

#ifndef _SMALL_PRINTF
		#include "local.h"
#else
	#ifdef INTEGER_ONLY
		#define _vfprintf_r  _vfiprintf_r 
	#endif
#endif





#ifndef _SMALL_PRINTF
	int
	#ifdef _HAVE_STDC
	_DEFUN (_sprintf_r, (ptr, str, fmt), struct _reent *ptr _AND char *str _AND _CONST char *fmt _DOTS)
	#else
	_sprintf_r (ptr, str, fmt, va_alist)
	     struct _reent *ptr;
	     char *str;
	     _CONST char *fmt;
	     va_dcl
	#endif
	{
	  int ret;
	  va_list ap;
	  FILE f;
	
	  f._flags = __SWR | __SSTR;
	  f._bf._base = f._p = (unsigned char *) str;
	  f._bf._size = f._w = INT_MAX;
	  f._file = -1;  /* No file. */
	#ifdef _HAVE_STDC
	  va_start (ap, fmt);
	#else
	  va_start (ap);
	#endif
	  ret = _vfprintf_r (ptr, &f, fmt, ap);
	  va_end (ap);
	  *f._p = 0;
	  return (ret);
	}
#endif

#ifndef _REENT_ONLY
int
#ifdef _HAVE_STDC
_DEFUN (sprintf, (str, fmt), char *str _AND _CONST char *fmt _DOTS)
#else
sprintf (str, fmt, va_alist)
     char *str;
     _CONST char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;
  FILE f;

  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = INT_MAX;
  f._file = -1;  /* No file. */
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = _vfprintf_r (_REENT, &f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  return (ret);
}
#endif


