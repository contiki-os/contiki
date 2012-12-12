/*
 * switches-actuators.c
 *
 *  Created on: Dec 11, 2012
 *      Author: cazulu
 */

#include "switches-actuators.h"
#include "lpc17xx_gpio.h"

//Array to store the current state of all the switches
static SW_STATE swStatusArray[NSWITCHES];

//Initialize the GPIO for the switches
void
initSwitches()
{
  //Set all the switch controlling pins as outputs
  GPIO_SetDir(SW_NGATE1_PORT, SW_NGATE1_PIN, 1);
  GPIO_SetDir(SW_NGATE2_PORT, SW_NGATE2_PIN, 1);
  GPIO_SetDir(SW_PGATE1_PORT, SW_PGATE1_PIN, 1);
  GPIO_SetDir(SW_PGATE2_PORT, SW_PGATE2_PIN, 1);

  //Turn off all the switches
  setSwitchState(SW_PGATE1, SW_OFF);
  setSwitchState(SW_PGATE2, SW_OFF);
  setSwitchState(SW_NGATE1, SW_OFF);
  setSwitchState(SW_NGATE2, SW_OFF);
}

//Change the state of a switch to SW_ON or SW_OFF
void
setSwitchState(int swId, int swState)
{
  switch (swId)
    {
  case SW_NGATE1:
    if(swState==SW_OFF){
        GPIO_ClearValue(SW_NGATE1_PORT, SW_NGATE1_PIN);
        swStatusArray[SW_NGATE1]=SW_OFF;
    }
    else if(swState==SW_ON){
        GPIO_SetValue(SW_NGATE1_PORT, SW_NGATE1_PIN);
        swStatusArray[SW_NGATE1]=SW_ON;
    }
    break;

  case SW_NGATE2:
    if(swState==SW_OFF){
        GPIO_ClearValue(SW_NGATE2_PORT, SW_NGATE2_PIN);
        swStatusArray[SW_NGATE2]=SW_OFF;
    }
    else if(swState==SW_ON){
        GPIO_SetValue(SW_NGATE2_PORT, SW_NGATE2_PIN);
        swStatusArray[SW_NGATE2]=SW_ON;
    }
    break;

  case SW_PGATE1:
    if(swState==SW_OFF){
        GPIO_ClearValue(SW_PGATE1_PORT, SW_PGATE1_PIN);
        swStatusArray[SW_PGATE1]=SW_OFF;
    }
    else if(swState==SW_ON){
        GPIO_SetValue(SW_PGATE1_PORT, SW_NGATE1_PIN);
        swStatusArray[SW_PGATE1]=SW_ON;
    }
    break;

  case SW_PGATE2:
    if(swState==SW_OFF){
        GPIO_ClearValue(SW_PGATE2_PORT, SW_PGATE2_PIN);
        swStatusArray[SW_PGATE2]=SW_OFF;
    }
    else if(swState==SW_ON){
        GPIO_SetValue(SW_PGATE2_PORT, SW_NGATE2_PIN);
        swStatusArray[SW_PGATE2]=SW_ON;
    }
    break;
    }

}

//Get the current state of a particular switch
int
getSwitchState(int swId)
{
  SW_STATE swState = SW_OFF;
  switch (swId)
    {
  case SW_NGATE1:
    swState = swStatusArray[SW_NGATE1];
    break;
  case SW_NGATE2:
    swState = swStatusArray[SW_NGATE2];
    break;
  case SW_PGATE1:
    swState = swStatusArray[SW_PGATE1];
    break;
  case SW_PGATE2:
    swState = swStatusArray[SW_PGATE2];
    break;
    }
  return (int)swState;
}
