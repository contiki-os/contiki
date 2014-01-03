/*
 * Copyright (c) 2013, Kerlink
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
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
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
 */
/**
 * \addtogroup efm32-devices
 * @{
 */
/**
 * \file
 *         EFM32 LESENSE driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include <stdlib.h>

#include "lesense.h"
#include "burtc.h"

#include "em_device.h"
#include "em_cmu.h"
#include "em_lesense.h"

#include "em_acmp.h"
#include "em_pcnt.h"
#include "em_prs.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

//#define DEBUG_LESENSE_STATE 1

///// TODO : Separate platform config and general pcount config


#define PCNT0_TOP_VALUE 0xFFFF

static pulsealarm_callback_t _pulsealarm_callback = NULL;

/**************************************************************************//**
 * @brief  Setup the ACMP
 *****************************************************************************/
void setupACMP(void)
{
  /* ACMP configuration constant table. */
  static const ACMP_Init_TypeDef initACMP =
  {
    .fullBias = false,                 /* fullBias */
    .halfBias = true,                  /* halfBias */
    .biasProg =  0x0,                  /* biasProg */
    .interruptOnFallingEdge =  false,  /* interrupt on rising edge */
    .interruptOnRisingEdge =  false,   /* interrupt on falling edge */
    //.interruptOnFallingEdge =  true,  /* interrupt on rising edge */
    //.interruptOnRisingEdge =  true,   /* interrupt on falling edge */
    .warmTime = acmpWarmTime4,       /* 512 cycle warmup to be safe */
    .hysteresisLevel = acmpHysteresisLevel0, /* No Hysteresis */
    .inactiveValue = false,            /* inactive value */
    .lowPowerReferenceEnabled = true, /* low power reference */
    .vddLevel = 32,                  /* VDD level : VDD/2 */
    .enable = false                    /* Don't request enabling. */
  };


  /* Configure ACMP. */
  ACMP_Init(ACMP1, &initACMP);
  /* Disable ACMP0 out to a pin. */
  ACMP_GPIOSetup(ACMP1, 0, false, false);
  /* Set up ACMP negSel to VDD, posSel is controlled by LESENSE. */
  ACMP_ChannelSet(ACMP1, acmpChannelVDD, acmpChannel0);
  /* LESENSE controls ACMP thus ACMP_Enable(ACMP0) should NOT be called in order
   * to ensure lower current consumption. */

#ifdef DEBUG_ACMP
  ACMP_ChannelSet(ACMP1, acmpChannelVDD, acmpChannel2);
  ACMP1->IEN = 1;
  ACMP_Enable(ACMP1);
  NVIC_ClearPendingIRQ(ACMP0_IRQn);
  NVIC_EnableIRQ(ACMP0_IRQn);
#endif
}

#ifdef DEBUG_ACMP
// ACMP IRQ Handler is shared between ACMP0 and ACMP1
void ACMP0_IRQHandler(void)
{
  PRINTF("%s\r\n",__func__);
  ACMP1->IFC = 1;
  NVIC_ClearPendingIRQ(ACMP0_IRQn);
}
#endif

/**************************************************************************//**
 * @brief  Setup the LESENSE
 *****************************************************************************/
