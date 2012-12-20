/*
 * Copyright (c) 2012, KTH, Royal Institute of Technology(Stockholm, Sweden)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * This is work by the CSD master project. Fall 2012. Microgrid team.
 * Author: Javier Lara Peinado <javierlp@kth.se>
 *
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
#define VMAX_FLOAT_DEFAULT    20.0
#define VMAX_FLOAT_MAX        30.0
#define IMAX_FLOAT_MIN        0.0
#define IMAX_FLOAT_DEFAULT    1.1
#define IMAX_FLOAT_MAX        6.0


//Function to set the parameters used by the DC-DC converter
//control algorithm:
//      -Vref:Desired output voltage
//      -Imax:Maximum inductor current allowed
//      -Vmax:Maximum output voltage allowed
void dc_converter_set_control_parameter(int paramId, float paramValue);

//Return the value of the selected parameter
float dc_converter_get_control_parameter(int paramId);

//Change the state of the algorithm
//and modify the switches accordingly
  void dc_converter_set_algorithm_state(int newConvState);

//Return the state of the control algorithm as an integer
int dc_converter_get_algorithm_state();

//Return the state of the control algorithm as a string
//(defined as a static char array inside function)
char *dc_converter_get_algorithm_state_string();

//Enable the voltage output for this user
void dc_converter_allow_user();

//Cut the user off from the grid
void dc_converter_forbid_user();

//Returns 0 if the user does not have access
//to the power grid
int dc_converter_get_user_status();



#endif /* BANG_CONTROL_H_ */
