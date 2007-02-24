#ifndef __STEPPER3_INTERRUPT_H__2MHD6D6PQ1__
#define __STEPPER3_INTERRUPT_H__2MHD6D6PQ1__

#include <AT91SAM7S64.h>
#include <inttypes.h>

/* Define periods/second */
#define PPS 128

/* Scaling factor for distance */
#define DIST_SCALE (2 * PPS * PPS)

/* Scaling factor for velocity */
#define VEL_SCALE PPS

typedef struct _StepperContext StepperContext;
typedef struct _StepperState StepperState;
typedef struct _StepperTimerStep StepperTimerStep;
typedef struct _StepperAccSeq StepperAccSeq;

#define MAX_STEPS_PER_PERIOD 40
#define NUM_STEPPERS 2

#define STEPPER_MAX_VELOCITY 4000
#define STEPPER_MAX_ACCELRATION 4000

struct _StepperAccSeq
{
  StepperAccSeq *next;
  unsigned long period;
  long acceleration;
};

#define STEPPER_ACC_INVALID LONG_MAX

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

typedef void (*StepperUserCallback)(unsigned int stepper_index,
				    unsigned long period);

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

typedef unsigned int StepperResult;
#define STEPPER_OK 0
#define STEPPER_ERR_MEM 1
#define STEPPER_ERR_TOO_LATE 2
#define STEPPER_ERR_INDEX 3

void
stepper_init(AT91PS_TC timer, unsigned int id);

void
stepper_init_io(unsigned int stepper_index, uint32_t mask,
		const uint32_t *acc, const uint32_t *run,
		const uint32_t *hold, unsigned int nsteps);

/* Returns true if the new sequence was actually added or false
   if the index is illegal or the first step in the sequence is too soon */

StepperResult
stepper_add_acc_seq(unsigned int stepper_index, StepperAccSeq *new_seq);

StepperResult
stepper_add_acc(unsigned int stepper_index, unsigned int period, long acc);

StepperResult
stepper_insert_callback(unsigned int stepper_index, unsigned int period);

void
stepper_set_callback_proc(StepperUserCallback callback);

unsigned long
stepper_current_period();

long
stepper_current_step(unsigned int stepper_index);

long long
stepper_step_frac(unsigned int stepper_index);

long
stepper_current_velocity(unsigned int stepper_index);


unsigned long
stepper_velocity(unsigned int stepper_index, unsigned long period);

StepperResult
stepper_state_at(unsigned int stepper_index, unsigned long period,
		 long *velocity, long long *position);

StepperResult
stepper_set_velocity(unsigned int stepper_index, unsigned long *periodp,
		     unsigned long max_acc, long final_speed);

StepperAccSeq *
stepper_allocate_seq();

void
stepper_free_seq(StepperAccSeq *seq);

#ifdef TIMING_ERRORS
void
stepper_timing_errors(unsigned int stepper_index, long *min, long *max);
#endif

#endif /* __STEPPER3_INTERRUPT_H__2MHD6D6PQ1__ */
