#include "utils.h"

/* Timer registers are all 16-bit wide with 16-bit access only */
#define TMR_OFFSET      (0x20)
#define TMR_BASE        (0x80007000)
#define TMR0_BASE       (TMR_BASE)
#define TMR1_BASE       (TMR_BASE + TMR_OFFSET*1)
#define TMR2_BASE       (TMR_BASE + TMR_OFFSET*2)
#define TMR3_BASE       (TMR_BASE + TMR_OFFSET*3)

#define TMR_REGOFF_COMP1    (0x0)
#define TMR_REGOFF_COMP2    (0x2)
#define TMR_REGOFF_CAPT     (0x4)
#define TMR_REGOFF_LOAD     (0x6)
#define TMR_REGOFF_HOLD     (0x8)
#define TMR_REGOFF_CNTR     (0xa)
#define TMR_REGOFF_CTRL     (0xc)
#define TMR_REGOFF_SCTRL    (0xe)
#define TMR_REGOFF_CMPLD1   (0x10)
#define TMR_REGOFF_CMPLD2   (0x12)
#define TMR_REGOFF_CSCTRL   (0x14)
#define TMR_REGOFF_ENBL     (0x1e)

/* one enable register to rule them all */
#define TMR_ENBL     ((volatile uint16_t *) TMR0_BASE + TMR_REGOFF_ENBL)

/* Timer 0 registers */
#define TMR0_COMP1   ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_COMP1))
#define TMR0_COMP_UP TMR0_COMP1
#define TMR0_COMP2   (TMR0_BASE + TMR_REGOFF_COMP2)
#define TMR0_COMP_DOWN TMR0_COMP2
#define TMR0_CAPT    ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_CAPT))
#define TMR0_LOAD    ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_LOAD))
#define TMR0_HOLD    ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_HOLD))
#define TMR0_CNTR    ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_CTRL))
#define TMR0_CTRL    ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_CTRL))
#define TMR0_SCTRL   ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_SCTRL))
#define TMR0_CMPLD1  ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_CMPLD1))
#define TMR0_CMPLD2  ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_CMPLD2))
#define TMR0_CSCTRL  ((volatile uint16_t *) (TMR0_BASE + TMR_REGOFF_CSCTRL))

/* Timer 1 registers */
#define TMR1_COMP1   ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_COMP1))
#define TMR1_COMP_UP TMR1_COMP1
#define TMR1_COMP2   ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_COMP2))
#define TMR1_COMP_DOWN TMR1_COMP2
#define TMR1_CAPT    ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_CAPT))
#define TMR1_LOAD    ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_LOAD))
#define TMR1_HOLD    ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_HOLD))
#define TMR1_CNTR    ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_CTRL))
#define TMR1_CTRL    ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_CTRL))
#define TMR1_SCTRL   ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_SCTRL))
#define TMR1_CMPLD1  ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_CMPLD1))
#define TMR1_CMPLD2  ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_CMPLD2))
#define TMR1_CSCTRL  ((volatile uint16_t *) (TMR1_BASE + TMR_REGOFF_CSCTRL))

/* Timer 2 registers */
#define TMR2_COMP1   ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_COMP1))
#define TMR2_COMP_UP TMR2_COMP1
#define TMR2_COMP2   ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_COMP2))
#define TMR2_COMP_DOWN TMR2_COMP2
#define TMR2_CAPT    ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_CAPT))
#define TMR2_LOAD    ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_LOAD))
#define TMR2_HOLD    ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_HOLD))
#define TMR2_CNTR    ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_CTRL))
#define TMR2_CTRL    ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_CTRL))
#define TMR2_SCTRL   ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_SCTRL))
#define TMR2_CMPLD1  ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_CMPLD1))
#define TMR2_CMPLD2  ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_CMPLD2))
#define TMR2_CSCTRL  ((volatile uint16_t *) (TMR2_BASE + TMR_REGOFF_CSCTRL))

/* Timer 3 registers */
#define TMR3_COMP1   ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_COMP1))
#define TMR3_COMP_UP TMR3_COMP1
#define TMR3_COMP2   ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_COMP2))
#define TMR3_COMP_DOWN TMR3_COMP2
#define TMR3_CAPT    ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_CAPT))
#define TMR3_LOAD    ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_LOAD))
#define TMR3_HOLD    ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_HOLD))
#define TMR3_CNTR    ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_CTRL))
#define TMR3_CTRL    ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_CTRL))
#define TMR3_SCTRL   ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_SCTRL))
#define TMR3_CMPLD1  ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_CMPLD1))
#define TMR3_CMPLD2  ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_CMPLD2))
#define TMR3_CSCTRL  ((volatile uint16_t *) (TMR3_BASE + TMR_REGOFF_CSCTRL))

#define TCF  15
#define TCF1 4
#define TCF2 5

#define TMR(num, reg)  CAT2(TMR,num,_##reg)

