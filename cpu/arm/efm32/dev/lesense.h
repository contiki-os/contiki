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
 *         EFM32 LESENSE header file
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#ifndef __LESENSE_H__
#define __LESENSE_H__

#include "contiki.h"
#include "em_lesense.h"

#define WITH_DIRECTION
//#define CONTINUOUS_POWER


/** Configuration for pulse sense channels. */
#define LESENSE_PULSE1_SENSOR_CH_CONF                     \
{                                                  \
  true,                      /* Enable scan channel. */    \
  false,                      /* Enable the assigned pin on scan channel. */ \
  true,                      /* Enable interrupts on channel. */ \
  lesenseChPinExHigh,        /* GPIO pin is high during the excitation period. */    \
  lesenseChPinIdleDis,       /* GPIO pin is low during the idle period. */ \
  true,                      /* Use alternate excitation pins for excitation. */    \
  true,                     /* Disabled to shift results from this channel to the decoder register. */ \
  false,                     /* Disabled to invert the scan result bit. */  \
  true,                      /* Enabled to store counter value in the result buffer. */   \
  lesenseClkLF,              /* Use the LF clock for excitation timing. */    \
  lesenseClkLF,              /* Use the LF clock for sample timing. */ \
  2,                     /* Excitation time is set to 1(+1) excitation clock cycles. */    \
  2,                     /* Sample delay is set to 1(+1) sample clock cycles. */ \
  15,                     /* Measure delay is set to 16 excitation clock cycles.*/    \
  0x38,                     /* ACMP threshold has been set to 0x38. */ \
  lesenseSampleModeACMP,     /* ACMP will be used in comparison. */    \
  lesenseSetIntNone, /*lesenseSetIntPosEdge,  */      /* Interrupt is generated if the sensor triggers. */ \
  0x0000U,                   /* Counter threshold has been set to 0x00. */    \
  lesenseCompModeLess        /* Compare mode has been set to trigger interrupt on "less". */ \
}

/** Configuration for pulse sense channels. */
#define LESENSE_PULSE2_SENSOR_CH_CONF                     \
{                                                  \
  true,                      /* Enable scan channel. */    \
  false,                      /* Enable the assigned pin on scan channel. */ \
  true,                      /* Enable interrupts on channel. */ \
  lesenseChPinExHigh,        /* GPIO pin is high during the excitation period. */    \
  lesenseChPinIdleDis,       /* GPIO pin is low during the idle period. */ \
  true,                      /* Use alternate excitation pins for excitation. */    \
  true,                     /* Disabled to shift results from this channel to the decoder register. */ \
  false,                     /* Disabled to invert the scan result bit. */  \
  true,                      /* Enabled to store counter value in the result buffer. */   \
  lesenseClkLF,              /* Use the LF clock for excitation timing. */    \
  lesenseClkLF,              /* Use the LF clock for sample timing. */ \
  2,                     /* Excitation time is set to 1(+1) excitation clock cycles. */    \
  2,                     /* Sample delay is set to 1(+1) sample clock cycles. */ \
  2,                     /* Measure delay is set to 0 excitation clock cycles.*/    \
  0x38,                     /* ACMP threshold has been set to 0x38. */ \
  lesenseSampleModeACMP,     /* ACMP will be used in comparison. */    \
  lesenseSetIntNone, /*lesenseSetIntPosEdge,  */    /* Interrupt is generated if the sensor triggers. */ \
  0x0000U,                   /* Counter threshold has been set to 0x00. */    \
  lesenseCompModeLess        /* Compare mode has been set to trigger interrupt on "less". */ \
}

