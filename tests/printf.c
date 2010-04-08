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

int main(void)
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
//	char buf[80];

	uart_init(INC, MOD, SAMP);

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
