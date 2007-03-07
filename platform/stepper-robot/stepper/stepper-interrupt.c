#include <stepper-interrupt.h>
#include <interrupt-utils.h>
#include <stdio.h>
#include <stepper.h>



StepperContext stepper_context;


static void
do_step(StepperTimerStep *step)
{
  const uint32_t *io_steps;
  StepperState *state = step->state;

  
  if (step->power >= STEPPER_POWER_ACC) {
    io_steps = state->acc_steps;
  } else if (step->power >= STEPPER_POWER_RUN) {
    io_steps = state->run_steps;
  } else {
    io_steps = state->hold_steps;
  }
  if (io_steps) {
    if (step->direction == STEPPER_DIRECTION_FORWARD){
      state->step_count++;
      /* dbg_putchar('+'); */
      if (++state->current_step == state->sequence_length)
	state->current_step = 0;
    } else {
      state->step_count--;
      /* dbg_putchar('-'); */
      if (state->current_step-- == 0)
	state->current_step = state->sequence_length-1;
    }
    *AT91C_PIOA_ODSR = (*AT91C_PIOA_ODSR & ~state->io_mask)
      | (io_steps[state->current_step] & state->io_mask);
#ifdef TIMING_ERRORS
  {
    long err = ((long)stepper_context.timer_channel->TC_CV - (long)step->time);
    if (err >= (TIMER_FREQ/PPS/2)) {
      err -= TIMER_FREQ/PPS;
    } else if (err < -(TIMER_FREQ/PPS/2)) {
      err += TIMER_FREQ/PPS;
    }
    if (err < state->err_min) state->err_min = err;
    if (err > state->err_max) state->err_max = err;
  }
#endif
  }
}

static void
set_hold(StepperState *state) {
  *AT91C_PIOA_ODSR = (*AT91C_PIOA_ODSR & ~state->io_mask)
      | (state->hold_steps[state->current_step] & state->io_mask);
}
static void
advance_step()
{
  StepperTimerStep *current =stepper_context.current_step;
  AT91PS_TC timer = stepper_context.timer_channel;
  unsigned int now = timer->TC_CV;
  while (current && current->time <= now) {
    do_step(current);
    current = current->next;
    if (!current) break;
    timer->TC_RA = current->time;
    now = timer->TC_CV;
  }
  stepper_context.current_step = current;
}


static inline int64_t
mulsu48_16(int64_t a, uint32_t b)
{
  return a*(int64_t)b;
}

/* Find a solution for s = a*t*t +v * t in the interval [t_low, t_high[ */
static unsigned long
solve_dist(long long s, long a, long long v, unsigned long t_low, unsigned long t_high)
{
  long long s_low =  mulsu48_16((a*(long)t_low+ v), t_low);
  long long s_high = mulsu48_16((a*(long)t_high + v), t_high);
  if (s >= s_low && s <= s_high) {
    while(t_low + 2 < t_high) {
      unsigned long t = (t_high + t_low) / 2;
      long long s_mid = mulsu48_16((a*(long)t + v), t);
      if (s < s_mid) {
	t_high = t;
	s_high = s_mid;
      } else {
	t_low = t;
	s_low = s_mid;
      }
    }
  } else {
    while(t_low + 1 < t_high) {
      unsigned long t = (t_high + t_low) / 2;
      long long s_mid = mulsu48_16((a*(long)t + v), t);
      if (s > s_mid) {
	t_high = t;
	s_high = s_mid;
      } else {
	t_low = t;
	s_low = s_mid;
      }
    }
  }
  return (t_high + t_low) / 2;
}


#define HEAP_SIZE 65
static StepperTimerStep step_heap[2][HEAP_SIZE];
static unsigned short heap_pos = 0; /* Next free position in heap */
static unsigned char current_heap = 0;

static StepperTimerStep *
allocate_step()
{
  if (heap_pos >= HEAP_SIZE) return NULL;
  return &step_heap[current_heap][heap_pos++];
}

static void
switch_step_heap()
{
  current_heap ^= 1;
  heap_pos = 0;
}

StepperTimerStep **
insert_step(StepperTimerStep **at, StepperState *state,
	    unsigned int time, uint8_t direction, uint8_t power)
{
  StepperTimerStep *new_step;
  while(*at && (*at)->time <= time) {
    at = &(*at)->next;
  }
  new_step = allocate_step();
  if (!new_step) return at;
  new_step->next = *at;
  new_step->state = state;
  new_step->time = time;
  new_step->direction = direction;
  new_step->power = power; 
  *at = new_step;
  /* dbg_putchar('!'); */
  return &new_step->next;
}

