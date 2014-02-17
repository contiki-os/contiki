/**
 * \file
 *         Overrides stdio.h so that we use uprintf functions and UART.
 * \author
 *         Tony Persson <tony.persson@rubico.com>
 */
 
#ifndef __MY_STDIO_H__
#define __MY_STDIO_H__

#include "uprintf.h"

#define printf(...) printf2(__VA_ARGS__)
#define sprintf(...) sprintf2(__VA_ARGS__)

#endif
