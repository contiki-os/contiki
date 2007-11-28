#ifndef __LIBCONIO_ARCH_MSLL_H__
#define __LIBCONIO_ARCH_MSLL_H__
#include "contiki.h"

void clrscr_arch(void);
void libputc_arch(unsigned char ch);
void libputs_arch(char* str);

unsigned char wherex(void);
unsigned char wherey(void);
void gotoxy(unsigned char x, unsigned char y);


#endif /* __LIBCONIO_ARCH_MSLL_H__ */
