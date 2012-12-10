/*
 * adc-sensor.c
 *
 *  Created on: Dec 5, 2012
 *      Author: cazulu
 */

#include "adc-sensors.h"
#include "lpc17xx_adc.h"
#include <stdio.h>

//Data structures for the Contiki sensor API
//TODO: Maybe put these in a separate file
const struct sensors_sensor *sensors[] = {
  &vout_sensor,
  &vin_sensor,
  &il_sensor,
  NULL
};
unsigned char sensors_flags[(sizeof(sensors) / sizeof(struct sensors_sensor *))];

//Variable to avoid intializing the ADC twice
static Bool isAdcInitialized = FALSE;

//Current status of the ADC channels
static int adcChannels[4];

void ADC_IRQHandler(void) __attribute__ ((interrupt));
static void initAdcModule();

//Variables for the ADC interruption
static uint32_t adgr;
static uint8_t channel;
static int channelCounter[4]={0,0,0,0};
static uint8_t adcFlags=0;

//Interrupt handler, stores the channel values into the adcChannels array
void ADC_IRQHandler(void){
  if(ADC_GlobalGetStatus(LPC_ADC, ADC_DATA_DONE)
      || ADC_GlobalGetStatus(LPC_ADC, ADC_DATA_BURST)){
      //Get and store the value of the latest channel converted
      //NOTE: There is an error in the ADC_GlobalGetData function,
      //it returns the whole ADGR register and not just the RESULT field
      adgr=ADC_GlobalGetData(LPC_ADC);
      channel=ADC_GDR_CH(adgr);
      adcChannels[channel]=ADC_GDR_RESULT(adgr);

      if(channelCounter[channel]==10000){
          printf("The value of channel %d is %d\n", channel, adcChannels[channel]);
          channelCounter[channel]=0;
      }
      else
        channelCounter[channel]++;
  }
}

static void initAdcModule(){
  if(!isAdcInitialized){
      ADC_Init(LPC_ADC, ADC_CONV_RATE);
      isAdcInitialized = TRUE;
      NVIC_DisableIRQ(ADC_IRQn);
      /* Set ADC interruption priority: preemption = 1, sub-priority = 1 */
      NVIC_SetPriority(ADC_IRQn, ((0x01<<3)|0x01));
      //Only the global DONE flag of the ADDR
      //will generate an interrupt
      ADC_IntConfig(LPC_ADC,ADC_ADGINTEN,SET);
      printf("ADC module properly initialized\n");
  }
}

/*---------------------------------------------------------------------------*/
static int
value_vout(int type)
{
  uint16_t reading;
  NVIC_DisableIRQ(ADC_IRQn);
  reading=adcChannels[VOUT_ADC_CHANNEL];
  NVIC_EnableIRQ(ADC_IRQn);
  return reading;
}

static int
value_vin(int type)
{
  uint16_t reading;
  NVIC_DisableIRQ(ADC_IRQn);
  reading=adcChannels[VIN_ADC_CHANNEL];
  NVIC_EnableIRQ(ADC_IRQn);
  return reading;
}

static int
value_il(int type)
{
  uint16_t reading;
  NVIC_DisableIRQ(ADC_IRQn);
  reading=adcChannels[VIOUT_ADC_CHANNEL]-adcChannels[VZCR_ADC_CHANNEL];
  NVIC_EnableIRQ(ADC_IRQn);
  return reading;
}
/*---------------------------------------------------------------------------*/
static int
status_vout(int type)
{
  return ADC_ChannelGetStatus(LPC_ADC, VOUT_ADC_CHANNEL, ADC_DATA_DONE);
}

static int
status_vin(int type)
{
  return ADC_ChannelGetStatus(LPC_ADC, VIN_ADC_CHANNEL, ADC_DATA_DONE);
}

static int
status_il(int type)
{
  return ADC_ChannelGetStatus(LPC_ADC, VZCR_ADC_CHANNEL, ADC_DATA_DONE)
      & ADC_ChannelGetStatus(LPC_ADC, VIOUT_ADC_CHANNEL, ADC_DATA_DONE);
}

