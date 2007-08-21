#include <AT91SAM7S64.h>

#ifndef MAX
#define MIN(a,b) (((a) > (b)) ? (b) : (a))
#endif

/* Pins for stepper 0 */
#define STEPPER0_I00 AT91C_PIO_PA17
#define STEPPER0_I01 AT91C_PIO_PA18
#define STEPPER0_PHASE0 AT91C_PIO_PA19

#define STEPPER0_I10 AT91C_PIO_PA21
#define STEPPER0_I11 AT91C_PIO_PA22
#define STEPPER0_PHASE1 AT91C_PIO_PA23

/* Pins for stepper 1 */
#define STEPPER1_I00 AT91C_PIO_PA24
#define STEPPER1_I01 AT91C_PIO_PA25
#define STEPPER1_PHASE0 AT91C_PIO_PA26

#define STEPPER1_I10 AT91C_PIO_PA27
#define STEPPER1_I11 AT91C_PIO_PA28
#define STEPPER1_PHASE1 AT91C_PIO_PA29

/* Common pins */
#define STEPPER_INHIBIT AT91C_PIO_PA7


#define  STEPPER_IOMASK(s) (STEPPER##s##_I00 | STEPPER##s##_I01 | STEPPER##s##_PHASE0 \
			 | STEPPER##s##_I10 | STEPPER##s##_I11 | STEPPER##s##_PHASE1)
#define STEPPER_PHASE(s,p,l) ((((l) < 2) ? (STEPPER##s##_I##p##1) : 0) | (((l) & 1) ? 0 : (STEPPER##s##_I##p##0)))

#define STEPPER_STEP(s, l0, l1) (\
((l0 >= 0) ? STEPPER_PHASE(s,0, l0):(STEPPER_PHASE(s,0, -(l0))|STEPPER##s##_PHASE0)) | \
((l1 >= 0) ? STEPPER_PHASE(s,1, l1):(STEPPER_PHASE(s,1,-(l1))|STEPPER##s##_PHASE1)))


#define FULL_STEP_BOTH(s,l) {\
  STEPPER_STEP(s,(l),(l)), \
  STEPPER_STEP(s,(l),-(l)), \
  STEPPER_STEP(s,-(l),-(l)), \
  STEPPER_STEP(s,-(l),(l))}


#define FULL_STEP_SINGLE(s,l) {\
  STEPPER_STEP(s,0,(l)), \
  STEPPER_STEP(s,(l),0), \
  STEPPER_STEP(s,0,-(l)), \
  STEPPER_STEP(s,-(l),0)}

#define HALF_STEP(s,l) {\
  STEPPER_STEP(s,0,(l)), \
  STEPPER_STEP(s,(l),(l)), \
  STEPPER_STEP(s,(l),0), \
  STEPPER_STEP(s,(l),-(l)), \
  STEPPER_STEP(s,0,-(l)), \
  STEPPER_STEP(s,-(l),-(l)), \
  STEPPER_STEP(s,-(l),0), \
  STEPPER_STEP(s,-(l),(l))}


#define MINI_STEP(s,l) {\
  STEPPER_STEP(s,0,(l)), \
  STEPPER_STEP(s,1,MIN((l),2)), \
  STEPPER_STEP(s,MIN((l),2),1), \
  STEPPER_STEP(s,(l),0), \
  STEPPER_STEP(s,MIN((l),2),-1), \
  STEPPER_STEP(s,1,-MIN((l),2)), \
  STEPPER_STEP(s,0,-(l)), \
  STEPPER_STEP(s,-1,-MIN((l),2)), \
  STEPPER_STEP(s,-MIN((l),2),-1), \
  STEPPER_STEP(s,-(l),0), \
  STEPPER_STEP(s,-MIN((l),2),1), \
  STEPPER_STEP(s,-1,MIN((l),2))}

#define MICRO_STEP(s,l) {\
  STEPPER_STEP(s,0,(l)), \
  STEPPER_STEP(s,1,(l)), \
  STEPPER_STEP(s,MIN((l),2),(l)), \
  STEPPER_STEP(s,(l),(l)), \
  STEPPER_STEP(s,(l),MIN((l),2)), \
  STEPPER_STEP(s,(l),1), \
  STEPPER_STEP(s,(l),0), \
  STEPPER_STEP(s,(l),-1), \
  STEPPER_STEP(s,(l),-MIN((l),2)), \
  STEPPER_STEP(s,(l),-(l)), \
  STEPPER_STEP(s,MIN((l),2),-(l)), \
  STEPPER_STEP(s,1,-(l)), \
  STEPPER_STEP(s,0,-(l)), \
  STEPPER_STEP(s,-1,-(l)), \
  STEPPER_STEP(s,-MIN((l),2),-(l)), \
  STEPPER_STEP(s,-(l),-(l)), \
  STEPPER_STEP(s,-(l),-MIN((l),2)), \
  STEPPER_STEP(s,-(l),-1), \
  STEPPER_STEP(s,-(l),0), \
  STEPPER_STEP(s,-(l),1), \
  STEPPER_STEP(s,-(l),MIN((l),2)), \
  STEPPER_STEP(s,-(l),(l)), \
  STEPPER_STEP(s,-MIN((l),2),(l)), \
  STEPPER_STEP(s,-1,(l))}

#define STEPPER_ENABLE() (*AT91C_PIOA_SODR = STEPPER_INHIBIT)
#define STEPPER_DISABLE() (*AT91C_PIOA_CODR = STEPPER_INHIBIT)
