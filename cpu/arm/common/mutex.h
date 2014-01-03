

#ifndef _ARM_MUTEX_H
#define _ARM_MUTEX_H

/*
 * MUTEX implementation
 * Inspired from Linux kernel code and ARM recommendations
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/ch01s03s02.html
 */


#define MUTEX_UNLOCKED 1
#define MUTEX_LOCKED   0

typedef struct mutex {
	int count;
} mutex_t;


static inline void atomic_set(volatile void *ptr, int i)
{
	unsigned long tmp;

    __asm__ __volatile__("@ atomic_set\n"
  "1:     ldrex   %0, [%1]\n"
  "       strex   %0, %2, [%1]\n"
  "       teq     %0, #0\n"
  "       bne     1b"
          : "=&r" (tmp)
          : "r" (ptr), "r" (i)
          : "cc");
}

static inline unsigned long atomic_xchg(volatile void *ptr, unsigned long new_value)
{
	unsigned long ret;
	unsigned int tmp;

	// Use with ARM version >= v6
    asm volatile("@ atomic_xchg\n"
     "1:     ldrex   %0, [%3]\n"      // Load exclusive *ptr in ret
     "       strex   %1, %2, [%3]\n"  // Store exclusive new_value in *ptr (and get result in tmp)
     "       teq     %1, #0\n"        // Check if Store-Exclusive failed
     "       bne     1b"              // Loop and retry if it failed
             : "=&r" (ret), "=&r" (tmp)
             : "r" (new_value), "r" (ptr)
             : "memory", "cc");

    // Return former value of *ptr ... now it's "new_value" in all cases
    return ret;
}


/**
 * mutex_init
 *
 */
static inline void mutex_init(mutex_t * pmutex)
{
	atomic_set(&pmutex->count,MUTEX_UNLOCKED);
}

/**
 * mutex_lock
 * Lock mutex by moving its count from 1 to 0 (infinite loop)
 *
 */
static inline void mutex_lock(mutex_t * pmutex)
{
	if(atomic_xchg(&pmutex->count,MUTEX_LOCKED) != MUTEX_UNLOCKED)
	{
		/* What to do when it failed and before retrying
		 * 1 - Busy wait on lock
		 * 2 - 'wfi' or 'wfe' wait on lock
		 * 3 - Call scheduler in a process oriented OS
		 *
		 * In our case, change will mostly happen on interrupt or rescheduling
		 * Contiki scheduler is based on systick so we can use wfe (and
		 */
		while(atomic_xchg(&pmutex->count,MUTEX_LOCKED) != MUTEX_UNLOCKED)
		{
			__WFE();
		}
	}

	// Now lock is acquired, call Data Memory barrier before accessing protected ressource
	__DMB();
}

/**
 * mutex_trylock
 * Try to lock mutex by moving its count from 1 to 0
 *
 * \return 1 if successfuly lock up mutex
 *         0 in all others cases
 *
 */
static inline int mutex_trylock(mutex_t * pmutex)
{
	if(atomic_xchg(&pmutex->count,MUTEX_LOCKED) == MUTEX_UNLOCKED)
	{
		// Return 1 if successfully lock mutex
		return 1;
	}

	// Return 0 in all other cases
	return 0;
}


/**
 * mutex_unlock
 * Unlock mutex by moving its count from 0 to 1
 *
 */
static inline void mutex_unlock(mutex_t * pmutex)
{
	// call Data Memory barrier before releasing protected ressource
	__DMB();

	atomic_xchg(&pmutex->count,MUTEX_UNLOCKED);

	__DSB(); // Ensure update has completed before signalling
	__SEV(); // Signal update (warn mutex_lock mutex is free now)
}


#endif
