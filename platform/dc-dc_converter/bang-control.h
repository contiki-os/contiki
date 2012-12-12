/*
 * bang-control.h
 *
 *  Created on: Dec 11, 2012
 *      Author: cazulu
 */

#ifndef BANG_CONTROL_H_
#define BANG_CONTROL_H_

#include "contiki.h"

//Declare the name of the controller process
PROCESS_NAME(bang_process);

//Identifiers of all the possible states
//of the control algorithm
typedef enum _converterState{
  CONV_BUCK_OFF=0,
  CONV_BUCK_SOFT,
  CONV_BUCK_ON,
  CONV_BOOST_OFF,
  CONV_BOOST_SOFT,
  CONV_BOOST_ON,
  CONV_DISCHARGE,
  CONV_ALL_OFF
}CONV_STATE;

//Identifiers of the algorithm parameters
typedef enum _converterParameter{
  CONV_VREF=0,
  CONV_IMAX,
  CONV_VMAX
}CONV_PARAM;

//Acceptable range of values of the algorithm parameters
#define VREF_FLOAT_MIN        0.0
#define VREF_FLOAT_DEFAULT    3.0
#define VMAX_FLOAT_MIN        0.0
#define VMAX_FLOAT_DEFAULT    5.0
#define VMAX_FLOAT_MAX        30.0
#define IMAX_FLOAT_MIN        0.0
#define IMAX_FLOAT_DEFAULT    0.1
#define IMAX_FLOAT_MAX        6.0


//Function to set the parameters used by the DC-DC converter
//control algorithm:
//      -Vref:Desired output voltage
//      -Imax:Maximum inductor current allowed
//      -Vmax:Maximum output voltage allowed
void setConverterParameter(int paramId, float paramValue);

//Return the value of the selected parameter
float getConverterParameter(int paramId);

//Change the state of the algorithm
//and modify the switches accordingly
  void setConverterState(int newConvState);

//Return the state of the control algorithm as an integer
int getConverterState();

//Return the state of the control algorithm as a string
//(defined as a static char array inside function)
char *getConverterStateString();



#endif /* BANG_CONTROL_H_ */
