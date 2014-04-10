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
  LLWU_WAKEUP_SOURCE_LPT = 0x01
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
