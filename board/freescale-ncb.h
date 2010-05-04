#ifndef BOARD_FREESCALE_NCB_H
#define BOARD_FREESCALE_NCB_H

#define LED_RED    23
#define LED_GREEN  24
#define LED_BLUE   25

/* XTAL TUNE parameters */
/* see http://devl.org/pipermail/mc1322x/2009-December/000162.html */
/* for details about how to make this measurment */

/* Coarse tune: add 4pf */
#define CTUNE_4PF 1
/* Coarse tune: add 0-15 pf */
#define CTUNE 8
/* Fine tune: add FTUNE * 156fF (FTUNE is 4bits) */
#define FTUNE 15

#include <std_conf.h>

#endif