/*---------------------------------------------------------------------------*/
static int
configure_vout(int type, int value)
{
  switch (type)
    {
  //Initialize the ADC used to get the output voltage
  //values from the voltage divider
  case SENSORS_HW_INIT:
    /* Initialize ADC module */
    initAdcModule();
    /* Initialize ADC pin */
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = PINSEL_FUNC_0;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    PinCfg.Pinnum = VOUT_PIN;
    PinCfg.Portnum = VOUT_PORT;
    PINSEL_ConfigPin(&PinCfg);

    printf("Vout sensor properly initialized\n");

    break;

  case SENSORS_ACTIVE:
    //Enable or disable the ADC channel depending
    //on the value variable
    if (value)
      {
        //Enable the ADC channel
        ADC_ChannelCmd(LPC_ADC, VOUT_ADC_CHANNEL, ENABLE);
        //Start burst conversion
        ADC_BurstCmd(LPC_ADC,ENABLE);
        /* Enable ADC in NVIC */
        NVIC_EnableIRQ(ADC_IRQn);
      }
    else{
        ADC_ChannelCmd(LPC_ADC, VOUT_ADC_CHANNEL, DISABLE);
        ADC_IntConfig(LPC_ADC,VOUT_ADC_CHANNEL,DISABLE);
    }
    break;
    }
  return 1;
}

static int
configure_vin(int type, int value)
{
  switch (type)
    {
  //Initialize the ADC used to get the input voltage
  //values from the voltage divider
  case SENSORS_HW_INIT:
    /* Initialize ADC module */
    initAdcModule();
    /* Initialize ADC pin */
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = PINSEL_FUNC_0;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    PinCfg.Pinnum = VIN_PIN;
    PinCfg.Portnum = VIN_PORT;
    PINSEL_ConfigPin(&PinCfg);

    printf("Vin sensor properly initialized\n");
    break;

  case SENSORS_ACTIVE:
    //Enable or disable the ADC channel depending
    //on the value variable
    if (value)
      {
        //Enable the ADC channel
        ADC_ChannelCmd(LPC_ADC, VIN_ADC_CHANNEL, ENABLE);
        //Start burst conversion
        ADC_BurstCmd(LPC_ADC,ENABLE);
        /* Enable ADC in NVIC */
        NVIC_EnableIRQ(ADC_IRQn);
      }
    else{
        ADC_ChannelCmd(LPC_ADC, VIN_ADC_CHANNEL, DISABLE);
        ADC_IntConfig(LPC_ADC, VIN_ADC_CHANNEL,DISABLE);
    }
    break;
    }
  return 1;
}

static int
configure_il(int type, int value)
{
  switch (type)
    {
  //Initialize the two ADCs used to get the inductor current
  //values from the ACS710, derived from Vzcr and Viout
  case SENSORS_HW_INIT:
    /* Initialize ADC module */
    initAdcModule();

    PINSEL_CFG_Type PinCfg;
    //CURRENT SENSOR VZCR
    /* Initialize ADC pin */
    PinCfg.Funcnum = PINSEL_FUNC_0;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    PinCfg.Pinnum = VZCR_PIN;
    PinCfg.Portnum = VZCR_PORT;
    PINSEL_ConfigPin(&PinCfg);

    //CURRENT SENSOR VIOUT
    /* Initialize ADC pin */
    PinCfg.Funcnum = PINSEL_FUNC_0;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    PinCfg.Pinnum = VIOUT_PIN;
    PinCfg.Portnum = VIOUT_PORT;
    PINSEL_ConfigPin(&PinCfg);

    printf("Current sensor properly initialized\n");

    break;

  case SENSORS_ACTIVE:
    //Enable or disable the ADC channel depending
    //on the value variable
    if (value)
      {
        //Enable the ADC channels
        ADC_ChannelCmd(LPC_ADC, VZCR_ADC_CHANNEL, ENABLE);
        ADC_ChannelCmd(LPC_ADC, VIOUT_ADC_CHANNEL, ENABLE);
        //Start burst conversion
        ADC_BurstCmd(LPC_ADC,ENABLE);
        /* Enable ADC in NVIC */
        NVIC_EnableIRQ(ADC_IRQn);
      }
    else{
        ADC_ChannelCmd(LPC_ADC, VZCR_ADC_CHANNEL, DISABLE);
        ADC_ChannelCmd(LPC_ADC, VIOUT_ADC_CHANNEL, DISABLE);
        ADC_IntConfig(LPC_ADC,VZCR_ADC_CHANNEL,DISABLE);
        ADC_IntConfig(LPC_ADC,VIOUT_ADC_CHANNEL,DISABLE);
    }
    break;
    }
  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(vout_sensor, VOUT_SENSOR, value_vout, configure_vout,
    status_vout);
SENSORS_SENSOR(vin_sensor, VIN_SENSOR, value_vin, configure_vin, status_vin);
SENSORS_SENSOR(il_sensor, IL_SENSOR, value_il, configure_il, status_il);
