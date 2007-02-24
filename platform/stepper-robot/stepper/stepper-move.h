#ifndef __STEPPER_MOVE_H__9UFUHMYMYS__
#define __STEPPER_MOVE_H__9UFUHMYMYS__

StepperResult
stepper_move(unsigned int stepper_index, unsigned long *periodp,
	     unsigned long a_max,unsigned long v_max, long s_end);

#endif /* __STEPPER_MOVE_H__9UFUHMYMYS__ */
