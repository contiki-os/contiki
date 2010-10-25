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
	<<scanf>>, <<fscanf>>, <<sscanf>>---scan and format input

INDEX
	scanf
INDEX
	fscanf
INDEX
	sscanf

ANSI_SYNOPSIS
        #include <stdio.h>

        int scanf(const char *<[format]> [, <[arg]>, ...]);
        int fscanf(FILE *<[fd]>, const char *<[format]> [, <[arg]>, ...]);
        int sscanf(const char *<[str]>, const char *<[format]> 
                   [, <[arg]>, ...]);

        int _scanf_r(struct _reent *<[ptr]>, const char *<[format]> [, <[arg]>, ...]);
        int _fscanf_r(struct _reent *<[ptr]>, FILE *<[fd]>, const char *<[format]> [, <[arg]>, ...]);
        int _sscanf_r(struct _reent *<[ptr]>, const char *<[str]>, const char *<[format]> 
                   [, <[arg]>, ...]);


TRAD_SYNOPSIS
	#include <stdio.h>

	int scanf(<[format]> [, <[arg]>, ...])
	char *<[format]>;

	int fscanf(<[fd]>, <[format]> [, <[arg]>, ...]);
	FILE *<[fd]>;
	char *<[format]>;

	int sscanf(<[str]>, <[format]> [, <[arg]>, ...]);
	char *<[str]>;
	char *<[format]>;

	int _scanf_r(<[ptr]>, <[format]> [, <[arg]>, ...])
        struct _reent *<[ptr]>;
	char *<[format]>;

	int _fscanf_r(<[ptr]>, <[fd]>, <[format]> [, <[arg]>, ...]);
        struct _reent *<[ptr]>;
	FILE *<[fd]>;
	char *<[format]>;

	int _sscanf_r(<[ptr]>, <[str]>, <[format]> [, <[arg]>, ...]);
        struct _reent *<[ptr]>;
	char *<[str]>;
	char *<[format]>;


DESCRIPTION
        <<scanf>> scans a series of input fields from standard input,
		one character at a time.  Each field is interpreted according to
		a format specifier passed to <<scanf>> in the format string at
        <<*<[format]>>>.  <<scanf>> stores the interpreted input from
		each field at the address passed to it as the corresponding argument
		following <[format]>.  You must supply the same number of
		format specifiers and address arguments as there are input fields.

        There must be sufficient address arguments for the given format
        specifiers; if not the results are unpredictable and likely
        disasterous.  Excess address arguments are merely ignored.

        <<scanf>> often produces unexpected results if the input diverges from
        an expected pattern. Since the combination of <<gets>> or <<fgets>>
        followed by <<sscanf>> is safe and easy, that is the preferred way
        to be certain that a program is synchronized with input at the end
		of a line.

        <<fscanf>> and <<sscanf>> are identical to <<scanf>>, other than the
        source of input: <<fscanf>> reads from a file, and <<sscanf>>
		from a string.

        The routines <<_scanf_r>>, <<_fscanf_r>>, and <<_sscanf_r>> are reentrant
        versions of <<scanf>>, <<fscanf>>, and <<sscanf>> that take an additional
        first argument pointing to a reentrancy structure.

        The string at <<*<[format]>>> is a character sequence composed
        of zero or more directives. Directives are composed of
        one or more whitespace characters, non-whitespace characters,
        and format specifications.

        Whitespace characters are blank (<< >>), tab (<<\t>>), or
		newline (<<\n>>).
        When <<scanf>> encounters a whitespace character in the format string
        it will read (but not store) all consecutive whitespace characters
        up to the next non-whitespace character in the input.

        Non-whitespace characters are all other ASCII characters except the
        percent sign (<<%>>).  When <<scanf>> encounters a non-whitespace
        character in the format string it will read, but not store
        a matching non-whitespace character.

        Format specifications tell <<scanf>> to read and convert characters
        from the input field into specific types of values, and store then
        in the locations specified by the address arguments.

        Trailing whitespace is left unread unless explicitly
        matched in the format string.

        The format specifiers must begin with a percent sign (<<%>>)
        and have the following form:

