#ifndef AVR_RNG_H_
#define AVR_RNG_H_

#include <inttypes.h>

//!	Returns a byte with eight random bits.
/*!	This function is very slow, and should only
**	be used when true random entropy is required.
**	The time it will take to complete is non-deterministic.
*/
extern uint8_t rng_get_uint8();

#endif