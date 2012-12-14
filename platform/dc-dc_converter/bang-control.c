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
//Indicates if we have finished configuring the
//starting parameters and can begin the algorithm
Bool isConfigured = FALSE;
//Indicates if the microgrid server has allowed
//the new user access to the grid
Bool userAllowed = FALSE;
//Current state of the control algorithm
CONV_STATE convState = CONV_ALL_OFF;
//Current value of the algorithm parameters
int vRef = 0;
int vMax = 0;
int iMax = 0;

//Enable the voltage output for this user
void allowUser(){
  userAllowed=TRUE;
}

//Cut the user off from the grid
void forbidUser(){
  userAllowed=FALSE;
}

//Returns 0 if the user does not have access
//to the power grid
int isUserAllowed(){
  return (int)userAllowed;
}

//Function to set the parameters used by the DC-DC converter
//control algorithm:
//      -Vref:Desired output voltage
//      -Imax:Maximum inductor current allowed
//      -Vmax:Maximum output voltage allowed
void
setConverterParameter(int paramId, float paramValue)
{
  int paramAdcValue = 0;
  //Change the parameter values if the are within the accepted
  //ranges and inform the controller process
  switch (paramId)
    {
  case CONV_VREF:
    paramAdcValue = VREF_FLOAT_TO_ADC(paramValue);
    if (paramAdcValue >= VREF_FLOAT_TO_ADC(VREF_FLOAT_MIN)
        && paramAdcValue <= vMax)
      {
        vRef = paramAdcValue;
        process_poll(&bang_process);
      }
    break;

  case CONV_IMAX:
    paramAdcValue = IMAX_FLOAT_TO_ADC(paramValue);
    if (paramAdcValue >= IMAX_FLOAT_TO_ADC(IMAX_FLOAT_MIN)
        && paramAdcValue <= IMAX_FLOAT_TO_ADC(IMAX_FLOAT_MAX))
      {
        iMax = paramAdcValue;
        process_poll(&bang_process);
      }
    break;

  case CONV_VMAX:
    paramAdcValue = VMAX_FLOAT_TO_ADC(paramValue);
    if (paramAdcValue >= VMAX_FLOAT_TO_ADC(VMAX_FLOAT_MIN)
        && paramAdcValue <= VMAX_FLOAT_TO_ADC(VMAX_FLOAT_MAX))
      {
        vMax = paramAdcValue;
        process_poll(&bang_process);
      }
    break;

    }
}

