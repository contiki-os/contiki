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

#include "contiki.h"
#include "adc-sensors.h"
#include "bang-control.h"
#include "lpc17xx_adc.h"

//TODO: Implement a glitch filter for the ADC values

//Data structures for the Contiki sensor API
const struct sensors_sensor *sensors[] =
  { &svector_sensor, NULL };
unsigned char sensors_flags[(sizeof(sensors) / sizeof(struct sensors_sensor *))];

//Current status of the ADC channels
static uint16_t adc_channels[4];

//Number of the channel currently being processed by the ADC
static uint8_t current_adc_channel=VOUT_ADC_CHANNEL;

void
ADC_IRQHandler(void) __attribute__ ((interrupt));

//Interrupt handler, stores the channel values into the adc_channels array
void
ADC_IRQHandler(void)
{
  switch(current_adc_channel){
  case VOUT_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VOUT_ADC_CHANNEL,ADC_DATA_DONE)){
        adc_channels[VOUT_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VOUT_ADC_CHANNEL);
        current_adc_channel=VIN_ADC_CHANNEL;
    }
    break;

  case VIN_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VIN_ADC_CHANNEL,ADC_DATA_DONE)){
        adc_channels[VIN_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VIN_ADC_CHANNEL);
        current_adc_channel=VZCR_ADC_CHANNEL;
    }
    break;

  case VZCR_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VZCR_ADC_CHANNEL,ADC_DATA_DONE)){
        adc_channels[VZCR_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VZCR_ADC_CHANNEL);
        current_adc_channel=VIOUT_ADC_CHANNEL;
    }
    break;

  case VIOUT_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VIOUT_ADC_CHANNEL,ADC_DATA_DONE)){
        adc_channels[VIOUT_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VIOUT_ADC_CHANNEL);
        current_adc_channel=VOUT_ADC_CHANNEL;
        //Poll the control algorithm to inform of the new values
        process_poll(&bang_process);
    }
    break;
  }

  /*
  if (ADC_GlobalGetStatus(LPC_ADC, ADC_DATA_DONE)
      || ADC_GlobalGetStatus(LPC_ADC, ADC_DATA_BURST))
    {


//       //Get and store the value of the latest channel converted
//       //NOTE: There is an error in the ADC_GlobalGetData function,
//       //it returns the whole ADGR register and not just the RESULT field
//       adgr = ADC_GlobalGetData(LPC_ADC );
//       channel = ADC_GDR_CH(adgr);
//       adcChannels[channel] = ADC_GDR_RESULT(adgr);



//      int Channel;
//      Channel = (LPC_ADC ->ADGDR >> 24) & 0x00000007;
//      adcChannels[Channel] = (LPC_ADC ->ADGDR >> 4) & 0x00000FFF;
//      Channel++;
//      if (Channel < 4)
//        ADCRead((channel + 1) % 4);

      channelCounter++;
      if (channelCounter == 50000)
        {
          channelCounter=0;
          printf("***Printing current ADC0 values***\n");
          printf("\t->AD0GDR=%x\n", adgr);
          printf("\t->ADC0[0]=%u\n", adcChannels[0]);
          printf("\t->ADC0[1]=%u\n", adcChannels[1]);
          printf("\t->ADC0[2]=%u\n", adcChannels[2]);
          printf("\t->ADC0[3]=%u\n", adcChannels[3]);
        }

      //Poll the control algorithm to inform of the new value
      if (channel == VIOUT_ADC_CHANNEL)
        {
          //process_poll(&bang_process);
        }
    }
    */
}

//Function to convert the ADC 12-bit values to floating numbers in SI units(V,A)
float
dc_converter_get_svector_parameter(int param_id)
{
  float param_float_value = 0.0;
  switch (param_id)
    {
  case SVECTOR_SENSOR_VOUT:
    param_float_value = VOUT_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_VOUT));
    break;

  case SVECTOR_SENSOR_VIN:
    param_float_value = VIN_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_VIN));
    break;

  case SVECTOR_SENSOR_IL:
    param_float_value = IL_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_IL));
    break;

  case SVECTOR_SENSOR_IIN:
    param_float_value = IIN_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_IIN));
    break;

  case SVECTOR_SENSOR_IOUT:
    param_float_value = IOUT_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_IOUT));
    break;
    }
  return param_float_value;
}

