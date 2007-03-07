#ifndef __STEPPER3_INTERRUPT_H__2MHD6D6PQ1__
#define __STEPPER3_INTERRUPT_H__2MHD6D6PQ1__

#include <stepper.h>

/* Timer frequency */
#define TIMER_FREQ 748800

typedef struct _StepperContext StepperContext;
typedef struct _StepperState StepperState;
typedef struct _StepperTimerStep StepperTimerStep;

#define MAX_STEPS_PER_PERIOD 40
#define NUM_STEPPERS 2

#define STEPPER_MAX_VELOCITY 4000
#define STEPPER_MAX_ACCELRATION 4000


#define TIMING_ERRORS

struct _StepperState
{
  long step_count;
  uint32_t io_mask;
  const uint32_t *acc_steps; /* Stepping sequence when accelerating */
  const uint32_t *run_steps; /* Stepping sequence when running */
  const uint32_t *hold_steps; /* Stepping sequence when stationary */
  uint8_t current_step;	/* in stepping sequence */
  uint8_t sequence_length;

  long velocity;	/* steps/second  * PPS */
  long acceleration;	/* steps/second^2 */
  long step_full;	/* steps, same as step_count at period boundaries */
  long step_frac;	/* (steps * PPS^2 * 2) % steps * PPS^2 */
  
  long n_steps;		/* full steps to move during this period */
  
  StepperAccSeq *acceleration_sequence;

#ifdef TIMING_ERRORS
  long err_max;
  long err_min;
#endif
};

#define STEPPER_POWER_ACC 30
#define STEPPER_POWER_RUN 20
#define STEPPER_POWER_HOLD 10
#define STEPPER_POWER_OFF 0

#define STEPPER_DIRECTION_NONE 0
#define STEPPER_DIRECTION_FORWARD 1
#define STEPPER_DIRECTION_BACKWARD 2

struct _StepperTimerStep
{
  StepperTimerStep *next;
  StepperState *state;
  uint16_t time;
  uint8_t direction;
  uint8_t power;
};


struct _StepperContext
{
  unsigned int flags;
  unsigned long period_count;
  AT91PS_TC timer_channel;
  StepperState steppers[NUM_STEPPERS];
  StepperTimerStep *steps;
  StepperTimerStep *current_step;
  StepperUserCallback user_callback;
};

extern StepperContext stepper_context;
#endif /* __STEPPER3_INTERRUPT_H__2MHD6D6PQ1__ */

void stepper_timer_interrupt(void);