/* Determine suitable power for the current state */
static uint8_t
get_power(StepperState *state)
{
  if (state->acceleration != 0) return STEPPER_POWER_ACC;
  if (state->velocity == 0) return STEPPER_POWER_HOLD;
  return STEPPER_POWER_RUN;
}

#define SQ(x) ((x)*(x))
#define S_SCALING ((2LL*SQ((long long)TIMER_FREQ)) / DIST_SCALE )
#define V_SCALING (2LL*TIMER_FREQ/VEL_SCALE)


static void
step_interval(StepperState *state)
{
  unsigned int i;
  long long v = state->velocity * V_SCALING;
  long long a = state->acceleration;
  unsigned long t = 0;
  StepperTimerStep **at = &stepper_context.steps;
  if (state->n_steps >= 0) {
    long long s = -state->step_frac * S_SCALING;
    for (i = 0; i < state->n_steps; i++) {
      s+= DIST_SCALE * S_SCALING;
      t =  solve_dist(s, a, v, t, TIMER_FREQ/PPS); 
      /* printf("F%ld\n", t); */
      at = insert_step(at, state, t, STEPPER_DIRECTION_FORWARD, get_power(state)); 
    }
  } else {
    long long s = (DIST_SCALE - state->step_frac) * S_SCALING;
    for (i = 0; i < -state->n_steps; i++) {
      s-= DIST_SCALE * S_SCALING;
      t =  solve_dist(s, a, v, t, TIMER_FREQ/PPS);
      /* printf("B%ld\n", t); */
      at = insert_step(at, state, t, STEPPER_DIRECTION_BACKWARD, get_power(state));
    }
  }
}
static void
setup_speed(StepperState *state)
{
  long steps;
  long step_frac;
  /* printf("%ld v= %ld s=%ld\n",stepper_context.period_count, */
/* 	 state->velocity, state->step_frac); */
  step_frac = (state->acceleration + 2 * state->velocity
	       + state->step_frac);
  steps = step_frac / DIST_SCALE;
  step_frac -= steps * DIST_SCALE;
  if (step_frac <0) {
    step_frac += DIST_SCALE;
    steps--;
  }
  
  /* printf("step_frac=%ld (%f) steps=%ld\n",step_frac, */
/* 	 (double)step_frac/(double)(DIST_SCALE), steps); */
  state->n_steps = steps;
  step_interval(state); 
  state->velocity += state->acceleration;
  state->step_frac = step_frac;
  state->step_full += steps;
}

static void
advance_period()
{
  unsigned int s;
  StepperTimerStep *current =stepper_context.current_step;
  /* Do any remaining step */
  while (current) {
    do_step(current);
    current = current->next;
  }
  /* Start from the beginning */
  stepper_context.current_step = stepper_context.steps;
  stepper_context.steps = NULL;
  if (stepper_context.current_step) {
    stepper_context.timer_channel->TC_RA = stepper_context.current_step->time;
  } else {
    stepper_context.timer_channel->TC_RA = 0xffff;
  }
  /* In case there is a step very early in the period */
  advance_step();
  stepper_context.period_count++;
  *AT91C_AIC_EOICR = 0;
  for(s = 0; s < NUM_STEPPERS; s++) {
    StepperState *state = &stepper_context.steppers[s];
    StepperAccSeq *acc_seq;
    if (state->acceleration == 0 && state->velocity == 0) {
      /* Set hold power if stationary */
      stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
      set_hold(state);
      stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
    }
    while ((acc_seq = state->acceleration_sequence)
	   && acc_seq->period == stepper_context.period_count + 1) {
      state->acceleration_sequence = acc_seq->next;
      if (acc_seq->acceleration == STEPPER_ACC_INVALID) {
	if (stepper_context.user_callback) {
	  stepper_context.user_callback(s, stepper_context.period_count);
	}
      } else {
	state->acceleration = acc_seq->acceleration;
      }
      acc_seq->next = NULL; /* Only free this one */
      stepper_free_seq(acc_seq);
    }
    setup_speed(&stepper_context.steppers[s]);
  }
  /* Prepare heap for next period */
  switch_step_heap();
}

/* Here we have a proper stack frame and can use local variables */
static void stepper_int_safe() __attribute((noinline));
static void
stepper_int_safe()
{
  unsigned int status;
  status = stepper_context.timer_channel->TC_SR;
  if (status & AT91C_TC_CPAS) {
    advance_step();
    /* dbg_putchar('*'); */
  }
  if (status & AT91C_TC_CPCS) {
    advance_period();
  } else {
    *AT91C_AIC_EOICR = 0;                   /* End of Interrupt */
  }
  
}

void NACKEDFUNC stepper_timer_interrupt (void) {
  ISR_STORE();
  ISR_ENABLE_NEST();
  stepper_int_safe();
  ISR_DISABLE_NEST();
  ISR_RESTORE();
}