void setupLESENSE(void)
{
  /* LESENSE channel configuration constant table. */
  static const LESENSE_ChAll_TypeDef initChs = LESENSE_SCAN_CONF;
  /* LESENSE alternate excitation channel configuration constant table. */
  static const LESENSE_ConfAltEx_TypeDef initAltEx = LESENSE_ALTEX_CONF;

  static const LESENSE_DecStAll_TypeDef initStates = LESENSE_STATES_CONF;
  /* LESENSE central configuration constant table. */
  static const LESENSE_Init_TypeDef initLESENSE =
  {
    .coreCtrl =
    {
      .scanStart = lesenseScanStartPeriodic,
      .prsSel = lesensePRSCh0,
      .scanConfSel = lesenseScanConfDecDef,
      .invACMP0 = false,
      .invACMP1 = false,
      .dualSample = false,
      .storeScanRes = false,
      .bufOverWr = true,
      .bufTrigLevel = lesenseBufTrigHalf,
      .wakeupOnDMA = lesenseDMAWakeUpDisable,
      .biasMode = lesenseBiasModeDutyCycle,
      .debugRun = false
    },

    .timeCtrl =
    {
      .startDelay = 0
    },

    .perCtrl =
    {
      .dacCh0Data = lesenseDACIfData,
      .dacCh0ConvMode = lesenseDACConvModeDisable,
      .dacCh0OutMode = lesenseDACOutModeDisable,
      .dacCh1Data = lesenseDACIfData,
      .dacCh1ConvMode = lesenseDACConvModeDisable,
      .dacCh1OutMode = lesenseDACOutModeDisable,
      .dacPresc = 0U,
      .dacRef = lesenseDACRefBandGap,
      .acmp0Mode = lesenseACMPModeMuxThres,
      .acmp1Mode = lesenseACMPModeMuxThres,
      .warmupMode = lesenseWarmupModeNormal
    },

    .decCtrl =
    {
      .decInput = lesenseDecInputSensorSt,
      .initState = 0U,
      .chkState = false,
      .intMap = true,
      .hystPRS0 = false,
      .hystPRS1 = false,
      .hystPRS2 = false,
      .hystIRQ = false,
      .prsCount = true,
      .prsChSel0 = lesensePRSCh0,
      .prsChSel1 = lesensePRSCh1,
      .prsChSel2 = lesensePRSCh2,
      .prsChSel3 = lesensePRSCh3
    }
  };


  /* Initialize LESENSE interface with RESET. */
  LESENSE_Init(&initLESENSE, true); // struct size = 64

  /* Configure scan channels. */
  LESENSE_ChannelAllConfig(&initChs); // struct size = 512

  /* Configure alternate excitation channels. */
  LESENSE_AltExConfig(&initAltEx); // struct size = 49

  /* COnfigure State Machine */
  LESENSE_DecoderStateAllConfig(&initStates); // struct size = 288

  /* Set scan frequency (in Hz). */
  (void)LESENSE_ScanFreqSet(0, 16);
  //(void)LESENSE_ScanFreqSet(0, 8);
  //(void)LESENSE_ScanFreqSet(0, 1);

  /* Set clock divisor for LF clock :
   * power on pulse = 2 ms -> 512Hz min
   * Excite and measure delay must be >= 2 clkLesense period
   * -> 1024 Hz ->  4096 / 4
   */
  LESENSE_ClkDivSet(lesenseClkLF, lesenseClkDiv_4);
  //LESENSE_ClkDivSet(lesenseClkHF, lesenseClkDiv_128);

  /* Start scanning LESENSE channels. */
  LESENSE_ScanStart();
  LESENSE_DecoderStart();
}


/**************************************************************************//**
 * @brief  Setup the PRS
 *****************************************************************************/
void setupPRS(void)
{
#ifdef DEBUG
  /* Use PRS location 1 and output PRS channel 0 on GPIO PF3 and 1 on PF4 */
  PRS->ROUTE = (1<<8) | 0x3;
  gpio_set_output_pushpull(GPIO_PF3,0);
#endif

  /* PRS channel 0 configuration. */
  PRS_SourceAsyncSignalSet(0,
                           PRS_CH_CTRL_SOURCESEL_LESENSED,
                           PRS_CH_CTRL_SIGSEL_LESENSEDEC0);
  PRS_SourceAsyncSignalSet(1,
                           PRS_CH_CTRL_SOURCESEL_LESENSED,
                           PRS_CH_CTRL_SIGSEL_LESENSEDEC1);
}


/**************************************************************************//**
 * @brief  Setup the PCNT for LESENSE
 *****************************************************************************/
