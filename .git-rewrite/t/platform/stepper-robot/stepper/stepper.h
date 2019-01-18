#ifndef __STEPPER_H__JPA916UOFT__
#define __STEPPER_H__JPA916UOFT__

#include <AT91SAM7S64.h>
#include <inttypes.h>
#include <limits.h>

/* Define periods/second */
#define PPS 128

/* Scaling factor for distance */
#define DIST_SCALE (2 * PPS * PPS)

/* Scaling factor for velocity */
#define VEL_SCALE PPS

typedef struct _StepperAccSeq StepperAccSeq;
struct _StepperAccSeq
{
  StepperAccSeq *next;
  unsigned long period;
  long acceleration;
};

#define STEPPER_ACC_INVALID LONG_MAX

#define STEPPER_MAX_VELOCITY 4000
#define STEPPER_MAX_ACCELRATION 4000

typedef void (*StepperUserCallback)(unsigned int stepper_index,
				    unsigned long period);


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

#endif /* __STEPPER_H__JPA916UOFT__ */
