#ifndef BOARD_QUAHOG10_H
#define BOARD_QUAHOG10_H

#define LED_REDBANK_0		(0x1ULL << 2)
#define LED_REDBANK_1		(0x1ULL << 0)
#define LED_REDBANK_2		(0x1ULL << 28)
#define LED_REDBANK_3		(0x1ULL << 25)
#define LED_REDBANK_4		(0x1ULL << 43)

#define LED_CENTERRED		(0x1ULL << 12)
#define LED_CENTERGREEN		(0x1ULL << 13)

#define LED_SIDERED			(0x1ULL << 12)
#define LED_SIDEGREEN		(0x1ULL << 21)
#define LED_SIDEYELLOW		(0x1ULL << 35)

// Buttons in GPIO mode
#define BUTTON1_OUT			(0x1ULL << 23)
#define BUTTON1_IN			(0x1ULL << 27)
#define BUTTON2_OUT			(0x1ULL << 22)
#define BUTTON2_IN			(0x1ULL << 26)

#define LED_RED   12
#define LED_GREEN 21
#define LED_BLUE  35

/* XTAL TUNE parameters */
/* see http://devl.org/pipermail/mc1322x/2009-December/000162.html */
/* for details about how to make this measurment */

/* Econotag also needs an addtional 12pf on board */
/* Coarse tune: add 4pf */
#define CTUNE_4PF 1
/* Coarse tune: add 0-15 pf (CTUNE is 4 bits) */
#define CTUNE 11
/* Fine tune: add FTUNE * 156fF (FTUNE is 5bits) */
#define FTUNE 7

#include <std_conf.h>

#endif
