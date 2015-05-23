#ifndef DEBUG_H_
#define DEBUG_H_

#if 0
#define debug_print(x)
#define debug_print8(x) 

#else

#include "avr/pgmspace.h"

void debug_print(char *str);

void debug_print8(unsigned char v);
void debug_print16(unsigned short v);
#endif



#endif /* DEBUG_H_ */
