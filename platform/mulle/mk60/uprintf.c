/**
 * \file
 *         printf and sprintf functions.
 * \author
 *         Georges Menie
 *         Henrik
 *         Tony Persson <tony.persson@rubico.com>
 */
 
/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)
	stdarg version contributed by Christian Ettinger

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdarg.h>
#include "MK60N512VMD100.h"
#include "uart.h"


/* The following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

#define PAD_RIGHT 1
#define PAD_ZERO  2

static int init = 0;

static int prints(char **out, const char *string, int width, int pad);

static void printchar(char **str, int c)
{
	if (str)
    {
		**str = c;
		++(*str);
	}
	else
    {
        uart_putchar(c);
    }
}


static void uli2a(unsigned long long num, unsigned long base, int uc, char **out, int width, int pad)
{
#if 0 /* TODO: Get some error in this code. Fix if needed */
	int n=0;
	unsigned long long d=1;
	char bf_tmp[30];
	char*bf = bf_tmp;

	while (num/d >= base)
		d*=base;
	while (d!=0L) {
		int dgt = num / d;
		num%=d;
		d/=base;
		if (n || dgt>0|| d==0) {
			*bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
			++n;
		}
	}
	*bf=0;
//	while (n++ < 8 && base == 16) {
//		*bf_out++ = '0';
//	}
	//strcpy(bf_out, bf_tmp);
	prints (out, bf_tmp, width, pad);
#else
    /* Placeholder while function is not woring correctly */
	uart_putchar('x');
	uart_putchar('x');
	uart_putchar('x');
#endif
}


static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0)
    {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT))
    {
		for ( ; width > 0; --width)
        {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string)
    {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width)
    {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}


static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0)
    {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u)
    {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg)
    {
		if( width && (pad & PAD_ZERO) )
        {
			printchar (out, '-');
			++pc;
			--width;
		}
		else
        {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print(char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format)
    {
		if (*format == '%')
        {
			char lng = 0;
			++format;
			width = pad = 0;
			if (*format == 'l') { lng = 1; ++format; }
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-')
            {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0')
            {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format)
            {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' )
            {
				register char *s = (char *)va_arg( args, int );
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' )
            {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' )
            {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' )
            {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' )
            {
				if (lng)
				{
					// Long
					uli2a(va_arg( args, unsigned long long), 10,0, out, width, pad);

				}
				else
				{
					pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				}
				continue;
			}
			if( *format == 'c' )
            {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
        else if(*format == '\n')
        {
            printchar (out, '\r');
            printchar (out, '\n');
            pc++;
        }
		else
        {
		out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

int printf2(const char *format, ...)
{
		int r;
        va_list args;
        if (!init)
        {
			uart_init();
            init = 1;
        }

        va_start( args, format );
        r = print( 0, format, args );
        while (!(UART1_S1 & UART_S1_TC_MASK));
        return r;
}

int sprintf2(char *out, const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return print( &out, format, args );
}

#ifdef TEST_PRINTF
int main(void)
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
	char buf[80];

	mi = (1 << (bs-1)) + 1;
	printf("%s\n", ptr);
	printf("printf test\n");
	printf("%s is null pointer\n", np);
	printf("%d = 5\n", i);
	printf("%d = - max int\n", mi);
	printf("char %c = 'a'\n", 'a');
	printf("hex %x = ff\n", 0xff);
	printf("hex %02x = 00\n", 0);
	printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	printf("%d %s(s)%", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");
	sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
	sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
	sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
	sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);

	return 0;
}

/*
 * if you compile this file with
 *   gcc -Wall $(YOUR_C_OPTIONS) -DTEST_PRINTF -c printf.c
 * you will get a normal warning:
 *   printf.c:214: warning: spurious trailing `%' in format
 * this line is testing an invalid % at the end of the format string.
 *
 * this should display (on 32bit int machine) :
 *
 * Hello world!
 * printf test
 * (null) is null pointer
 * 5 = 5
 * -2147483647 = - max int
 * char a = 'a'
 * hex ff = ff
 * hex 00 = 00
 * signed -3 = unsigned 4294967293 = hex fffffffd
 * 0 message(s)
 * 0 message(s) with %
 * justif: "left      "
 * justif: "     right"
 *  3: 0003 zero padded
 *  3: 3    left justif.
 *  3:    3 right justif.
 * -3: -003 zero padded
 * -3: -3   left justif.
 * -3:   -3 right justif.
 */

#endif
