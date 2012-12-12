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
  CONV_BUCK_OFF,
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
  CONV_VREF,
  CONV_IMAX,
  CONV_VMAX
}CONV_PARAM;

//Acceptable range of values of the algorithm parameters
//TODO: Check Jorge's code and thesis for the right values
#define VREF_MIN        0
#define VREF_DEFAULT    0
#define VREF_MAX        0
#define IMAX_MIN        0
#define IMAX_DEFAULT    0
#define IMAX_MAX        0
#define VMAX_MIN        0
#define VMAX_DEFAULT    0
#define VMAX_MAX        0

//Function to set the parameters used by the DC-DC converter
//control algorithm:
//      -Vref:Desired output voltage
//      -Imax:Maximum inductor current allowed
//      -Vmax:Maximum output voltage allowed
void setConverterParameter(int paramId, int paramValue);

//Return the value of the selected parameter
int getConverterParameter(int paramId);



#endif /* BANG_CONTROL_H_ */
