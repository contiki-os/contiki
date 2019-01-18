#ifndef __DEBUG_H__
#define __DEBUG_H__

#if 0
#define debug_print(x)
#define debug_print8(x) 

#else

#include "avr/pgmspace.h"

void debug_print(char *str);

void debug_print8(unsigned char v);
void debug_print16(unsigned short v);
#endif



#endif /* __DEBUG_H__ */
