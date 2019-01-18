
#include "sys/clock.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile clock_time_t count;

/*---------------------------------------------------------------------------*/
SIGNAL(SIG_OUTPUT_COMPARE0)
{
  ++count;
  if(etimer_pending()) {
    etimer_request_poll();
  }
}

/* External clock source does not work ? */
#if 0
void
clock_init(void)
{
  cli ();
  TIMSK &= ((unsigned char)~(1 << (TOIE0)));
  TIMSK &= ((unsigned char)~(1 << (OCIE0)));
  /* Disable TC0 interrupt */

  /** 
   *  set Timer/Counter0 to be asynchronous 
   *  from the CPU clock with a second external 
   *  clock(32,768kHz)driving it
   */
  ASSR |= (1 << (AS0));
  TCCR0 = _BV (CS02) | _BV (CS01) | _BV (WGM1);

  TCNT0 = 0;
  OCR0 = 128;
  
  TIMSK |= (1 << (OCIE0));
  TIMSK |= (1 << (TOIE0));
  sei ();
}
#endif

/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  cli ();

  /* Select internal clock */
  ASSR = 0x00;

  /* Set counter to zero */
  TCNT0 = 0;

  /*
   * Set comparison register: 
   * Crystal freq. is 16000000,
   * pre-scale factor is 1024, i.e. we have 125 "ticks" / sec:
   * 16000000 = 1024 * 125 * 125
   */
  OCR0 = 125;
  
  /*
   * Set timer control register:
   *  - prescale: 1024 (CS00 - CS02)
   *  - counter reset via comparison register (WGM01)
   */
  TCCR0 =  _BV(CS00) | _BV(CS01) |  _BV(CS02) |  _BV(WGM01);
  
  /* Clear interrupt flag register */
  TIFR = 0x00;

  /*
   * Raise interrupt when value in OCR0 is reached. Note that the
   * counter value in TCNT0 is cleared automatically.
   */
  TIMSK = _BV (OCIE0);

  /*
   * Counts the number of ticks. Since clock_time_t is an unsigned
   * 16 bit data type, time intervals of up to 524 seconds can be
   * measured.
   */
  count = 0;

  sei ();
}

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of TODO
 */
void
clock_delay(unsigned int i)
{
  for (; i > 0; i--) {		/* Needs fixing XXX */
    unsigned j;
    for (j = 50; j > 0; j--)
      asm volatile("nop");
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 1 / 125 sec = 0.008 ms.
 *
 */
void
clock_wait(int i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
    // TODO
}

unsigned long
clock_seconds(void)
{
  return count / CLOCK_SECOND;
}
