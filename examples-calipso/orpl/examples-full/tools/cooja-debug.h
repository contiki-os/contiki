/*
 * cooja-debug.h
 *
 *  Created on: Jan 12, 2011
 *      Author: simonduq
 */

#ifndef COOJA_DEBUG_H_
#define COOJA_DEBUG_H_

#define WITH_DEBUG 1

#if WITH_DEBUG
volatile char *cooja_debug_ptr;
#define HEXC(c) (((c) & 0xf) <= 9 ? ((c) & 0xf) + '0' : ((c) & 0xf) + 'a' - 10)
#define COOJA_DEBUG_STR(str) do { cooja_debug_ptr = str; } while(0);
#define COOJA_DEBUG_CINT(c, val)
#define COOJA_DEBUG_INT(val) do { char tmp[10] = {0}; uint16_t v = (uint16_t)(val); int i=9; if(v==0) tmp[--i] = '0'; while(v>0) { tmp[--i] = '0' + v%10; v /= 10; } cooja_debug_ptr = tmp+i;  } while(0)
#define COOJA_DEBUG_INTH(val) do { char tmp[5] = {0}; uint16_t v = (uint16_t)(val); tmp[0] = HEXC(v>>12); tmp[1] = HEXC(v>>8); tmp[2] = HEXC(v>>4); tmp[3] = HEXC(v); cooja_debug_ptr = tmp;  } while(0)
#define COOJA_DEBUG_PRINTF(...) do { char tmp[200]; sprintf(tmp, __VA_ARGS__); cooja_debug_ptr = tmp; } while(0);
#define COOJA_DEBUG_ADDR(addr) { int i; COOJA_DEBUG_STR("addr"); for(i=0;i<16;i++) COOJA_DEBUG_INTH(((u8_t *)addr)[i]); COOJA_DEBUG_STR("end addr");}
#else
#define COOJA_DEBUG_STR(str)
#define COOJA_DEBUG_CINT(c, val)
#define COOJA_DEBUG_INT(val)
#define COOJA_DEBUG_INTH(val)
#define COOJA_DEBUG_PRINTF(...)
#define COOJA_DEBUG_ADDR(addr)
#endif

#endif /* COOJA_DEBUG_H_ */
