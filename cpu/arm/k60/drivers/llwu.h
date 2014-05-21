/*
 * llwu.h
 *
 *  Created on: Mar 19, 2014
 *      Author: henmak
 */

#ifndef LLWU_H_
#define LLWU_H_

enum LLWU_WAKEUP_SOURCE
{
  /*********** LLWU_PE1 ******************/
  LLWU_WAKEUP_SOURCE_P0_RISING = 1,
  LLWU_WAKEUP_SOURCE_P0_FALLING = 2,
  LLWU_WAKEUP_SOURCE_P0_ANY = 3,

  LLWU_WAKEUP_SOURCE_P1_RISING = 4,
  LLWU_WAKEUP_SOURCE_P1_FALLING = 5,
  LLWU_WAKEUP_SOURCE_P1_ANY = 6,

  LLWU_WAKEUP_SOURCE_P2_RISING = 7,
  LLWU_WAKEUP_SOURCE_P2_FALLING = 8,
  LLWU_WAKEUP_SOURCE_P2_ANY = 9,

  LLWU_WAKEUP_SOURCE_P3_RISING = 10,
  LLWU_WAKEUP_SOURCE_P3_FALLING = 11,
  LLWU_WAKEUP_SOURCE_P3_ANY = 12,
  /*********** LLWU_PE2 ******************/
  LLWU_WAKEUP_SOURCE_P4_RISING = 13,
  LLWU_WAKEUP_SOURCE_P4_FALLING = 14,
  LLWU_WAKEUP_SOURCE_P4_ANY = 15,

  LLWU_WAKEUP_SOURCE_P5_RISING = 16,
  LLWU_WAKEUP_SOURCE_P5_FALLING = 17,
  LLWU_WAKEUP_SOURCE_P5_ANY = 18,

  LLWU_WAKEUP_SOURCE_P6_RISING = 19,
  LLWU_WAKEUP_SOURCE_P6_FALLING = 20,
  LLWU_WAKEUP_SOURCE_P6_ANY = 21,

  LLWU_WAKEUP_SOURCE_P7_RISING = 22,
  LLWU_WAKEUP_SOURCE_P7_FALLING = 23,
  LLWU_WAKEUP_SOURCE_P7_ANY = 24,
  /*********** LLWU_PE3 ******************/
  LLWU_WAKEUP_SOURCE_P8_RISING = 25,
  LLWU_WAKEUP_SOURCE_P8_FALLING = 26,
  LLWU_WAKEUP_SOURCE_P8_ANY = 27,

  LLWU_WAKEUP_SOURCE_P9_RISING = 28,
  LLWU_WAKEUP_SOURCE_P9_FALLING = 29,
  LLWU_WAKEUP_SOURCE_P9_ANY = 30,

  LLWU_WAKEUP_SOURCE_P10_RISING = 31,
  LLWU_WAKEUP_SOURCE_P10_FALLING = 32,
  LLWU_WAKEUP_SOURCE_P10_ANY = 33,

  LLWU_WAKEUP_SOURCE_P11_RISING = 34,
  LLWU_WAKEUP_SOURCE_P11_FALLING = 35,
  LLWU_WAKEUP_SOURCE_P11_ANY = 36,
  /*********** LLWU_PE4 ******************/
  LLWU_WAKEUP_SOURCE_P12_RISING = 37,
  LLWU_WAKEUP_SOURCE_P12_FALLING = 38,
  LLWU_WAKEUP_SOURCE_P12_ANY = 39,

  LLWU_WAKEUP_SOURCE_P13_RISING = 40,
  LLWU_WAKEUP_SOURCE_P13_FALLING = 41,
  LLWU_WAKEUP_SOURCE_P13_ANY = 42,

  LLWU_WAKEUP_SOURCE_P14_RISING = 43,
  LLWU_WAKEUP_SOURCE_P14_FALLING = 44,
  LLWU_WAKEUP_SOURCE_P14_ANY = 45,

  LLWU_WAKEUP_SOURCE_P15_RISING = 46,
  LLWU_WAKEUP_SOURCE_P15_FALLING = 47,
  LLWU_WAKEUP_SOURCE_P15_ANY = 48,

  LLWU_WAKEUP_SOURCE_LPT = 49,
};

typedef struct llwu_control
{
  struct llwu_control *next;
  char allow_llwu;
} llwu_control_t;

// Macro to create new llwu control struct for clients.
#define LLWU_CONTROL(name) static llwu_control_t name[1] = {{NULL, 1}} //default allow deep sleep

void llwu_init();

/**
 *  Sleep until some process is polled, ie interrupt occurs.
 */
void llwu_sleep();
/**
 * Register as a controller for llwu.
 * @param c Controller struct.
 */
void llwu_register(llwu_control_t* c);

/**
 * Method for a controller to allow or disallow deep sleep.
 * @param c Control struct
 * @param allow 0 to disable deep sleep, other than 0 to allow.
 */
void llwu_set_allow(llwu_control_t* c, char allow);

void llwu_enable_wakeup_source(enum LLWU_WAKEUP_SOURCE s);
void llwu_enable_disable_source(enum LLWU_WAKEUP_SOURCE s);
#endif /* LLWU_H_ */
