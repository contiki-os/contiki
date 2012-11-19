/*
 * leds_arch.h
 *
 *  Created on: Nov 18, 2012
 *      Author: cazulu
 */

#ifndef LEDS_ARCH_H_
#define LEDS_ARCH_H_

#define PIN_LED2 1
#define PIN_LED3 1
#define LED2    PIN_LED2<<18   //LED2 P1.18
#define LED3    PIN_LED3<<29   //LED3 P1.29

void toggleLeds();

#endif /* LEDS_ARCH_H_ */