#ifdef WITH_DIRECTION
/** Configuration for pulse sense channels. */
#define LESENSE_DIRECTION_SENSOR_CH_CONF                     \
{                                                  \
  true,                      /* Enable scan channel. */    \
  false,                      /* Enable the assigned pin on scan channel. */ \
  true,                      /* Enable interrupts on channel. */ \
  lesenseChPinExHigh,        /* GPIO pin is high during the excitation period. */    \
  lesenseChPinIdleDis,       /* GPIO pin is low during the idle period. */ \
  true,                      /* Use alternate excitation pins for excitation. */    \
  true,                     /* Disabled to shift results from this channel to the decoder register. */ \
  false,                     /* Disabled to invert the scan result bit. */  \
  true,                      /* Enabled to store counter value in the result buffer. */   \
  lesenseClkLF,              /* Use the LF clock for excitation timing. */    \
  lesenseClkLF,              /* Use the LF clock for sample timing. */ \
  1,                     /* Excitation time is set to 1(+1) excitation clock cycles. */    \
  1,                     /* Sample delay is set to 1(+1) sample clock cycles. */ \
  1,                     /* Measure delay is set to 0 excitation clock cycles.*/    \
  0x38,                     /* ACMP threshold has been set to 0x38. */ \
  lesenseSampleModeACMP,     /* ACMP will be used in comparison. */    \
  lesenseSetIntNone,      /* Interrupt is generated if the sensor triggers. */ \
  0x0000U,                   /* Counter threshold has been set to 0x00. */    \
  lesenseCompModeLess        /* Compare mode has been set to trigger interrupt on "less". */ \
}
#else
#define LESENSE_DIRECTION_SENSOR_CH_CONF LESENSE_DISABLED_CH_CONF
#endif

/** Configuration for disabled channels. */
#define LESENSE_DISABLED_CH_CONF                     \
{                                                  \
  false,                     /* Disable scan channel. */    \
  false,                     /* Disable the assigned pin on scan channel. */ \
  false,                     /* Disable interrupts on channel. */ \
  lesenseChPinExDis,         /* GPIO pin is disabled during the excitation period. */    \
  lesenseChPinIdleDis,       /* GPIO pin is disabled during the idle period. */ \
  false,                     /* Don't use alternate excitation pins for excitation. */    \
  false,                     /* Disabled to shift results from this channel to the decoder register. */ \
  false,                     /* Disabled to invert the scan result bit. */  \
  false,                     /* Disabled to store counter value in the result buffer. */   \
  lesenseClkLF,              /* Use the LF clock for excitation timing. */    \
  lesenseClkLF,              /* Use the LF clock for sample timing. */ \
  0x00U,                     /* Excitation time is set to 5(+1) excitation clock cycles. */    \
  0x00U,                     /* Sample delay is set to 7(+1) sample clock cycles. */ \
  0x00U,                     /* Measure delay is set to 0 excitation clock cycles.*/    \
  0x00U,                     /* ACMP threshold has been set to 0. */ \
  lesenseSampleModeCounter,  /* ACMP output will be used in comparison. */    \
  lesenseSetIntNone,         /* No interrupt is generated by the channel. */ \
  0x00U,                     /* Counter threshold has been set to 0x01. */    \
  lesenseCompModeLess        /* Compare mode has been set to trigger interrupt on "less". */ \
}

#define LESENSE_SCAN_CONF                                         \
{                                                                            \
  {                                                                          \
    LESENSE_DISABLED_CH_CONF,          /* Channel 0. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 1. */                      \
	LESENSE_DISABLED_CH_CONF,          /* GAS_WATER_PULSE_1 (state > 7) */   \
	LESENSE_DISABLED_CH_CONF,          /* GAS_WATER_PULSE_2 */               \
    LESENSE_DISABLED_CH_CONF,          /* GAS_WATER_WATCH  */                \
    LESENSE_DISABLED_CH_CONF,          /* WATER_DIRECTION */                 \
    LESENSE_DISABLED_CH_CONF,          /* Channel 6. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 7. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 8. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 9. */                      \
    LESENSE_PULSE1_SENSOR_CH_CONF,      /* GAS_WATER_PULSE_1 */               \
	LESENSE_PULSE2_SENSOR_CH_CONF,      /* GAS_WATER_PULSE_2 */               \
	LESENSE_DISABLED_CH_CONF,          /* GAS_WATER_WATCH  */                \
	LESENSE_DIRECTION_SENSOR_CH_CONF,          /* WATER_DIRECTION */          \
    LESENSE_DISABLED_CH_CONF,          /* Channel 14. */                     \
    LESENSE_DISABLED_CH_CONF           /* Channel 15. */                     \
  }                                                                          \
}