.       %[*][<[width]>][<[size]>]<[type]>

        Each format specification begins with the percent character (<<%>>).
        The other fields are:
	o+
		o *
		an optional marker; if present, it suppresses interpretation and
        assignment of this input field.

        o <[width]>
		an optional maximum field width: a decimal integer,
		which controls the maximum number of characters that
		will be read before converting the current input field.  If the
		input field has fewer than <[width]> characters, <<scanf>>
		reads all the characters in the field, and then
		proceeds with the next field and its format specification.

		If a whitespace or a non-convertable character occurs
		before <[width]> character are read, the characters up
		to that character are read, converted, and stored.
		Then <<scanf>> proceeds to the next format specification.

        o size
		<<h>>, <<l>>, and <<L>> are optional size characters which
		override the default way that <<scanf>> interprets the
		data type of the corresponding argument.


.Modifier   Type(s)
.   hh      d, i, o, u, x, n  convert input to char,
.                             store in char object
.
.   h       d, i, o, u, x, n  convert input to short,
.                             store in short object
.
.   h       D, I, O, U, X     no effect
.           e, f, c, s, p
.
.   l       d, i, o, u, x, n  convert input to long,
.                             store in long object
.
.   l       e, f, g           convert input to double
.                             store in a double object
.
.   l       D, I, O, U, X     no effect
.           c, s, p
.
.   ll      d, i, o, u, x, n  convert to long long,
.                             store in long long
.
.   L       d, i, o, u, x, n  convert to long long,
.                             store in long long
.
.   L       e, f, g, E, G     convert to long double,
.                             store in long double
.
.   L      all others         no effect


        o <[type]>

		A character to specify what kind of conversion
                <<scanf>> performs.  Here is a table of the conversion
                characters:

		o+
		o  %
		No conversion is done; the percent character (<<%>>) is stored.

		o c
		Scans one character.  Corresponding <[arg]>: <<(char *arg)>>.

		o s
		Reads a character string into the array supplied.
		Corresponding <[arg]>: <<(char arg[])>>.

		o  [<[pattern]>]
		Reads a non-empty character string into memory
		starting at <[arg]>.  This area must be large
		enough to accept the sequence and a
		terminating null character which will be added
		automatically.  (<[pattern]> is discussed in the paragraph following
		this table). Corresponding <[arg]>: <<(char *arg)>>.

		o d
		Reads a decimal integer into the corresponding <[arg]>: <<(int *arg)>>.

		o D
		Reads a decimal integer into the corresponding
		<[arg]>: <<(long *arg)>>.

		o o
		Reads an octal integer into the corresponding <[arg]>: <<(int *arg)>>.

		o O
		Reads an octal integer into the corresponding <[arg]>: <<(long *arg)>>.

		o u
		Reads an unsigned decimal integer into the corresponding
		<[arg]>: <<(unsigned int *arg)>>.
			

		o U
		Reads an unsigned decimal integer into the corresponding <[arg]>:
		<<(unsigned long *arg)>>.

		o x,X
		Read a hexadecimal integer into the corresponding <[arg]>:
		<<(int *arg)>>.

		o e, f, g
		Read a floating-point number into the corresponding <[arg]>:
		<<(float *arg)>>.

		o E, F, G
		Read a floating-point number into the corresponding <[arg]>:
		<<(double *arg)>>.

		o  i
		Reads a decimal, octal or hexadecimal integer into the
		corresponding <[arg]>: <<(int *arg)>>.

		o  I
		Reads a decimal, octal or hexadecimal integer into the
		corresponding <[arg]>: <<(long *arg)>>.

		o  n
		Stores the number of characters read in the corresponding
		<[arg]>: <<(int *arg)>>.

		o  p
                Stores a scanned pointer.  ANSI C leaves the details
		to each implementation; this implementation treats
		<<%p>> exactly the same as <<%U>>.  Corresponding
		<[arg]>: <<(void **arg)>>.  
                o-

	A <[pattern]> of characters surrounded by square brackets can be used
	instead of the <<s>> type character.  <[pattern]> is a set of
	characters which define a search set of possible characters making up
	the <<scanf>> input field.  If the first character in the brackets is a
	caret (<<^>>), the search set is inverted to include all ASCII characters
	except those between the brackets.  There is also a range facility
	which you can use as a shortcut. <<%[0-9] >> matches all decimal digits.
	The hyphen must not be the first or last character in the set.
	The character prior to the hyphen must be lexically less than the
	character after it.

	Here are some <[pattern]> examples:
		o+
		o %[abcd]
		matches strings containing only <<a>>, <<b>>, <<c>>, and <<d>>.

		o %[^abcd]
		matches strings containing any characters except <<a>>, <<b>>,
		<<c>>, or <<d>>

		o %[A-DW-Z]
		matches strings containing <<A>>, <<B>>, <<C>>, <<D>>, <<W>>,
		<<X>>, <<Y>>, <<Z>>

		o %[z-a]
		matches the characters  <<z>>, <<->>, and <<a>>
		o-

	Floating point numbers (for field types <<e>>, <<f>>, <<g>>, <<E>>,
	<<F>>, <<G>>) must correspond to the following general form:

