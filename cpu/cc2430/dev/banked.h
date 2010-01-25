/**
 * \file
 *
 *   SDCC bank switching macro define file
 *
 * \author
 * 
 *   Anthony "Asterisk" Ambuehl
 *
 *   SDCC (small device cross compiler) has built-in support for bank switching using predefined macros __banked.
 *   To avoid compilation issues on other compilers include this file which will replace __banked with the empty string on unsupported compilers.
 *
 *   In addition, the file can add the codeseg pragma to place code into specific banks, if specific macro is set.
 *   However the same result can be achieved by using the segment.rules file.
 *
 */

#ifndef __BANKED_H
#ifdef SDCC
#ifndef HAVE_SDCC_BANKING
#define __banked 
#else
#ifdef BANKED_IN_HOME
#pragma codeseg HOME
#endif
#ifdef BANKED_IN_BANK1
#pragma codeseg BANK1
#endif
#ifdef BANKED_IN_BANK2
#pragma codeseg BANK2
#endif
#ifdef BANKED_IN_BANK3
#pragma codeseg BANK3
#endif
#endif
#else
#define __banked
#endif

#endif /*__BANKED_H*/