void setupPCNT(void)
{
  /* PCNT configuration constant table. */
  static const PCNT_Init_TypeDef initPCNT =
  {
    .mode = pcntModeOvsSingle, /* Oversampling, single mode. */
    .counter = 0x0, /* Counter value has been initialized to 0. */
    .top = PCNT0_TOP_VALUE, /* Counter top value. */
    .negEdge = false, /* Use positive edge. */
    .countDown = false, /* Up-counting. */
    .filter = false, /* Filter disabled. */
    .hyst = false, /* Hysteresis disabled. */
    .s1CntDir = true, /* Counter direction is given by CNTDIR. */
    .cntEvent = pcntCntEventBoth, /* Regular counter counts up on upcount events. */
    .auxCntEvent = pcntCntEventNone, /* Auxiliary counter doesn't respond to events. */
    .s0PRS = pcntPRSCh0, /* PRS channel 0 selected as S0IN. */
    .s1PRS = pcntPRSCh1  /* PRS channel 1 selected as S1IN. */
  };

  /* Initialize PCNT. */
  PCNT_Init(PCNT0, &initPCNT); // struct size = 24
  /* Enable PRS input S0 in PCNT. */
  PCNT_PRSInputEnable(PCNT0, pcntPRSInputS0, true);
  PCNT_PRSInputEnable(PCNT0, pcntPRSInputS1, true);

  /* Enable the PCNT peripheral. */
  PCNT_Enable(PCNT0, pcntModeOvsSingle);
  /* Enable the PCNT overflow interrupt. */
  PCNT_IntEnable(PCNT0, PCNT_IEN_OF | PCNT_IEN_UF);

  NVIC_ClearPendingIRQ(PCNT0_IRQn);
  NVIC_EnableIRQ(PCNT0_IRQn);
}


/**************************************************************************//**
 * @brief  Setup the PCNT in standalone
 *****************************************************************************/
void setupPCNT_standalone(void)
{
  /* PCNT configuration constant table. */
  static const PCNT_Init_TypeDef initPCNT =
  {
    .mode = pcntModeOvsSingle, /* Oversampling, single mode. */
    .counter = 0x0, /* Counter value has been initialized to 0. */
    .top = PCNT0_TOP_VALUE, /* Counter top value. */
    .negEdge = false, /* Use positive edge. */
    .countDown = false, /* Up-counting. */
    .filter = true, /* Filter disabled. */
    .hyst = false, /* Hysteresis disabled. */
    .s1CntDir = true, /* Counter direction is given by CNTDIR. */
    .cntEvent = pcntCntEventUp, /* Regular counter counts up on upcount events. */
    .auxCntEvent = pcntCntEventNone, /* Auxiliary counter doesn't respond to events. */
    .s0PRS = pcntPRSCh0, /* PRS channel 0 selected as S0IN. */
    .s1PRS = pcntPRSCh1  /* PRS channel 1 selected as S1IN. */
  };

  /* Initialize PCNT. */
  PCNT_Init(PCNT0, &initPCNT); // struct size = 24
  /* Enable PRS input S0 in PCNT. */
  //PCNT_PRSInputEnable(PCNT0, pcntPRSInputS0, true);
  //PCNT_PRSInputEnable(PCNT0, pcntPRSInputS1, true);

  // Enable PRS S0in location 0 (PC13)
  PCNT0->ROUTE = PCNT_ROUTE_LOCATION_LOC0 << _PCNT_ROUTE_LOCATION_SHIFT;

  /* Enable the PCNT peripheral. */
  PCNT_Enable(PCNT0, pcntModeOvsSingle);
  /* Enable the PCNT overflow interrupt. */
  PCNT_IntEnable(PCNT0, PCNT_IEN_OF | PCNT_IEN_UF);

  NVIC_ClearPendingIRQ(PCNT0_IRQn);
  NVIC_EnableIRQ(PCNT0_IRQn);
}

/*---------------------------------------------------------------------------*/
/**
 * Initialize the LESENSE.
 *
 */
