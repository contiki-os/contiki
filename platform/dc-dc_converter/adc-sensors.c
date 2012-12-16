/*
 * adc-sensor.c
 *
 *  Created on: Dec 5, 2012
 *      Author: cazulu
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
static uint16_t adcChannels[4];

void
ADCRead(unsigned char ADC)
{
  LPC_ADC ->ADCR &= ~(0x000000FF);                 // Remove ADC selected
  LPC_ADC ->ADCR |= (1 << ADC);                    // Select ADC
  LPC_ADC ->ADCR |= (1 << 24);                             // Start conversion
}

void
ADC_IRQHandler(void) __attribute__ ((interrupt));

//Variables for the ADC interruption
static uint8_t currentChannel=0;

//Interrupt handler, stores the channel values into the adcChannels array
void
ADC_IRQHandler(void)
{
  switch(currentChannel){
  case VOUT_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VOUT_ADC_CHANNEL,ADC_DATA_DONE)){
        adcChannels[VOUT_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VOUT_ADC_CHANNEL);
        currentChannel=VIN_ADC_CHANNEL;
    }
    break;

  case VIN_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VIN_ADC_CHANNEL,ADC_DATA_DONE)){
        adcChannels[VIN_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VIN_ADC_CHANNEL);
        currentChannel=VZCR_ADC_CHANNEL;
    }
    break;

  case VZCR_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VZCR_ADC_CHANNEL,ADC_DATA_DONE)){
        adcChannels[VZCR_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VZCR_ADC_CHANNEL);
        currentChannel=VIOUT_ADC_CHANNEL;
    }
    break;

  case VIOUT_ADC_CHANNEL:
    if(ADC_ChannelGetStatus(LPC_ADC, VIOUT_ADC_CHANNEL,ADC_DATA_DONE)){
        adcChannels[VIOUT_ADC_CHANNEL]=ADC_ChannelGetData(LPC_ADC, VIOUT_ADC_CHANNEL);
        currentChannel=VOUT_ADC_CHANNEL;
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
getFloatParameter(int paramType)
{
  float floatValue = 0.0;
  switch (paramType)
    {
  case SVECTOR_SENSOR_VOUT:
    floatValue = VOUT_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_VOUT));
    break;

  case SVECTOR_SENSOR_VIN:
    floatValue = VIN_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_VIN));
    break;

  case SVECTOR_SENSOR_IL:
    floatValue = IL_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_IL));
    break;

  case SVECTOR_SENSOR_IIN:
    floatValue = IIN_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_IIN));
    break;

  case SVECTOR_SENSOR_IOUT:
    floatValue = IOUT_ADC_TO_FLOAT(svector_sensor.value(SVECTOR_SENSOR_IOUT));
    break;
    }
  return floatValue;
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
    reading = adcChannels[VOUT_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_VIN:
    reading = adcChannels[VIN_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_IL:
    reading = adcChannels[VIOUT_ADC_CHANNEL] - adcChannels[VZCR_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_IIN:
    reading = adcChannels[VIOUT_ADC_CHANNEL] - adcChannels[VZCR_ADC_CHANNEL];
    break;

  case SVECTOR_SENSOR_IOUT:
    reading = adcChannels[VIOUT_ADC_CHANNEL] - adcChannels[VZCR_ADC_CHANNEL];
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
     PINSEL_CFG_Type PinCfg;
     PinCfg.Funcnum = VOUT_PINFUNC;
     PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     PinCfg.Pinnum = VOUT_PIN;
     PinCfg.Portnum = VOUT_PORT;
     PINSEL_ConfigPin(&PinCfg);
     //VIN ADC pin
     PinCfg.Funcnum = VIN_PINFUNC;
     PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     PinCfg.Pinnum = VIN_PIN;
     PinCfg.Portnum = VIN_PORT;
     PINSEL_ConfigPin(&PinCfg);
     //VZCR ADC pin
     PinCfg.Funcnum = VZCR_PINFUNC;
     PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     PinCfg.Pinnum = VZCR_PIN;
     PinCfg.Portnum = VZCR_PORT;
     PINSEL_ConfigPin(&PinCfg);
     //VIOUT ADC pin
     PinCfg.Funcnum = VIOUT_PINFUNC;
     PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
     PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
     PinCfg.Pinnum = VIOUT_PIN;
     PinCfg.Portnum = VIOUT_PORT;
     PINSEL_ConfigPin(&PinCfg);

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
