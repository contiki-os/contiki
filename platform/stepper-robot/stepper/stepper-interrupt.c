#include <stepper-interrupt.h>
#include <interrupt-utils.h>
#include <stdio.h>


/* Timer frequency */
#define TIMER_FREQ 748800

static StepperContext stepper_context;

static StepperAccSeq *free_seq = NULL;

StepperAccSeq *
stepper_allocate_seq()
{
  StepperAccSeq *seq;
  if (!free_seq) return NULL;
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  seq = free_seq;
  free_seq = seq->next;
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  return seq;
}

void
stepper_free_seq(StepperAccSeq *seq)
{
  StepperAccSeq *s;
  if (!seq) return;
  s = seq;
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  while(s->next) s = s->next;
  s->next = free_seq;
  free_seq = seq;
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
}

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

void NACKEDFUNC stepper_int (void) {
  ISR_STORE();
  ISR_ENABLE_NEST();
  stepper_int_safe();
  ISR_DISABLE_NEST();
  ISR_RESTORE();
}

static void
stepper_state_init(StepperState *stepper)
{
  stepper->step_count = 0;
  stepper->io_mask = 0;
  stepper->acc_steps = NULL;
  stepper->run_steps = NULL;
  stepper->hold_steps = NULL;
  stepper->current_step = 0;
  stepper->sequence_length = 0;

  stepper->velocity = 0;
  stepper->acceleration = 0;
  stepper->step_full = 0;
  stepper->step_frac = 0;
  stepper->n_steps = 0;

#ifdef TIMING_ERRORS
  stepper->err_min = TIMER_FREQ;
  stepper->err_max = -TIMER_FREQ;
#endif
  
}

void
stepper_init(AT91PS_TC timer, unsigned int id)
{
  unsigned int s;
  stepper_context.flags = 0;
  stepper_context.timer_channel = timer;
  stepper_context.steps = NULL;
  stepper_context.current_step = NULL;
  stepper_context.period_count = 0;
  stepper_context.user_callback = NULL;
  
  for (s = 0; s < NUM_STEPPERS; s++) {
    stepper_state_init(&stepper_context.steppers[s]);
  }
  timer->TC_CMR = (AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO
		   | AT91C_TC_CLKS_TIMER_DIV3_CLOCK);
  timer->TC_RC = TIMER_FREQ / PPS;
  timer->TC_RA = 0xffff;
  timer->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  *AT91C_PMC_PCER = (1 << id);
  
  AT91C_AIC_SMR[id] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 7;
  AT91C_AIC_SVR[id] =  (unsigned long)stepper_int;
  *AT91C_AIC_IECR = (1 << id);
  timer->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
}

void
stepper_init_io(unsigned int stepper_index, uint32_t mask,
		const uint32_t *acc, const uint32_t *run,
		const uint32_t *hold, unsigned int nsteps)
{
  StepperState *state;
  if (stepper_index >= NUM_STEPPERS) return;
  state = &stepper_context.steppers[stepper_index];
  
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  
  state->io_mask = mask;
  state->acc_steps = acc;
  state->run_steps = run;
  state->hold_steps = hold;
  state->current_step = 0;
  state->sequence_length = nsteps;
  *AT91C_PIOA_OWER = mask;
  *AT91C_PIOA_MDDR = mask;

  *AT91C_PIOA_ODSR = ((*AT91C_PIOA_ODSR & ~mask)
		      | (state->hold_steps[0] & mask));
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  *AT91C_PIOA_OER = mask;
}

/**
   Append an acceleration sequence
   
   Truncates the current acceleration sequence at the insertion time
   and appends the new sequence at that position.. The insertion time
   is the time of the first element of the new sequence. The
   truncation takes place after any elements with the acceleration set
   to STEPPER_ACC_INVALID (user callbacks) that has the same time as
   the insertion time. All other elements with the same time is
   replaced.

   \param stepper_index Index of the stepper the sequence is intended for.
   \param new_seq A linked list of sequence elements to append.
 */
StepperResult
stepper_add_acc_seq(unsigned int stepper_index, StepperAccSeq *new_seq)
{
  StepperResult res = STEPPER_ERR_TOO_LATE;
  StepperAccSeq **seqp;
  StepperState *state;
  if (stepper_index >= NUM_STEPPERS) return STEPPER_ERR_INDEX;
  state = &stepper_context.steppers[stepper_index];
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  seqp = &state->acceleration_sequence;
  while(*seqp && ((*seqp)->period < new_seq->period || ((*seqp)->period == new_seq->period && (*seqp)->acceleration == STEPPER_ACC_INVALID))) {
    seqp = &(*seqp)->next;
  }
  if (new_seq->period > stepper_context.period_count + 1) {
    /* Replace tail of sequence */
    if (*seqp) stepper_free_seq(*seqp);
    *seqp = new_seq;
    res = STEPPER_OK;
  }
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  return res;
}
  
/**
   Insert a callback mark
   
   Inserts a callback mark at the indicated period. This will cause
   the callback procedure to be called just before that period,
   usually near the beginning of the previous period. Does not
   truncate the current sequence.

   \param stepper_index Index of the stepper the callbak is intended for.
   \param period When the callback should be invoked

   \sa stepper_set_callback_proc
*/
  
