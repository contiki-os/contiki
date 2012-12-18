/*
 * switches-actuators.c
 *
 *  Created on: Dec 11, 2012
 *      Author: cazulu
 */

#include "switches-actuators.h"
#include "lpc17xx_gpio.h"

//Array to store the current state of all the switches
static SW_STATE switch_status_array[NSWITCHES];

//Initialize the GPIO for the switches
void
switches_init()
{
  LPC_PINCON->PINSEL4 &= ~0x0000000FF;            // Set GPIO control as digital outputs
    LPC_GPIO2->FIODIRL |= 0x000F;                   // Set P2.0, P2.1, P2.2, P2.3 as outputs
    LPC_GPIO2->FIOPINL &= ~0x000F;                  // Set OFF all Buck and Boost switches
  /*
  //Set all the switch controlling pins as outputs
  GPIO_SetDir(SW_NGATE1_PORT, SW_NGATE1_PIN, 1);
  GPIO_SetDir(SW_NGATE2_PORT, SW_NGATE2_PIN, 1);
  GPIO_SetDir(SW_PGATE1_PORT, SW_PGATE1_PIN, 1);
  GPIO_SetDir(SW_PGATE2_PORT, SW_PGATE2_PIN, 1);

  //Turn off all the switches
  switches_set_gate_state(SW_PGATE1, SW_OFF);
  switches_set_gate_state(SW_PGATE2, SW_OFF);
  switches_set_gate_state(SW_NGATE1, SW_OFF);
  switches_set_gate_state(SW_NGATE2, SW_OFF);
  */

}

//Change the state of a switch to SW_ON or SW_OFF
void
switches_set_gate_state(int sw_id, int sw_state)
{
  switch (sw_id)
    {
  case SW_NGATE1:
    if(sw_state==SW_OFF){
        GPIO_ClearValue(SW_NGATE1_PORT, SW_NGATE1_PIN);
        switch_status_array[SW_NGATE1]=SW_OFF;
    }
    else{
        GPIO_SetValue(SW_NGATE1_PORT, SW_NGATE1_PIN);
        switch_status_array[SW_NGATE1]=SW_ON;
    }
    break;

  case SW_NGATE2:
    if(sw_state==SW_OFF){
        GPIO_ClearValue(SW_NGATE2_PORT, SW_NGATE2_PIN);
        switch_status_array[SW_NGATE2]=SW_OFF;
    }
    else{
        GPIO_SetValue(SW_NGATE2_PORT, SW_NGATE2_PIN);
        switch_status_array[SW_NGATE2]=SW_ON;
    }
    break;

  case SW_PGATE1:
    if(sw_state==SW_OFF){
        GPIO_ClearValue(SW_PGATE1_PORT, SW_PGATE1_PIN);
        switch_status_array[SW_PGATE1]=SW_OFF;
    }
    else{
        GPIO_SetValue(SW_PGATE1_PORT, SW_NGATE1_PIN);
        switch_status_array[SW_PGATE1]=SW_ON;
    }
    break;

  case SW_PGATE2:
    if(sw_state==SW_OFF){
        GPIO_ClearValue(SW_PGATE2_PORT, SW_PGATE2_PIN);
        switch_status_array[SW_PGATE2]=SW_OFF;
    }
    else{
        GPIO_SetValue(SW_PGATE2_PORT, SW_NGATE2_PIN);
        switch_status_array[SW_PGATE2]=SW_ON;
    }
    break;
    }

}

//Get the current state of a particular switch
int
switches_get_gate_state(int sw_id)
{
  SW_STATE sw_state = SW_OFF;
  switch (sw_id)
    {
  case SW_NGATE1:
    sw_state = switch_status_array[SW_NGATE1];
    break;
  case SW_NGATE2:
    sw_state = switch_status_array[SW_NGATE2];
    break;
  case SW_PGATE1:
    sw_state = switch_status_array[SW_PGATE1];
    break;
  case SW_PGATE2:
    sw_state = switch_status_array[SW_PGATE2];
    break;
    }
  return (int)sw_state;
}
