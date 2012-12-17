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
static Bool algorithm_configured = FALSE;
//Indicates if the microgrid server has allowed
//the new user access to the grid
static Bool user_allowed = FALSE;
//Current state of the control algorithm
static CONV_STATE converter_state = CONV_ALL_OFF;
//Current value of the algorithm parameters
static int v_ref = 0;
static int v_max = 0;
static int i_max = 0;

//Enable the voltage output for this user
void dc_converter_allow_user(){
  user_allowed=TRUE;
}

//Cut the user off from the grid
void dc_converter_forbid_user(){
  user_allowed=FALSE;
}

//Returns 0 if the user does not have access
//to the power grid
int dc_converter_get_user_status(){
  return (int)user_allowed;
}

//Function to set the parameters used by the DC-DC converter
//control algorithm:
//      -Vref:Desired output voltage
//      -Imax:Maximum inductor current allowed
//      -Vmax:Maximum output voltage allowed
void
dc_converter_set_control_parameter(int param_id, float param_float_value)
{
  int param_adc_value = 0;
  //Change the parameter values if the are within the accepted
  //ranges and inform the controller process
  switch (param_id)
    {
  case CONV_VREF:
    param_adc_value = VREF_FLOAT_TO_ADC(param_float_value);
    if (param_adc_value >= VREF_FLOAT_TO_ADC(VREF_FLOAT_MIN)
        && param_adc_value <= v_max)
      {
        v_ref = param_adc_value;
        process_poll(&bang_process);
      }
    break;

  case CONV_IMAX:
    param_adc_value = IMAX_FLOAT_TO_ADC(param_float_value);
    if (param_adc_value >= IMAX_FLOAT_TO_ADC(IMAX_FLOAT_MIN)
        && param_adc_value <= IMAX_FLOAT_TO_ADC(IMAX_FLOAT_MAX))
      {
        i_max = param_adc_value;
        process_poll(&bang_process);
      }
    break;

  case CONV_VMAX:
    param_adc_value = VMAX_FLOAT_TO_ADC(param_float_value);
    if (param_adc_value >= VMAX_FLOAT_TO_ADC(VMAX_FLOAT_MIN)
        && param_adc_value <= VMAX_FLOAT_TO_ADC(VMAX_FLOAT_MAX))
      {
        v_max = param_adc_value;
        process_poll(&bang_process);
      }
    break;

    }
}

