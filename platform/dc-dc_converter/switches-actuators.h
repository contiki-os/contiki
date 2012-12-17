/*
 * switches-actuators.h
 *
 *  Created on: Dec 11, 2012
 *      Author: cazulu
 */

#ifndef SWITCHES_ACTUATORS_H_
#define SWITCHES_ACTUATORS_H_

#define NSWITCHES 4

//Sw1L: P2.0
//Connected to Ngate1 through the FAN3278
#define SW_NGATE1_PORT    2
#define SW_NGATE1_PIN     _BIT(0)
//Sw1H: P2.1
//Connected to Pgate1 through the FAN3278
#define SW_PGATE1_PORT   2
#define SW_PGATE1_PIN    _BIT(1)
//Sw2L: P2.2
//Connected to Ngate2 through the FAN3278
#define SW_NGATE2_PORT    2
#define SW_NGATE2_PIN     _BIT(2)
//Sw2H: P2.3
//Connected to Pgate2 through the FAN3278
#define SW_PGATE2_PORT   2
#define SW_PGATE2_PIN    _BIT(3)

typedef enum{
  SW_NGATE1=0,
  SW_NGATE2,
  SW_PGATE1,
  SW_PGATE2
}SW_ID;

typedef enum{
  SW_ON,
  SW_OFF
}SW_STATE;

//Initialize the GPIO for the switches
void
switches_init();

//Change the state of a switch to SW_ON or SW_OFF
void
switches_set_gate_state(int swId, int swState);

//Get the current state of a particular switch
int
switches_get_gate_state(int swId);


#endif /* SWITCHES_ACTUATORS_H_ */