/*---------------------------------------------------------------------------*/
static int
value_svector(int type)
{
  int reading;
  NVIC_DisableIRQ(ADC_IRQn);
  switch (type)
    {
  case SVECTOR_SENSOR_VOUT:
    reading = adc_channels[VOUT_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_VIN:
    reading = adc_channels[VIN_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_IL:
    reading = adc_channels[VIOUT_ADC_CHANNEL] - adc_channels[VZCR_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_IIN:
    reading = adc_channels[VIOUT_ADC_CHANNEL] - adc_channels[VZCR_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_IOUT:
    reading = adc_channels[VIOUT_ADC_CHANNEL] - adc_channels[VZCR_ADC_CHANNEL];
    break;

  default:
    reading = 0;
    break;
    }
  NVIC_EnableIRQ(ADC_IRQn);
  return reading;
}
/*---------------------------------------------------------------------------*/
static int
status_svector(int type)
{
  int16_t status;

  switch (type)
    {
  case SVECTOR_SENSOR_VOUT:
    status = ADC_ChannelGetStatus(LPC_ADC, VOUT_ADC_CHANNEL, ADC_DATA_DONE);
    break;

  case SVECTOR_SENSOR_VIN:
    status = ADC_ChannelGetStatus(LPC_ADC, VIN_ADC_CHANNEL, ADC_DATA_DONE);
    break;

  case SVECTOR_SENSOR_IL:
    status = ADC_ChannelGetStatus(LPC_ADC, VZCR_ADC_CHANNEL, ADC_DATA_DONE)
        & ADC_ChannelGetStatus(LPC_ADC, VIOUT_ADC_CHANNEL, ADC_DATA_DONE);
    break;

  default:
    status = 0;
    break;
    }
  return status;
}

/*---------------------------------------------------------------------------*/
static int
configure_svector(int type, int value)
{
  switch (type)
    {
  case SENSORS_HW_INIT:

     //Initialize the ADC module
     ADC_Init(LPC_ADC, ADC_CONV_RATE);
     NVIC_DisableIRQ(ADC_IRQn);
     //Set ADC interruption to the highest priority
     NVIC_SetPriority(ADC_IRQn, 0);

     //Initialize the ADC pins
     //VOUT ADC pin
     PINSEL_CFG_Type pincfg;
     pincfg.Funcnum = VOUT_PINFUNC;
     pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     pincfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     pincfg.Pinnum = VOUT_PIN;
     pincfg.Portnum = VOUT_PORT;
     PINSEL_ConfigPin(&pincfg);
     //VIN ADC pin
     pincfg.Funcnum = VIN_PINFUNC;
     pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     pincfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     pincfg.Pinnum = VIN_PIN;
     pincfg.Portnum = VIN_PORT;
     PINSEL_ConfigPin(&pincfg);
     //VZCR ADC pin
     pincfg.Funcnum = VZCR_PINFUNC;
     pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     pincfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     pincfg.Pinnum = VZCR_PIN;
     pincfg.Portnum = VZCR_PORT;
     PINSEL_ConfigPin(&pincfg);
     //VIOUT ADC pin
     pincfg.Funcnum = VIOUT_PINFUNC;
     pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     pincfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     pincfg.Pinnum = VIOUT_PIN;
     pincfg.Portnum = VIOUT_PORT;
     PINSEL_ConfigPin(&pincfg);

    break;

  case SENSORS_ACTIVE:
    //Enable or disable the ADC channels depending
    //on the value variable
    if (value)
      {
        //Disable the interrupt for the global ADC data register
        ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, DISABLE);
         //Enable the different ADC channels and their interrupts
         ADC_ChannelCmd(LPC_ADC, VOUT_ADC_CHANNEL, ENABLE);
         ADC_IntConfig(LPC_ADC, VOUT_ADC_CHANNEL, ENABLE);
         ADC_ChannelCmd(LPC_ADC, VIN_ADC_CHANNEL, ENABLE);
         ADC_IntConfig(LPC_ADC, VIN_ADC_CHANNEL, ENABLE);
         ADC_ChannelCmd(LPC_ADC, VZCR_ADC_CHANNEL, ENABLE);
         ADC_IntConfig(LPC_ADC, VZCR_ADC_CHANNEL, ENABLE);
         ADC_ChannelCmd(LPC_ADC, VIOUT_ADC_CHANNEL, ENABLE);
         ADC_IntConfig(LPC_ADC, VIOUT_ADC_CHANNEL, ENABLE);
         //Start burst conversion
         ADC_BurstCmd(LPC_ADC,ENABLE);

         //Enable ADC interruptions
         NVIC_EnableIRQ(ADC_IRQn);


        /*
        NVIC_EnableIRQ(ADC_IRQn);
        NVIC_SetPriority(ADC_IRQn, 2);
        ADCRead(0);
        */
      }
    else
      {
        //Disable the ADC interruptions
        NVIC_DisableIRQ(ADC_IRQn);
        //Stop burst conversion
        ADC_BurstCmd(LPC_ADC, DISABLE);
        //Disable the ADC channels
        ADC_ChannelCmd(LPC_ADC, VOUT_ADC_CHANNEL, DISABLE);
        ADC_ChannelCmd(LPC_ADC, VIN_ADC_CHANNEL, DISABLE);
        ADC_ChannelCmd(LPC_ADC, VZCR_ADC_CHANNEL, DISABLE);
        ADC_ChannelCmd(LPC_ADC, VIOUT_ADC_CHANNEL, DISABLE);
      }
    break;

    }
  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(svector_sensor, SVECTOR_SENSOR, value_svector, configure_svector,
    status_svector);