void
lesense_init(void)
{
  /* Configure controller */

  // Enable clocks

  // LFA clock started by RTC
  /* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  /* Disable clock source for LFB clock. */
  //CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_Disabled);

  /* Enable HF peripheral clock. */
  CMU_ClockEnable(cmuClock_HFPER, true);
  /* Enable clock for ACMP0. */
  CMU_ClockEnable(cmuClock_ACMP1, true);
  /* Enable clock for PRS. */
  CMU_ClockEnable(cmuClock_PRS, true);
  /* Enable CORELE clock. */
  CMU_ClockEnable(cmuClock_CORELE, true);
  /* Enable clock for PCNT. */
  CMU_ClockEnable(cmuClock_PCNT0, true);


  CMU_ClockEnable(cmuClock_LESENSE, true);
  /* Enable clock divider for LESENSE. Max divider (low power) -> CLKlesense = 4096 Hz*/
  CMU_ClockDivSet(cmuClock_LESENSE, cmuClkDiv_8);


  setupACMP();

  setupPRS();

  setupPCNT();

  setupLESENSE();

#ifdef DEBUG_LESENSE_STATE
  LESENSE_IntEnable(LESENSE_IFC_SCANCOMPLETE);
  NVIC_ClearPendingIRQ(LESENSE_IRQn);
  NVIC_EnableIRQ(LESENSE_IRQn);
#endif
}
/*---------------------------------------------------------------------------*/
void
lesense_init_pcnt_only(void)
{
  /* Configure controller */

  /* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  /* Enable CORELE clock. */
  CMU_ClockEnable(cmuClock_CORELE, true);
  /* Enable clock for PCNT. */
  CMU_ClockEnable(cmuClock_PCNT0, true);

  setupPCNT_standalone();


}
/*---------------------------------------------------------------------------*/


#ifdef DEBUG_LESENSE_STATE
void LESENSE_IRQHandler(void)
{
  PRINTF("Scanres = %lX\r\n",LESENSE_ScanResultGet());
  LESENSE_IntClear(LESENSE_IFC_SCANCOMPLETE);
}
#endif

void PCNT0_IRQHandler(void)
{
  if(PCNT_IntGet(PCNT0) & PCNT_IF_OF)
  {
    PRINTF("PCNT0 Overflow\r\n");
    BURTC_RETREG->s32_pcount += (PCNT_TopGet(PCNT0)+1);
  }

  if(PCNT_IntGet(PCNT0) & PCNT_IF_UF)
  {
    PRINTF("PCNT0 Underflow\r\n");
    BURTC_RETREG->s32_pcount -= (int32_t)(PCNT_TopGet(PCNT0)+1);
  }

  if(_pulsealarm_callback != NULL)
  {
    // Re Program default TOP
    PCNT_TopSet(PCNT0, PCNT0_TOP_VALUE);
    _pulsealarm_callback();
    _pulsealarm_callback = NULL;
  }

  PCNT_IntClear(PCNT0, PCNT_IF_OF | PCNT_IF_UF);
}

void lesense_set_pulsealarm(uint16_t pulse_offset, pulsealarm_callback_t pulsealarm_callback)
{
  // MCH - TODO : Make sure pulse will not occur during Topset
  PCNT_TopSet(PCNT0, PCNT_CounterGet(PCNT0) + (pulse_offset-1));
  _pulsealarm_callback = pulsealarm_callback;
}

void lesense_set_pcount_offset(int32_t pulse_offset)
{
  BURTC_RETREG->s32_pcount += pulse_offset;
}

void lesense_set_pcount(int32_t pcount_abs_value)
{
  BURTC_RETREG->s32_pcount = (pcount_abs_value - PCNT_CounterGet(PCNT0));
}

int32_t lesense_get_pcount(void)
{
  return (int32_t)(PCNT_CounterGet(PCNT0) + BURTC_RETREG->s32_pcount);
}

/** @} */
