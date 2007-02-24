#ifndef __IO_H__7UTLUP9AG6__
#define __IO_H__7UTLUP9AG6__

#include <AT91SAM7S64.h>


#ifndef BV
#define BV(x) (1<<(x))
#endif

int splhigh(void);

void splx(int saved);

#endif /* __IO_H__7UTLUP9AG6__ */
