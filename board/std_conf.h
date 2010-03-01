#ifndef STDCONF_H
#define STDCONF_H

#include <default_lowlevel.h>

#ifndef vreg_init
#define vreg_init() default_vreg_init()
#endif

/* XTAL TUNE parameters */
/* recommended defaults from the datasheet */

/* Coarse tune: add 4pf */
#ifndef CTUNE_4PF
#define CTUNE_4PF 1
#endif
/* Coarse tune: add 0-7 pf */
#ifndef CTUNE
#define CTUNE 5
#endif
/* Fine tune: add FTUNE * 156fF (FTUNE is 4bits) */
#ifndef FTUNE
#define FTUNE 16
#endif
/* datasheet recommends taht you don't change this */
#ifndef IBIAS
#define IBIAS 0x1F
#endif

#endif