//Return the value of the selected parameter
  float
  getConverterParameter(int paramId)
  {
    float paramValue = 0.0;
    switch (paramId)
      {
    case CONV_VREF:
      paramValue = VREF_ADC_TO_FLOAT(vRef);
      break;

    case CONV_IMAX:
      paramValue = IMAX_ADC_TO_FLOAT(iMax);
      break;

    case CONV_VMAX:
      paramValue = VMAX_ADC_TO_FLOAT(vMax);
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
  void
  setConverterState(int newConvState)
  {
    switch ((CONV_STATE)newConvState)
      {

    case CONV_BUCK_OFF:
      setSwitchState(SW_NGATE1, SW_ON);
      setSwitchState(SW_PGATE1, SW_OFF);
      setSwitchState(SW_NGATE2, SW_OFF);
      setSwitchState(SW_PGATE2, SW_ON);
      convState = (CONV_STATE)newConvState;
      break;

    case CONV_BUCK_SOFT:
      setSwitchState(SW_NGATE1, SW_OFF);
      setSwitchState(SW_PGATE1, SW_OFF);
      setSwitchState(SW_NGATE2, SW_OFF);
      setSwitchState(SW_PGATE2, SW_ON);
      convState = (CONV_STATE)newConvState;
      break;

    case CONV_BUCK_ON:
      setSwitchState(SW_NGATE1, SW_OFF);
      setSwitchState(SW_PGATE1, SW_ON);
      setSwitchState(SW_NGATE2, SW_OFF);
      setSwitchState(SW_PGATE2, SW_ON);
      convState = (CONV_STATE)newConvState;
      break;

    case CONV_BOOST_OFF:
      setSwitchState(SW_NGATE1, SW_OFF);
      setSwitchState(SW_PGATE1, SW_ON);
      setSwitchState(SW_NGATE2, SW_OFF);
      setSwitchState(SW_PGATE2, SW_ON);
      convState = (CONV_STATE)newConvState;
      break;

    case CONV_BOOST_SOFT:
      setSwitchState(SW_NGATE1, SW_OFF);
      setSwitchState(SW_PGATE1, SW_ON);
      setSwitchState(SW_NGATE2, SW_OFF);
      setSwitchState(SW_PGATE2, SW_OFF);
      convState = (CONV_STATE)newConvState;
      break;

    case CONV_BOOST_ON:
      setSwitchState(SW_NGATE1, SW_OFF);
      setSwitchState(SW_PGATE1, SW_ON);
      setSwitchState(SW_NGATE2, SW_ON);
      setSwitchState(SW_PGATE2, SW_OFF);
      convState = (CONV_STATE)newConvState;
      break;

    case CONV_DISCHARGE:
      setSwitchState(SW_NGATE1, SW_ON);
      setSwitchState(SW_PGATE1, SW_OFF);
      setSwitchState(SW_NGATE2, SW_ON);
      setSwitchState(SW_PGATE2, SW_OFF);
      convState = (CONV_STATE)newConvState;
      break;

    case CONV_ALL_OFF:
      setSwitchState(SW_NGATE1, SW_OFF);
      setSwitchState(SW_PGATE1, SW_OFF);
      setSwitchState(SW_NGATE2, SW_OFF);
      setSwitchState(SW_PGATE2, SW_OFF);
      convState = (CONV_STATE)newConvState;
      break;
      }

  }

//Return the state of the control algorithm
  int
  getConverterState()
  {
    return (int) convState;
  }

//Return the state of the control algorithm as a string
//(defined as a static char array inside function)
  char *
  getConverterStateString()
  {
    char * strPtr = NULL;
    switch (convState)
      {

    case CONV_BUCK_OFF:
      {
        static char buckOffString[] = "BUCK_OFF";
        strPtr = buckOffString;
        break;
      }

    case CONV_BUCK_SOFT:
      {
        static char buckSoftString[] = "BUCK_SOFT";
        strPtr = buckSoftString;
        break;
      }

    case CONV_BUCK_ON:
      {
        static char buckOnString[] = "BUCK_ON";
        strPtr = buckOnString;
        break;
      }

    case CONV_BOOST_OFF:
      {
        static char boostOffString[] = "BOOST_OFF";
        strPtr = boostOffString;
        break;
      }

    case CONV_BOOST_SOFT:
      {
        static char boostSoftString[] = "BOOST_SOFT";
        strPtr = boostSoftString;
        break;
      }

    case CONV_BOOST_ON:
      {
        static char boostOnString[] = "BOOST_ON";
        strPtr = boostOnString;
        break;
      }

    case CONV_DISCHARGE:
      {
        static char dischargeString[] = "DISCHARGE";
        strPtr = dischargeString;
        break;
      }

    case CONV_ALL_OFF:
      {
        static char allOffString[] = "ALL_OFF";
        strPtr = allOffString;
        break;
      }
      }

    return strPtr;

  }

  //This is the process poll handler and
  //the bang-bang algorithm main loop
  static void pollhandler()
  {
    //Parameters used to store the value of the state vector variables
    int vOut, vIn, iL;

    //Get the current value of the ADC parameters
    vOut = svector_sensor.value(SVECTOR_SENSOR_VOUT);
    vIn = svector_sensor.value(SVECTOR_SENSOR_VIN);
    iL = svector_sensor.value(SVECTOR_SENSOR_IL);

    if (!isConfigured || !userAllowed || vRef <= 0)
      {
        setConverterState(CONV_ALL_OFF);
      }
    else if (vOut <= vMax)
      {
        setConverterState(CONV_DISCHARGE);
      }
    else if (iL > iMax)
      {
        if (vOut > vIn)
          setConverterState(CONV_BOOST_OFF);
        else
          setConverterState(CONV_BUCK_OFF);
      }
    else if (vRef <= vIn)
      {
        if (vOut <= vRef)
          setConverterState(CONV_BOOST_ON);
        else if (iL <= 0)
          setConverterState(CONV_BOOST_SOFT);
        else
          setConverterState(CONV_BOOST_OFF);
      }
    else if (vOut <= vRef)
      {
        setConverterState(CONV_BUCK_ON);
      }
    else if (iL <= 0)
      {
        setConverterState(CONV_BUCK_SOFT);
      }
    else
      {
        setConverterState(CONV_BUCK_OFF);
      }

  }

//Declare the process
  PROCESS(bang_process, "Bang-Bang control algorithm process");

  PROCESS_THREAD(bang_process, ev, data)
  {

    //Tell Contiki that this process has a poll handler
    //that can be invoked with process_poll(&bang_process)
    PROCESS_POLLHANDLER(pollhandler());

    PROCESS_BEGIN();

      //Initialize the switches
      initSwitches();
      //Set the starting state of the algorithm
      setConverterState(CONV_ALL_OFF);
      //Set the parameters to their default values
      vMax = VMAX_FLOAT_TO_ADC(VMAX_FLOAT_DEFAULT);
      iMax = IMAX_FLOAT_TO_ADC(IMAX_FLOAT_DEFAULT);
      vRef = VREF_FLOAT_TO_ADC(VREF_FLOAT_DEFAULT);
      isConfigured = TRUE;

      //Start the ADC sensors and the sensor controlling process
      printf("Starting the Contiki sensor process\n");
      process_start(&sensors_process, NULL );
      //Wait for the ADC to settle before starting to sample
      static struct etimer et;
      etimer_set(&et, 5*CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      printf("Activating the sensors\n");
      SVECTOR_SENSOR_ACTIVATE();

      //Do nothing, all the real control stuff
      //is in the poll handler so it can be invoked
      //by the ADC interruption
      PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

    PROCESS_END();
}
