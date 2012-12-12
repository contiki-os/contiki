/*
 * bang-control.c
 *
 *  Created on: Dec 11, 2012
 *      Author: cazulu
 */
#include <stdio.h>
#include "lib/sensors.h"
#include "switches-actuators.h"
#include "adc-sensors.h"
#include "bang-control.h"

/*** This file contains the implementation of the bang-bang algorithm ***/
//TODO: Implement a shoot-through minimizing technique to increase converter efficiency

//Current state of the control algorithm
CONV_STATE convState;
//Current value of the algorithm parameters
int vRef;
int vMax;
int iMax;


//Function to set the parameters used by the DC-DC converter
//control algorithm:
//      -Vref:Desired output voltage
//      -Imax:Maximum inductor current allowed
//      -Vmax:Maximum output voltage allowed
void setConverterParameter(int paramId, int paramValue){
  //Change the parameter values if the are within the accepted
  //ranges and inform the controller process
  switch(paramId){
  case CONV_VREF:
    if(paramValue>=VREF_MIN && paramValue<=VREF_MAX){
     vRef=paramValue;
     process_poll(&bang_process);
    }
    break;

  case CONV_IMAX:
    if(paramValue>=IMAX_MIN && paramValue<=IMAX_MAX){
        iMax=paramValue;
        process_poll(&bang_process);
    }
    break;

  case CONV_VMAX:
    if(paramValue>=VMAX_MAX && paramValue<=VMAX_MIN){
        vMax=paramValue;
        process_poll(&bang_process);
    }
    break;
  }

}

//Return the value of the selected parameter
int getConverterParameter(int paramId){
  int paramValue=0;
  switch(paramId){
  case CONV_VREF:
    paramValue=vRef;
    break;

  case CONV_IMAX:
    paramValue=iMax;
    break;

  case CONV_VMAX:
    paramValue=vMax;
    break;
  }
  return paramValue;
}


/* Matrix of converter state vs state of the switches
 *      STATE           |       N-CHANNEL1      |       P-CHANNEL1      |       N-CHANNEL2      |       P-CHANNEL2      |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      BUCK_OFF        |       ON              |       OFF             |       OFF             |       ON              |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      BUCK_SOFT       |       OFF             |       OFF             |       OFF             |       ON              |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      BUCK_ON         |       OFF             |       ON              |       OFF             |       ON              |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      BOOST_OFF       |       OFF             |       ON              |       OFF             |       ON              |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      BOOST_SOFT      |       OFF             |       ON              |       OFF             |       OFF             |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      BOOST_ON        |       OFF             |       ON              |       ON              |       OFF             |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      DISCHARGE       |       ON              |       OFF             |       ON              |       OFF             |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 *      ALL_OFF         |       OFF             |       OFF             |       OFF             |       OFF             |
 *----------------------|-----------------------|-----------------------|-----------------------|-----------------------|
 */
