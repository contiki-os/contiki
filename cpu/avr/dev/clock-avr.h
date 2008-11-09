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

#elif defined (__AVR_ATmega1284P__) || (__AVR_AT90USB1287__) || (__AVR_ATmega1281__)

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
   * pre-scale factor is 1024, i.e. we have 125 "ticks" / sec: \
   * 8000000 = 1024 * 125 * 62.5 \
   */ \
  OCR0A = 62; \
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

#define AVR_OUTPUT_COMPARE_INT TIMER0_COMPA_vect

#else
#error "Setup CPU in clock-avr.h"
#endif

#endif //CONTIKI_CLOCK_AVR_H