//Return the value of the selected parameter
  float
  dc_converter_get_control_parameter(int param_id)
  {
    float param_float_value = 0.0;
    switch (param_id)
      {
    case CONV_VREF:
      param_float_value = VREF_ADC_TO_FLOAT(v_ref);
      break;

    case CONV_IMAX:
      param_float_value = IMAX_ADC_TO_FLOAT(i_max);
      break;

    case CONV_VMAX:
      param_float_value = VMAX_ADC_TO_FLOAT(v_max);
      break;
      }
    return param_float_value;
  }

  /* Matrix of converter state VS state of the switches
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
  dc_converter_set_algorithm_state(int new_conv_state)
  {
    switch ((CONV_STATE)new_conv_state)
      {

    case CONV_BUCK_OFF:
      switches_set_gate_state(SW_NGATE1, SW_ON);
      switches_set_gate_state(SW_PGATE1, SW_OFF);
      switches_set_gate_state(SW_NGATE2, SW_OFF);
      switches_set_gate_state(SW_PGATE2, SW_ON);
      converter_state = (CONV_STATE)new_conv_state;
      break;

    case CONV_BUCK_SOFT:
      switches_set_gate_state(SW_NGATE1, SW_OFF);
      switches_set_gate_state(SW_PGATE1, SW_OFF);
      switches_set_gate_state(SW_NGATE2, SW_OFF);
      switches_set_gate_state(SW_PGATE2, SW_ON);
      converter_state = (CONV_STATE)new_conv_state;
      break;

    case CONV_BUCK_ON:
      switches_set_gate_state(SW_NGATE1, SW_OFF);
      switches_set_gate_state(SW_PGATE1, SW_ON);
      switches_set_gate_state(SW_NGATE2, SW_OFF);
      switches_set_gate_state(SW_PGATE2, SW_ON);
      converter_state = (CONV_STATE)new_conv_state;
      break;

    case CONV_BOOST_OFF:
      switches_set_gate_state(SW_NGATE1, SW_OFF);
      switches_set_gate_state(SW_PGATE1, SW_ON);
      switches_set_gate_state(SW_NGATE2, SW_OFF);
      switches_set_gate_state(SW_PGATE2, SW_ON);
      converter_state = (CONV_STATE)new_conv_state;
      break;

    case CONV_BOOST_SOFT:
      switches_set_gate_state(SW_NGATE1, SW_OFF);
      switches_set_gate_state(SW_PGATE1, SW_ON);
      switches_set_gate_state(SW_NGATE2, SW_OFF);
      switches_set_gate_state(SW_PGATE2, SW_OFF);
      converter_state = (CONV_STATE)new_conv_state;
      break;

    case CONV_BOOST_ON:
      switches_set_gate_state(SW_NGATE1, SW_OFF);
      switches_set_gate_state(SW_PGATE1, SW_ON);
      switches_set_gate_state(SW_NGATE2, SW_ON);
      switches_set_gate_state(SW_PGATE2, SW_OFF);
      converter_state = (CONV_STATE)new_conv_state;
      break;

    case CONV_DISCHARGE:
      switches_set_gate_state(SW_NGATE1, SW_ON);
      switches_set_gate_state(SW_PGATE1, SW_OFF);
      switches_set_gate_state(SW_NGATE2, SW_ON);
      switches_set_gate_state(SW_PGATE2, SW_OFF);
      converter_state = (CONV_STATE)new_conv_state;
      break;

    case CONV_ALL_OFF:
      switches_set_gate_state(SW_NGATE1, SW_OFF);
      switches_set_gate_state(SW_PGATE1, SW_OFF);
      switches_set_gate_state(SW_NGATE2, SW_OFF);
      switches_set_gate_state(SW_PGATE2, SW_OFF);
      converter_state = (CONV_STATE)new_conv_state;
      break;
      }

  }

//Return the state of the control algorithm
  int
  dc_converter_get_algorithm_state()
  {
    return (int) converter_state;
  }

//Return the state of the control algorithm as a string
//(defined as a static char array inside function)
  char *
  dc_converter_get_algorithm_state_string()
  {
    char * str_ptr = NULL;
    switch (converter_state)
      {

    case CONV_BUCK_OFF:
      {
        static char buck_off_string[] = "BUCK_OFF";
        str_ptr = buck_off_string;
        break;
      }

    case CONV_BUCK_SOFT:
      {
        static char buck_soft_string[] = "BUCK_SOFT";
        str_ptr = buck_soft_string;
        break;
      }

    case CONV_BUCK_ON:
      {
        static char buck_on_string[] = "BUCK_ON";
        str_ptr = buck_on_string;
        break;
      }

    case CONV_BOOST_OFF:
      {
        static char boost_off_string[] = "BOOST_OFF";
        str_ptr = boost_off_string;
        break;
      }

    case CONV_BOOST_SOFT:
      {
        static char boost_soft_string[] = "BOOST_SOFT";
        str_ptr = boost_soft_string;
        break;
      }

    case CONV_BOOST_ON:
      {
        static char boost_on_string[] = "BOOST_ON";
        str_ptr = boost_on_string;
        break;
      }

    case CONV_DISCHARGE:
      {
        static char discharge_string[] = "DISCHARGE";
        str_ptr = discharge_string;
        break;
      }

    case CONV_ALL_OFF:
      {
        static char all_off_string[] = "ALL_OFF";
        str_ptr = all_off_string;
        break;
      }
      }

    return str_ptr;

  }

  //This is the process poll handler and
  //the bang-bang algorithm main loop
  static void pollhandler()
  {
    //Parameters used to store the value of the state vector variables
    int v_out, v_in, i_l;

    //Get the current value of the ADC parameters
    v_out = svector_sensor.value(SVECTOR_SENSOR_VOUT);
    v_in = svector_sensor.value(SVECTOR_SENSOR_VIN);
    i_l = svector_sensor.value(SVECTOR_SENSOR_IL);

    if (!algorithm_configured || !user_allowed || v_ref <= 0)
      {
        dc_converter_set_algorithm_state(CONV_ALL_OFF);
      }
    else if (v_out <= v_max)
      {
        dc_converter_set_algorithm_state(CONV_DISCHARGE);
      }
    else if (i_l > i_max)
      {
        if (v_out > v_in)
          dc_converter_set_algorithm_state(CONV_BOOST_OFF);
        else
          dc_converter_set_algorithm_state(CONV_BUCK_OFF);
      }
    else if (v_ref <= v_in)
      {
        if (v_out <= v_ref)
          dc_converter_set_algorithm_state(CONV_BOOST_ON);
        else if (i_l <= 0)
          dc_converter_set_algorithm_state(CONV_BOOST_SOFT);
        else
          dc_converter_set_algorithm_state(CONV_BOOST_OFF);
      }
    else if (v_out <= v_ref)
      {
        dc_converter_set_algorithm_state(CONV_BUCK_ON);
      }
    else if (i_l <= 0)
      {
        dc_converter_set_algorithm_state(CONV_BUCK_SOFT);
      }
    else
      {
        dc_converter_set_algorithm_state(CONV_BUCK_OFF);
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
      switches_init();
      //Set the starting state of the algorithm
      dc_converter_set_algorithm_state(CONV_ALL_OFF);
      //Set the parameters to their default values
      v_max = VMAX_FLOAT_TO_ADC(VMAX_FLOAT_DEFAULT);
      i_max = IMAX_FLOAT_TO_ADC(IMAX_FLOAT_DEFAULT);
      v_ref = VREF_FLOAT_TO_ADC(VREF_FLOAT_DEFAULT);
      algorithm_configured = TRUE;

      //Start the ADC sensors and the sensor controlling process
      printf("Starting the Contiki sensor process\n");
      process_start(&sensors_process, NULL );
      //Wait for the ADC to settle before starting to sample
      static struct etimer et;
      etimer_set(&et, CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      printf("Activating the sensors\n");
      SVECTOR_SENSOR_ACTIVATE();

      //Do nothing, all the real control stuff
      //is in the poll handler so it can be invoked
      //by the ADC interruption
      PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

    PROCESS_END();
}
