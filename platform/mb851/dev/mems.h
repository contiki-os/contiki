/**
 * \addtogroup mb851-platform
 *
 * @{
 */
/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : mems.h
* Author             : MCD Application Team
* Version            : V1.0
* Date               : January 2010
* Description        : header file for mems driver
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef _MEMS_H_
#define _MEMS_H_
#include "mems-regs.h"
#include <stdint.h>

/* Sensitivity values in mg/digit */
#define HIGH_RANGE_SENSITIVITY      72
#define LOW_RANGE_SENSITIVITY       18

#define MEMS_LOW_RANGE              0
#define MEMS_HIGH_RANGE             1

/*
 * Mems data type: three acceleration values each related to a specific
 * direction.
 * Watch out: only lower data values (e.g. those terminated by the _l) are 
 * currently used by the device.
 */
typedef struct mems_data_s {
  int8_t outx_l;
  int8_t outx_h;
  int8_t outy_l;
  int8_t outy_h;
  int8_t outz_l;
  int8_t outz_h;
} mems_data_t;

uint8_t mems_init(void);

uint8_t mems_on(void);

uint8_t mems_off(void);

uint8_t mems_set_fullscale(boolean range);

boolean mems_get_fullscale(void);

uint8_t mems_get_value(mems_data_t * mems_data);

#endif /* _MEMS_H_ */
/** @} */
