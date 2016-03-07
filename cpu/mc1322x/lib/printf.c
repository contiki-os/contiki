/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details.
 *
 *
 */

/**
 * \file lib/printf.c
 *
 * \brief sprintf functions to replace newlib for AVR32 UC3.
 *
 * \author $Author: umanzoli $
 *
 * Created on : 17-mar-2009
 *
 *
 */

/*
 * Copyright 2001, 2002 Georges Menie (www.menie.org)
 * stdarg version contributed by Christian Ettinger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include <mc1322x.h>
#include <stdint.h>

#define __putc(x) uart1_putc(x)

/**
 * Structure to hold data to be passed to print function with format.
 * Aka print context.
 */
struct __print_ctx_t
{
	//! pointer to next char to be filled.
	char*	_ptr;
	//! maximum length of the buffer.
	size_t	_max_len;
};
typedef struct __print_ctx_t	_print_ctx_t;

/**
 * Pad string to right
 */
#define _PRINTFMT_PAD_RIGHT	1

/**
 * Pad the number with zeroes
 */
#define _PRINTFMT_PAD_ZERO	2

/**
 * The following should be enough for 32 bit int
 */
#define _PRINTFMT_INT_BUF_LEN	12

/**
 * Print a character to stdout (if string is null)
 * otherwise, put the character at the end of the provided string.
 */
static void __print_char( _print_ctx_t* ctx, char c )
{
	if( ctx ) {
		if( c == '\r' || c == '\n' ) {
			if( ctx->_max_len > 1 ) {
				*(ctx->_ptr)='\r';
				ctx->_max_len--;
				ctx->_ptr++;
				*(ctx->_ptr)='\n';
				ctx->_max_len--;
				ctx->_ptr++;
			} else {
				*(ctx->_ptr)='\n';
				ctx->_max_len--;
				ctx->_ptr++;
			}
		} else {
			if( ctx->_max_len ) {
				*(ctx->_ptr)=c;
				ctx->_max_len--;
				ctx->_ptr++;
			}
		}
	} else {
		__putc( (uint8_t)c );
	}
}

/**
 * Print a string to a given string.
 */
static int __print_str( _print_ctx_t* ctx,
			const char *string,
			int width,
			int pad,
			int print_limit,
			bool is_number )
{
	int pc = 0;
	int padchar = ' ';
	int i, len;

	if( width > 0 ) {
		register int len = 0;
		register const char *ptr;
		for( ptr = string; *ptr; ++ptr )
			++len;
		if( len >= width )
			width = 0;
		else
			width -= len;
		if( pad & _PRINTFMT_PAD_ZERO )
			padchar = '0';
	}
	if( !( pad & _PRINTFMT_PAD_RIGHT ) ) {
		for( ; width > 0; --width ) {
			__print_char( ctx, padchar );
			++pc;
		}
	}

	// The string to print is not the result of a number conversion to ascii.
	if( false == is_number ) {
		// For a string, printlimit is the max number of characters to display.
		for( ; print_limit && *string; ++string, --print_limit ) {
			__print_char( ctx, *string );
			++pc;
		}
	}

	// The string to print represents an integer number.
	if( true == is_number ) {
		// In this case, printlimit is the min number of digits to print.

		// If the length of the number to print is less than the min nb of i
		// digits to display, we add 0 before printing the number.
		len = strlen( string );
		if( len < print_limit ) {
			i = print_limit - len;
			for( ; i; i-- ) {
				__print_char( ctx, '0' );
				++pc;
			}
		}
	}

	/*
	 * Else: The string to print is not the result of a number conversion to ascii.
	 * For a string, printlimit is the max number of characters to display.
	 */
	for( ; print_limit && *string; ++string, --print_limit ) {
		__print_char( ctx, *string );
		++pc;
	}

	for( ; width > 0; --width ) {
		__print_char( ctx, padchar );
		++pc;
	}

	return pc;
}

/**
 * Print a number to the given string, with the given base.
 */
