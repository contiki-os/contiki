#ifndef CONTIKI_CLOCK_AVR_H
#define CONTIKI_CLOCK_AVR_H

#if defined (__AVR_ATmega128__)

#define AVR_OUTPUT_COMPARE_INT TIMER0_COMP_vect

#define OCRSetup() \
  /* Select internal clock */ \
  ASSR = 0x00; 				  \
\
  /* Set counter to zero */   \
  TCNT0 = 0;				  \
\
  /*						  \
   * Set comparison register: \
   * Crystal freq. is 16000000,\
   * pre-scale factor is 1024, i.e. we have 125 "ticks" / sec: \
   * 16000000 = 1024 * 125 * 125 \
   */ \
  OCR0 = 125; \
\
  /* 								\
   * Set timer control register: 	\
   *  - prescale: 1024 (CS00 - CS02) \
   *  - counter reset via comparison register (WGM01) \
   */ 								\
  TCCR0 =  _BV(CS00) | _BV(CS01) |  _BV(CS02) |  _BV(WGM01); \
\
  /* Clear interrupt flag register */ \
  TIFR = 0x00; \
\
  /* \
   * Raise interrupt when value in OCR0 is reached. Note that the \
   * counter value in TCNT0 is cleared automatically. \
   */ \
  TIMSK = _BV (OCIE0);

#elif defined (__AVR_ATmega128RFA1__) && 0

#define AVR_OUTPUT_COMPARE_INT TIMER0_COMPA_vect
#define OCRSetup() \
  /* Select internal clock */ \
  ASSR = 0x00; 				  \
\
  /* Set counter to zero */   \
  TCNT0 = 0;				  \
\
  /*						  \
   * Set comparison register: \
   * Crystal freq. is 8000000,\
   * pre-scale factor is 1024, we want 125 ticks / sec: \
   * 8000000 = 1024 * 126.01 * 62, less 1 for CTC mode \
   */ \
  OCR0A = 61; \
\
  /* 								\
   * Set timer control register: 	\
   *  - prescale: 1024 (CS00 - CS02) \
   *  - counter reset via comparison register (WGM01) \
   */ 								\
  TCCR0A = _BV(WGM01); \
  TCCR0B =  _BV(CS00) | _BV(CS02); \
\
  /* Clear interrupt flag register */ \
  TIFR0 = TIFR0; \
\
  /* \
   * Raise interrupt when value in OCR0 is reached. Note that the \
   * counter value in TCNT0 is cleared automatically. \
   */ \
  TIMSK0 = _BV (OCIE0A);

  
#elif defined (__AVR_ATmega1284P__) || (__AVR_AT90USB1287__) || (__AVR_ATmega1281__) || defined (__AVR_ATmega128RFA1__)
/*
  The Raven has a 32768Hz watch crystal that can be used to clock the timer
  while the 1284p is sleeping. The Jackdaw has pads for a crystal. The crystal
  can be used to clock the 8 bit timer2.
  The 1284p routine also uses TIMER2 to sleep a variable number of seconds.
  It restores the values here after a wake.
*/
#if AVR_CONF_USE32KCRYSTAL
#define AVR_OUTPUT_COMPARE_INT TIMER2_COMPA_vect
#define OCRSetup() \
  /* Clock from crystal on TOSC0-1 */ \
  ASSR = _BV(AS2);		      \
\
  /* Set counter to zero */   \
  TCNT2 = 0;				  \
\
  /*						  \
   * Set comparison register: \
   * Crystal freq. is 32768,\
   * pre-scale factor is 8, we want 125 ticks / sec: \
   * 32768 = 8 * 124.1 * 33, less 1 for CTC mode\
   */ \
  OCR2A = 32; \
\
  /* 								\
   * Set timer control register: 	\
   *  - prescale: 8 (CS21) \
   *  - counter reset via comparison register (WGM21) \
   */ 								\
  TCCR2A = _BV(WGM21); \
  TCCR2B = _BV(CS21);  \
\
  /* Clear interrupt flag register */ \
  TIFR2 = TIFR2; \
\
  /* \
   * Raise interrupt when value in OCR2 is reached. Note that the \
   * counter value in TCNT2 is cleared automatically. \
   */ \
  TIMSK2 = _BV (OCIE2A);
#else
#define AVR_OUTPUT_COMPARE_INT TIMER0_COMPA_vect
#define OCRSetup() \
  /* Select internal clock */ \
  ASSR = 0x00; 				  \
\
  /* Set counter to zero */   \
  TCNT0 = 0;				  \
\
  /*						  \
   * Set comparison register: \
   * Crystal freq. is 8000000,\
   * pre-scale factor is 1024, we want 125 ticks / sec: \
   * 8000000 = 1024 * 126.01 * 62, less 1 for CTC mode \
   */ \
  OCR0A = 61; \
\
  /* 								\
   * Set timer control register: 	\
   *  - prescale: 1024 (CS00 - CS02) \
   *  - counter reset via comparison register (WGM01) \
   */ 								\
  TCCR0A = _BV(WGM01); \
  TCCR0B =  _BV(CS00) | _BV(CS02); \
\
  /* Clear interrupt flag register */ \
  TIFR0 = TIFR0; \
\
  /* \
   * Raise interrupt when value in OCR0 is reached. Note that the \
   * counter value in TCNT0 is cleared automatically. \
   */ \
  TIMSK0 = _BV (OCIE0A);
#endif /* AVR_CONF_USE32KCRYSTAL */

#else
#error "Setup CPU in clock-avr.h"
#endif

#endif //CONTIKI_CLOCK_AVR_H
