/*
 * llwu.h
 *
 *  Created on: Mar 19, 2014
 *      Author: henmak
 */

#ifndef LLWU_H_
#define LLWU_H_

#include <stdint.h>
#include "K60.h"
#include "synchronization.h"

#ifdef __cplusplus
extern "C" {
#endif

extern volatile uint32_t llwu_inhibit_lls_sema;
extern volatile uint32_t llwu_inhibit_vlps_sema;
extern volatile uint32_t llwu_inhibit_stop_sema;

/** Internal modules whose interrupts are mapped to LLWU wake up sources.
 *
 * Wake up source module number: Kinetis Module
 * 0: LPTMR
 * 1: CMP0
 * 2: CMP1
 * 3: CMP2
 * 4: TSI
 * 5: RTC Alarm
 * 6: Reserved
 * 7: RTC Seconds
 *
 * Other modules CAN NOT be used to wake the CPU from LLS or VLLSx power modes.
 */
typedef enum llwu_wakeup_module {
  LLWU_WAKEUP_MODULE_LPTMR = 0,
  LLWU_WAKEUP_MODULE_CMP0 = 1,
  LLWU_WAKEUP_MODULE_CMP1 = 2,
  LLWU_WAKEUP_MODULE_CMP2 = 3,
  LLWU_WAKEUP_MODULE_TSI = 4,
  LLWU_WAKEUP_MODULE_RTC_ALARM = 5,
  LLWU_WAKEUP_MODULE_RESERVED = 6,
  LLWU_WAKEUP_MODULE_RTC_SECONDS = 7,
  LLWU_WAKEUP_MODULE_END,
} llwu_wakeup_module_t;

/** enum that maps physical pins to wakeup pin numbers in LLWU module
 *
 * Other modules CAN NOT be used to wake the CPU from LLS or VLLSx power modes. */
typedef enum llwu_wakeup_pin {
  LLWU_WAKEUP_PIN_PTE1 = 0,
  LLWU_WAKEUP_PIN_PTE2 = 1,
  LLWU_WAKEUP_PIN_PTE4 = 2,
  LLWU_WAKEUP_PIN_PTA4 = 3,
  LLWU_WAKEUP_PIN_PTA13 = 4,
  LLWU_WAKEUP_PIN_PTB0 = 5,
  LLWU_WAKEUP_PIN_PTC1 = 6,
  LLWU_WAKEUP_PIN_PTC3 = 7,
  LLWU_WAKEUP_PIN_PTC4 = 8,
  LLWU_WAKEUP_PIN_PTC5 = 9,
  LLWU_WAKEUP_PIN_PTC6 = 10,
  LLWU_WAKEUP_PIN_PTC11 = 11,
  LLWU_WAKEUP_PIN_PTD0 = 12,
  LLWU_WAKEUP_PIN_PTD2 = 13,
  LLWU_WAKEUP_PIN_PTD4 = 14,
  LLWU_WAKEUP_PIN_PTD6 = 15,
  LLWU_WAKEUP_PIN_END
} llwu_wakeup_pin_t;

typedef enum llwu_wakeup_edge {
  LLWU_WAKEUP_EDGE_DISABLE = 0b00,
  LLWU_WAKEUP_EDGE_RISING = 0b01,
  LLWU_WAKEUP_EDGE_FALLING = 0b10,
  LLWU_WAKEUP_EDGE_ANY = 0b11,
} llwu_wakeup_edge_t;

#define LLWU_WAKEUP_EDGE_MASK (0x03)
#define LLWU_WAKEUP_EDGE_WIDTH (2)
#define LLWU_WAKEUP_PIN_REG_SIZE (4)

typedef struct llwu_control {
  struct llwu_control *next;
  char allow_llwu;
} llwu_control_t;

/* Macro to create new llwu control struct for clients. */
#define LLWU_CONTROL(name) static llwu_control_t name[1] = { { NULL, 1 } } /* default allow deep sleep */

void llwu_init();

/**
 *  Sleep until some process is polled, ie interrupt occurs.
 */
void llwu_sleep();
/**
 * Register as a controller for llwu.
 * @param c Controller struct.
 */
void llwu_register(llwu_control_t *c);

/**
 * Method for a controller to allow or disallow deep sleep.
 * @param c Control struct
 * @param allow 0 to disable deep sleep, other than 0 to allow.
 */
void llwu_set_allow(llwu_control_t *c, char allow);

void llwu_enable_wakeup_module(const llwu_wakeup_module_t module);
void llwu_disable_wakeup_module(const llwu_wakeup_module_t module);
void llwu_set_wakeup_pin(const llwu_wakeup_pin_t pin, const llwu_wakeup_edge_t edge);

/* We can not use a lock variable for the inhibit counters, because that can
 * lead to deadlocks in ISRs, but we can use the exclusive load/store
 * instructions (same as used for implementing thread safe locks) for the
 * inhibit counters themselves to make sure we never lose an increment or
 * decrement. */
#define LLWU_INHIBIT_STOP() (exclusive_increment(&llwu_inhibit_lls_sema))
#define LLWU_INHIBIT_VLPS() (exclusive_increment(&llwu_inhibit_vlps_sema))
#define LLWU_INHIBIT_LLS() (exclusive_increment(&llwu_inhibit_lls_sema))
#define LLWU_UNINHIBIT_STOP() (exclusive_decrement(&llwu_inhibit_stop_sema))
#define LLWU_UNINHIBIT_VLPS() (exclusive_decrement(&llwu_inhibit_vlps_sema))
#define LLWU_UNINHIBIT_LLS() (exclusive_decrement(&llwu_inhibit_lls_sema))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LLWU_H_ */