//Change the state of the algorithm
//and modify the switches accordingly
void setConverterState(int newConvState){
  switch(newConvState){

  case CONV_BUCK_OFF:
    setSwitchState(SW_NGATE1, SW_ON);
    setSwitchState(SW_PGATE1, SW_OFF);
    setSwitchState(SW_NGATE2, SW_OFF);
    setSwitchState(SW_PGATE2, SW_ON);
    convState=newConvState;
    break;

  case CONV_BUCK_SOFT:
    setSwitchState(SW_NGATE1, SW_OFF);
    setSwitchState(SW_PGATE1, SW_OFF);
    setSwitchState(SW_NGATE2, SW_OFF);
    setSwitchState(SW_PGATE2, SW_ON);
    convState=newConvState;
    break;


  case CONV_BUCK_ON:
    setSwitchState(SW_NGATE1, SW_OFF);
    setSwitchState(SW_PGATE1, SW_ON);
    setSwitchState(SW_NGATE2, SW_OFF);
    setSwitchState(SW_PGATE2, SW_ON);
    convState=newConvState;
    break;

  case CONV_BOOST_OFF:
    setSwitchState(SW_NGATE1, SW_OFF);
    setSwitchState(SW_PGATE1, SW_ON);
    setSwitchState(SW_NGATE2, SW_OFF);
    setSwitchState(SW_PGATE2, SW_ON);
    convState=newConvState;
    break;

  case CONV_BOOST_SOFT:
    setSwitchState(SW_NGATE1, SW_OFF);
    setSwitchState(SW_PGATE1, SW_ON);
    setSwitchState(SW_NGATE2, SW_OFF);
    setSwitchState(SW_PGATE2, SW_OFF);
    convState=newConvState;
    break;

  case CONV_BOOST_ON:
    setSwitchState(SW_NGATE1, SW_OFF);
    setSwitchState(SW_PGATE1, SW_ON);
    setSwitchState(SW_NGATE2, SW_ON);
    setSwitchState(SW_PGATE2, SW_OFF);
    convState=newConvState;
    break;

  case CONV_DISCHARGE:
    setSwitchState(SW_NGATE1, SW_ON);
    setSwitchState(SW_PGATE1, SW_OFF);
    setSwitchState(SW_NGATE2, SW_ON);
    setSwitchState(SW_PGATE2, SW_OFF);
    convState=newConvState;
    break;

  case CONV_ALL_OFF:
    setSwitchState(SW_NGATE1, SW_OFF);
    setSwitchState(SW_PGATE1, SW_OFF);
    setSwitchState(SW_NGATE2, SW_OFF);
    setSwitchState(SW_PGATE2, SW_OFF);
    convState=newConvState;
    break;
  }

}

//Return the state of the control algorithm
int getConverterState(){
  return (int)convState;
}

//Declare the process
PROCESS(bang_process, "Bang-Bang control algorithm process");

PROCESS_THREAD(bang_process, ev, data){

  PROCESS_BEGIN();

  //Start the ADC sensors and the sensor controlling process
  printf("Starting the Contiki sensor process\n");
  process_start(&sensors_process, NULL);
  printf("Activating the sensors\n");
  SVECTOR_SENSOR_ACTIVATE();
  //Initialize the switches
  initSwitches();
  //Set the starting state of the algorithm
  setConverterState(CONV_ALL_OFF);
  //Set the parameters to their default values
  setConverterParameter(CONV_VREF, VREF_DEFAULT);
  setConverterParameter(CONV_IMAX, IMAX_DEFAULT);
  setConverterParameter(CONV_VMAX, VMAX_DEFAULT);

  //Parameters used to store the value of the state vector variables
  int vOut, vIn, iL;

  //Main control loop
  while(1){
      //Wait until the ADC interrupt polls the process
      PROCESS_WAIT_EVENT();
      //Get the current value of the ADC parameters
      vOut= svector_sensor.value(SVECTOR_SENSOR_VOUT);
      vIn= svector_sensor.value(SVECTOR_SENSOR_VIN);
      iL= svector_sensor.value(SVECTOR_SENSOR_IL);

      if(vRef<=0)
        setConverterState(CONV_ALL_OFF);
      else if(vOut<=vMax)
        setConverterState(CONV_DISCHARGE);
      else if(iL>iMax){
          if(vOut>vIn)
            setConverterState(CONV_BOOST_OFF);
          else
            setConverterState(CONV_BUCK_OFF);
      }
      else if(vRef<=vIn){
          if(vOut<=vRef)
            setConverterState(CONV_BOOST_ON);
          else if(iL<=0)
            setConverterState(CONV_BOOST_SOFT);
          else
            setConverterState(CONV_BOOST_OFF);
      }
      else if(vOut<=vRef)
        setConverterState(CONV_BUCK_ON);
      else if(iL<=0)
        setConverterState(CONV_BUCK_SOFT);
      else
        setConverterState(CONV_BUCK_OFF);
  }

  PROCESS_END();
}