StepperResult
stepper_insert_callback(unsigned int stepper_index, unsigned int period)
{
  StepperResult res = STEPPER_ERR_TOO_LATE;
  StepperAccSeq **seqp;
  StepperState *state;
  if (stepper_index >= NUM_STEPPERS) return STEPPER_ERR_INDEX;
  state = &stepper_context.steppers[stepper_index];
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  seqp = &state->acceleration_sequence;
  while(*seqp && (*seqp)->period < period) {
    seqp = &(*seqp)->next;
  }
  if (period > stepper_context.period_count + 1) {
    StepperAccSeq *new_seq = stepper_allocate_seq();
    if (!new_seq) {
      res = STEPPER_ERR_MEM;
    } else {
      new_seq->next = *seqp;
      *seqp = new_seq;
      new_seq->period = period;
      new_seq->acceleration = STEPPER_ACC_INVALID;
      res = STEPPER_OK;
    }
  }
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  return res;
}

StepperResult
stepper_add_acc(unsigned int stepper_index, unsigned int period, long acc)
{
  StepperAccSeq *seq = stepper_allocate_seq();
  /* printf("stepper_add_acc: %d %d %ld\n", stepper_index, period, acc); */
  if (!seq) return STEPPER_ERR_MEM;
  seq->next = NULL;
  seq->period = period;
  seq->acceleration = acc;
  return stepper_add_acc_seq(stepper_index, seq);
}

void
stepper_set_callback_proc(StepperUserCallback callback)
{
  stepper_context.user_callback = callback;
}

unsigned long
stepper_current_period()
{
  return stepper_context.period_count;
}

long
stepper_current_step(unsigned int stepper_index)
{
  StepperState *state = &stepper_context.steppers[stepper_index];
  return state->step_count;
}

long long
stepper_step_frac(unsigned int stepper_index)
{
  long long s;
  StepperState *state = &stepper_context.steppers[stepper_index];
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  s = state->step_full * DIST_SCALE + state->step_frac;
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  return s;
}

long
stepper_current_velocity(unsigned int stepper_index)
{
  StepperState *state = &stepper_context.steppers[stepper_index];
  return state->velocity;
}

/* Calculate the speed at given current given the current acceleration
   sequence. */
unsigned long
stepper_velocity(unsigned int stepper_index, unsigned long period)
{
  long a;
  long v;
  unsigned long t;
  StepperState *state;
  StepperAccSeq *seq;
  state = &stepper_context.steppers[stepper_index];
  
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  seq = state->acceleration_sequence;
  a = state->acceleration;
  v = state->velocity;
  t = stepper_context.period_count + 1;

  while(seq && seq->period < period) {
    v += a * (seq->period - t);
    t = seq->period;
    a = seq->acceleration;
    seq = seq->next;
  }
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  v += a * (period - t);
  return v;
}

/**
   Calculate the speed and position at specified period given the
   current acceleration sequence.

   \param stepper_index Index of the stepper the callbak is intended for.
   \param period The period to calculate for
   \param Speed return
   \param Position return. In fractional steps

*/
StepperResult
stepper_state_at(unsigned int stepper_index, unsigned long period,
		 long *velocity, long long *position)
{
  long a;
  long v;
  long long s;
  unsigned long t;
  long dt;
  StepperState *state;
  StepperAccSeq *seq;
  state = &stepper_context.steppers[stepper_index];
  
  stepper_context.timer_channel->TC_IDR = AT91C_TC_CPCS | AT91C_TC_CPAS;
  if (period < stepper_context.period_count + 2) {
    stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
    return STEPPER_ERR_TOO_LATE;
  }
  seq = state->acceleration_sequence;
  a = state->acceleration;
  v = state->velocity;
  t = stepper_context.period_count + 1;
  s = state->step_full * (long long)DIST_SCALE + state->step_frac;
  while(seq && seq->period < period) {
    dt = seq->period - t;
    s += (a * (long long)dt + 2 * v) * dt;
    v += a * (seq->period - t);
    t = seq->period;
    a = seq->acceleration;
    seq = seq->next;
  }
  stepper_context.timer_channel->TC_IER = AT91C_TC_CPCS | AT91C_TC_CPAS;
  dt = period - t;
  *position = s + (a * (long long)dt + (DIST_SCALE/VEL_SCALE) * v) * dt;
  *velocity = v + a * dt;
  
  return STEPPER_OK;
}


StepperResult
stepper_set_velocity(unsigned int stepper_index, unsigned long *periodp,
		     unsigned long max_acc, long final_speed)
{
  long start_period = *periodp;
  long v = stepper_velocity(stepper_index, start_period);
  if (final_speed == v) {
    return stepper_add_acc(stepper_index, start_period, 0);
  } else {
    StepperResult res;
    long a = (final_speed > v) ? max_acc : -max_acc;
    long t = ((long)(final_speed - v)) / a;
    long diff = (final_speed - v) - t * a;
    if (t > 0) {
      res = stepper_add_acc(stepper_index, start_period, a);
      if (res != STEPPER_OK) return res;
    }
    if (diff) {
      res = stepper_add_acc(stepper_index, start_period+t, diff);
      if (res != STEPPER_OK) return res;
      t++;
    }
    *periodp = start_period+t;
    return stepper_add_acc(stepper_index, start_period+t, 0);
  }
}

#ifdef TIMING_ERRORS
void
stepper_timing_errors(unsigned int stepper_index, long *min, long *max)
{
  StepperState *state;
  state = &stepper_context.steppers[stepper_index];
  *min = state->err_min;
  *max = state->err_max;
  state->err_max = -TIMER_FREQ;
  state->err_min = TIMER_FREQ;
}
#endif
