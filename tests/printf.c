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

#include <mc1322x.h>
#include <board.h>

#include <stdio.h>
#include <math.h>

#include "tests.h"
#include "config.h"

#define print_size(x) do { \
	printf("sizeof(");			\
	printf(#x);				\
	printf("): %d\n", sizeof(x));		\
	} while(0)

#if (__linux__)
FILE *stderr;

void __assert_fail(void) {
	return;
}
	
int fputs(const char *s, FILE *stream) {
	return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb,
	      FILE *stream) {
	return 0;
}
#endif



int main(void)
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
//	char buf[80];

	uart_init(UART1, 115200);

	print_size(int8_t);
	print_size(uint8_t);
	print_size(int16_t);
	print_size(uint16_t);
	print_size(int32_t);
	print_size(uint32_t);
	print_size(int64_t);
	print_size(uint64_t);

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
	printf("%d %s(s)", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");

	printf("sqrt(5) * 100  = %d\n", (int) (sqrt(5)*100));

//	sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
//	sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
//	sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
//	sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
//	sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
//	sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
//	sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
//	sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);

	while(1) { continue; }
}

/*
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