#ifdef CONTINUOUS_POWER
/** Default configuration for alternate excitation channel. */
#define LESENSE_DDC_ALTEX_CH_CONF                                       \
{                                                                              \
  true,                   /* Alternate excitation enabled.*/                  \
  lesenseAltExPinIdleHigh, /* Alternate excitation pin is low in idle. */ \
  true                    /* Excite for all channels. */        \
}
#else
/** Default configuration for alternate excitation channel. */
#define LESENSE_DDC_ALTEX_CH_CONF                                       \
{                                                                              \
  true,                   /* Alternate excitation enabled.*/                  \
  lesenseAltExPinIdleLow, /* Alternate excitation pin is low in idle. */ \
  true                    /* Excite for all channels. */        \
}
#endif
/** Disabled configuration for alternate excitation channel. */
#define LESENSE_ALTEX_DIS_CH_CONF                                   \
{                                                                              \
  false,                   /* Alternate excitation enabled.*/                  \
  lesenseAltExPinIdleDis,  /* Alternate excitation pin is disabled in idle. */ \
  false                    /* Excite only for corresponding channel. */        \
}

/** Default configuration for all alternate excitation channels. */
#define LESENSE_ALTEX_CONF                                          \
{                                                                              \
  lesenseAltExMapALTEX,                                                         \
  {                                                                            \
    LESENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 0. */\
    LESENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 1. */\
    LESENSE_DDC_ALTEX_CH_CONF, /* LES_ALTEX2 : GAS_WATER_POWER_EN_1 */\
    LESENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 3. */\
    LESENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 4. */\
    LESENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 5. */\
    LESENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 6. */\
    LESENSE_ALTEX_DIS_CH_CONF  /* Alternate excitation channel 7. */\
  }                                                                            \
}

/** Default configuration for decoder state condition. */
#ifdef WITH_DIRECTION
/* State 0 : Wait '1' */
#define LESENSE_ST_CONF_STATE0_UP                                         \
{                                                                      \
  0b0011,               /* Pulse up is detected  */                \
  0b1000,               /*  */                \
  1,                  /* Next state is state 8 (wait '1') */                    \
  lesenseTransActUp,   /* Count up */        \
  false                /* No interrupt triggered on compare match. */  \
}
#define LESENSE_ST_CONF_STATE0_DOWN                                         \
{                                                                      \
  0b0111,               /* Pulse down is detected  */                \
  0b1000,               /*  */                \
  1,                  /* Next state is state 8 (wait '1') */                    \
  lesenseTransActDown,   /* Count down */        \
  false                /* No interrupt triggered on compare match. */  \
}

/* State 1 : Wait '0' */
#define LESENSE_ST_CONF_STATE1                                         \
{                                                                      \
  0b000,               /* all 0  */                \
  0b100,               /*  */                \
  0,                  /* Next state is state 0 (Wait '0') */                    \
  lesenseTransActNone, /* No PRS action performed on compare match. */ \
  false                /* No interrupt triggered on compare match. */  \
}
#else
/* State 0 : Wait '1' */
#define LESENSE_ST_CONF_STATE0_UP                                         \
{                                                                      \
  0b11,               /* Pulse up is detected  */                \
  0b00,               /*  */                \
  1,                  /* Next state is state 8 (wait '1') */                    \
  lesenseTransActUp,   /* Count up */        \
  false                /* No interrupt triggered on compare match. */  \
}

