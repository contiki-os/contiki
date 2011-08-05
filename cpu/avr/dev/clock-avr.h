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
   * Crystal freq. is F_CPU,\
   * pre-scale factor is 1024, we want CLOCK_CONF_SECOND ticks / sec: \
   * F_CPU = 1024 * CLOCK_CONF_SECOND * OCR0 \
   */ \
  OCR0 = F_CPU/1024UL/CLOCK_CONF_SECOND; \
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
   * Crystal freq. is F_CPU,\
   * pre-scale factor is 1024, we want CLOCK_CONF_SECOND ticks / sec: \
   * F_CPU = 1024 * CLOCK_CONF_SECOND * OCR0A, less 1 for CTC mode \
   */ \
  OCR0A = F_CPU/1024/CLOCK_CONF_SECOND - 1; \
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
   * pre-scale factor is 8, we want CLOCK_CONF_SECOND ticks / sec: \
   * 32768 = 8 * CLOCK_CONF_SECOND * OCR2A, less 1 for CTC mode\
   */ \
  OCR2A = 32768/8/CLOCK_CONF_SECOND - 1; \
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
   * Crystal freq. is F_CPU,\
   * pre-scale factor is 1024, we want CLOCK_CONF_SECOND ticks / sec: \
   * F_CPU = 1024 * CLOCK_CONF_SECOND * OCR2A, less 1 for CTC mode \
   */ \
  OCR0A = F_CPU/1024UL/CLOCK_CONF_SECOND - 1; \
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

#elif defined (__AVR_ATmega644__) || defined (__AVR_ATmega328P__)

#define OCRSetup() \
  /* Set counter to zero */   \
  TCNT0 = 0;   \
\
  /*   \
   * Set comparison register: \
   * Crystal freq. is F_CPU,\
   * pre-scale factor is 256, want CLOCK_CONF_SECOND ticks / sec: \
   */ \
  OCR0A = F_CPU/256UL/CLOCK_CONF_SECOND - 1; \
\
  /* \
   * Set timer control register: \
   *  - prescale: 256 (CS02) \
   *  - counter reset via comparison register (WGM01) \
   */ \
  TCCR0A =  _BV(WGM01); \
  TCCR0B =  _BV(CS02); \
\
  /* Clear interrupt flag register */ \
  TIFR0 = 0x00; \
\
  /* \
   * Raise interrupt when value in OCR0 is reached. Note that the \
   * counter value in TCNT0 is cleared automatically. \
   */ \
  TIMSK0 = _BV (OCIE0A);

#define AVR_OUTPUT_COMPARE_INT TIMER0_COMPA_vect

#elif defined (__AVR_ATmega8515__) || defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)

#define AVR_OUTPUT_COMPARE_INT TIMER0_COMP_vect

#define OCRSetup() \
  /* Set counter to zero */   \
  TCNT0 = 0;   \
\
  /*   \
   * Set comparison register: \
   * Crystal freq. is F_CPU,\
   * pre-scale factor is 256, we want CLOCK_CONF_SECOND ticks / sec: \
   * F_CPU = 256 * CLOCK_CONF_SECOND * OCR0 \
   */ \
  OCR0 = F_CPU/256UL/CLOCK_CONF_SECOND; \
\
  /* \
   * Set timer control register: \
   *  - prescale: 256 (CS02) \
   *  - counter reset via comparison register (WGM01) \
   */ \
  TCCR0 =  _BV(CS02) |  _BV(WGM01); \
\
  /* Clear interrupt flag register */ \
  TIFR = 0x00; \
\
  /* \
   * Raise interrupt when value in OCR0 is reached. Note that the \
   * counter value in TCNT0 is cleared automatically. \
   */ \
  TIMSK = _BV (OCIE0);

#elif defined (__AVR_ATmega8__)

#define AVR_OUTPUT_COMPARE_INT TIMER2_COMP_vect

#define OCRSetup() \
  /* Set counter to zero */   \
  TCNT2 = 0;   \
\
  /*   \
   * Set comparison register: \
   * Crystal freq. is F_CPU,\
   * pre-scale factor is 256, we want CLOCK_CONF_SECOND ticks / sec: \
   * F_CPU = 256 * CLOCK_CONF_SECOND * OCR2 \
   */ \
  OCR2 = F_CPU/256UL/CLOCK_CONF_SECOND; \
\
  /* \
   * Set timer control register: \
   *  - prescale: 256 (CS21 CS22) \
   *  - counter reset via comparison register (WGM21) \
   */ \
  TCCR2 =  _BV(CS22) | _BV(CS21) |  _BV(WGM21); \
\
  /* Clear interrupt flag register */ \
  TIFR = 0x00; \
\
  /* \
   * Raise interrupt when value in OCR2 is reached. Note that the \
   * counter value in TCNT2 is cleared automatically. \
   */ \
  TIMSK = _BV (OCIE2);
#else
#error "Setup CPU in clock-avr.h"
#endif

#endif //CONTIKI_CLOCK_AVR_H
