#ifndef BOARD_REDBEE_R1_H
#define BOARD_REDBEE_R1_H

#define LED_RED   8
#define LED_GREEN 9
#define LED_BLUE  10

/* XTAL TUNE parameters */
/* see http://devl.org/pipermail/mc1322x/2009-December/000162.html */
/* for details about how to make this measurment */

/* Coarse tune: add 4pf */
#define CTUNE_4PF 1
/* Coarse tune: add 0-15 pf */
#define CTUNE 3
/* Fine tune: add FTUNE * 156fF (FTUNE is 4bits) */
#define FTUNE 2

#include <std_conf.h>

#endif