.		[+/-] ddddd[.]ddd [E|e[+|-]ddd]

	where objects inclosed in square brackets are optional, and <<ddd>>
	represents decimal, octal, or hexadecimal digits.
	o-

RETURNS
        <<scanf>> returns the number of input fields successfully
        scanned, converted and stored; the return value does
        not include scanned fields which were not stored.

        If <<scanf>> attempts to read at end-of-file, the return
        value is <<EOF>>.

        If no fields were stored, the return value is <<0>>.

        <<scanf>> might stop scanning a particular field before
        reaching the normal field end character, or may
        terminate entirely.

        <<scanf>> stops scanning and storing the current field
        and moves to the next input field (if any)
        in any of the following situations:

	O+
	o       The assignment suppressing character (<<*>>) appears
	after the <<%>> in the format specification; the current
	input field is scanned but not stored.

	o       <[width]> characters have been read (<[width]> is a
	width specification, a positive decimal integer).

	o       The next character read cannot be converted
	under the the current format (for example,
	if a <<Z>> is read when the format is decimal).

	o       The next character in the input field does not appear
	in the search set (or does appear in the inverted search set).
	O-

	When <<scanf>> stops scanning the current input field for one of
	these reasons, the next character is considered unread and
	used as the first character of the following input field, or the
	first character in a subsequent read operation on the input.

	<<scanf>> will terminate under the following circumstances:

	O+
	o       The next character in the input field conflicts
	with a corresponding non-whitespace character in the
	format string.

	o       The next character in the input field is <<EOF>>.

	o       The format string has been exhausted.
	O-

	When the format string contains a character sequence that is
	not part of a format specification, the same character
	sequence must appear in the input; <<scanf>> will
	scan but not store the matched characters.  If a
	conflict occurs, the first conflicting character remains in the input
	as if it had never been read.

PORTABILITY
<<scanf>> is ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <string.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "local.h"

/* | ARGSUSED */
/*SUPPRESS 590*/
static
_READ_WRITE_RETURN_TYPE
eofread (cookie, buf, len)
     _PTR cookie;
     char *buf;
     int len;
{
  return 0;
}

#ifndef _REENT_ONLY 

#ifdef _HAVE_STDC
int 
_DEFUN (sscanf, (str, fmt), _CONST char *str _AND _CONST char *fmt _DOTS)
#else
int 
sscanf (str, fmt, va_alist)
     _CONST char *str;
     _CONST char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;
  FILE f;

  f._flags = __SRD;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._r = strlen (str);
  f._read = eofread;
  f._ub._base = NULL;
  f._lb._base = NULL;
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = __svfscanf_r (_REENT, &f, fmt, ap);
  va_end (ap);
  return ret;
}

#endif /* !_REENT_ONLY */

#ifdef _HAVE_STDC
int 
_DEFUN (_sscanf_r, (ptr, str, fmt), struct _reent *ptr _AND _CONST char *str _AND _CONST char *fmt _DOTS)
#else
int 
_sscanf_r (ptr, str, fmt, va_alist)
     struct _reent *ptr;
     _CONST char *str;
     _CONST char *fmt;
     va_dcl
#endif
{
  int ret;
  va_list ap;
  FILE f;

  f._flags = __SRD;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._r = strlen (str);
  f._read = eofread;
  f._ub._base = NULL;
  f._lb._base = NULL;
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = __svfscanf_r (ptr, &f, fmt, ap);
  va_end (ap);
  return ret;
}