static int __print_int( _print_ctx_t* ctx,
			int i,
			int b,
			int sg,
			int width,
			int pad,
			int letbase,
			int print_limit )
{
	char print_buf[_PRINTFMT_INT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if( i == 0 ) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return __print_str( ctx, print_buf, width, pad, print_limit, true );
	}

	if( sg && b == 10 && i < 0 ) {
		neg = 1;
		u = -i;
	}

	s = print_buf + _PRINTFMT_INT_BUF_LEN - 1;
	*s = '\0';

	while( u ) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if( neg ) {
		if( width && ( pad & _PRINTFMT_PAD_ZERO ) ) {
			__print_char( ctx, '-' );
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}

	return pc + __print_str( ctx, s, width, pad, print_limit, true );
}
/*
#if __GNUC__
int fprintf( __FILE *stream, const char *format, ... )
{
	return 0;
}
#endif
*/

/**
 * Print the given arguments, with given format onto string out.
 */
static int __print_fmt( _print_ctx_t* ctx, const char *format, va_list args )
{
	int	width;
	int	pad;
	int	print_limit;
	int	pc = 0;
	char	scr[2];

	for( ; *format != 0; ++format ) {
		if( *format == '%' ) {
			++format;
			width = pad = print_limit = 0;

			if( *format == '\0' ) {
				break;
			}

			if( *format == '%' ) {
				goto out;
			}

			if( *format == '-' ) {
				++format;
				pad = _PRINTFMT_PAD_RIGHT;
			}

			while( *format == '0' ) {
				++format;
				pad |= _PRINTFMT_PAD_ZERO;
			}

			for( ; *format >= '0' && *format <= '9'; ++format ) {
				width *= 10;
				width += *format - '0';
			}

			if( *format == '.' ) {
				++format;
				for( ; *format >= '0' && *format <= '9'; ++format ) {
					print_limit *= 10;
					print_limit += *format - '0';
				}
			}

			if( 0 == print_limit ) {
				print_limit--;
			}

			if( *format == 'l' ) {
				++format;
			}

			if( *format == 's' ) {
				register char *s = (char *) va_arg( args, int );
				pc += __print_str( 	ctx,
							s ? s : "(null)",
							width,
							pad,
							print_limit,
							false );
				continue;
			}

			if( *format == 'd' ) {
				pc += __print_int( ctx, va_arg( args, int ), 10, 1, width, pad, 'a', print_limit );
				continue;
			}

			if( ( *format == 'x' ) || ( *format == 'p' ) ) {
				pc += __print_int( ctx, va_arg( args, int ), 16, 0, width, pad, 'a', print_limit );
				continue;
			}

			if( *format == 'X' ) {
				pc += __print_int( ctx, va_arg( args, int ), 16, 0, width, pad, 'A', print_limit );
				continue;
			}

			if( *format == 'u' ) {
				pc += __print_int( ctx, va_arg( args, int ), 10, 0, width, pad, 'a', print_limit );
				continue;
			}

			if( *format == 'c' ) {
				// char are converted to int then pushed on the stack
				scr[0] = (char) va_arg( args, int );
				scr[1] = '\0';
				pc += __print_str( ctx, scr, width, pad, print_limit, false );
				continue;
			}
		} else {
out:
			__print_char( ctx, *format );
			++pc;
		}
	}

	if( ctx && ctx->_max_len ) {
		*(ctx->_ptr) = '\0';
	}

	return pc;
}

#define BLOCK_MEM_SIZE 1024

int sprintf( char *out, const char *format, ... )
{
	int		retval = 0;
	_print_ctx_t	ctx;
	va_list		args;

	ctx._ptr = out;
	ctx._max_len = BLOCK_MEM_SIZE;

	va_start( args, format );
	retval = __print_fmt( &ctx, format, args );
	va_end( args );

	return retval;
}

int printf( const char *format, ... )
{
	int		retval = 0;
//	memory_t*	buf;
	va_list		args;

	/*
	buf = memory_alloc( 10 );

	if( buf ) {
		_print_ctx_t	ctx;
		ctx._ptr = (char*)buf->_data;
		ctx._max_len = BLOCK_MEM_SIZE;

		va_start( args, format );
		retval = __print_fmt( &ctx, format, args );
		va_end( args );

		buf->_len = strlen( (const char*)buf->_data );

//		LCD_WriteString( ll, buf );
		ll++;
		ll &= 0x03;
		if( uart_task_send( buf ) == false ) {
			memory_free( buf );
		}
	}
 */

	va_start( args, format );
	retval = __print_fmt( NULL, format, args );
	va_end( args );

	return retval;
}