/* State 1 : Wait '0' */
#define LESENSE_ST_CONF_STATE1                                         \
{                                                                      \
  0b00,               /* all 0  */                \
  0b00,               /*  */                \
  0,                  /* Next state is state 0 (Wait '0') */                    \
  lesenseTransActNone, /* No PRS action performed on compare match. */ \
  false                /* No interrupt triggered on compare match. */  \
}

#endif

#if 0
/* State 8: check '1' */
#define LESENSE_ST_CONF_STATE8_A                                         \
{                                                                      \
  0b01,               /* Pulse up is confirmed  */                \
  0b10,               /*  */                                        \
  1,                    /* Next state is state 1 (wait '0') */                    \
  lesenseTransActUp,   /* Count up */ \
  false                /* No interrupt triggered on compare match. */  \
}
#define LESENSE_ST_CONF_STATE8_B                                         \
{                                                                      \
  0b00,               /* Pulse up was a mistake  */                \
  0b10,               /*  */                                        \
  0,                    /* Next state is state 0 (wait '1') */                    \
  lesenseTransActNone,  /* No PRS action performed on compare match. */ \
  false                /* No interrupt triggered on compare match. */  \
}

/* State 10 : check '0' */
#define LESENSE_ST_CONF_STATE10_A                                         \
{                                                                      \
  0b00,               /* Pulse down is confirmed  */                \
  0b10,               /*  */                                        \
  0,                    /* Next state is state 0 (wait '1') */                    \
  lesenseTransActNone,   /* Count up */ \
  false                /* No interrupt triggered on compare match. */  \
}
#define LESENSE_ST_CONF_STATE10_B                                         \
{                                                                      \
  0b01,               /* Pulse up was a mistake  */                \
  0b10,               /*  */                                        \
  1,                    /* Come back to state 1 (wait '0') */                    \
  lesenseTransActNone,  /* No PRS action performed on compare match. */ \
  false                /* No interrupt triggered on compare match. */  \
}
#endif


#ifdef WITH_DIRECTION
#define LESENSE_STATES_CONF \
{  /* chain |   Descriptor A         |   Descriptor B   */ \
  {                                                                                                                \
    { false, LESENSE_ST_CONF_STATE0_UP, LESENSE_ST_CONF_STATE0_DOWN }, /* Decoder state 0. */                      \
    { false, LESENSE_ST_CONF_STATE1, LESENSE_ST_CONF_STATE1 }, /* Decoder state 1. */                              \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 2. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 3. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 4. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 5. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 6. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 7. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 8. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 9. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 10. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 11. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 12. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 13. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 14. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT } /* Decoder state 15. */                            \
  }                                                                                                                \
}

#else
#define LESENSE_STATES_CONF \
{  /* chain |   Descriptor A         |   Descriptor B   */ \
  {                                                                                                                \
    { false, LESENSE_ST_CONF_STATE0_UP, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 0. */                      \
    { false, LESENSE_ST_CONF_STATE1, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 1. */                              \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 2. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 3. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 4. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 5. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 6. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 7. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 8. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 9. */                            \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 10. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 11. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 12. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 13. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT }, /* Decoder state 14. */                           \
    { false, LESENSE_ST_CONF_DEFAULT, LESENSE_ST_CONF_DEFAULT } /* Decoder state 15. */                            \
  }                                                                                                                \
}
#endif

typedef void (*pulsealarm_callback_t)(void);


void lesense_init(void);
void lesense_init_pcnt_only(void);

int32_t lesense_get_pcount(void);

void lesense_set_pcount(int32_t pcount_abs_value);
void lesense_set_pcount_offset(int32_t pulse_offset);
void lesense_set_pulsealarm(uint16_t pulse_offset, pulsealarm_callback_t pulsealarm_callback);

/** @} */

#endif /* __LESENSE_H__ */
