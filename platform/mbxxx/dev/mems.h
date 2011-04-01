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

/* Define --------------------------------------------------------------------*/
#include "mems_regs.h"

/* Define --------------------------------------------------------------------*/
// Sensitivity values in mg/digit
#define HIGH_RANGE_SENSITIVITY  72
#define LOW_RANGE_SENSITIVITY  18

#define MEMS_LOW_RANGE   0
#define MEMS_HIGH_RANGE  1

/* Typedef -------------------------------------------------------------------*/
/* Mems data type: three acceleration values each related to a specific direction
   Watch out: only lower data values (e.g. those terminated by the _l) are 
   currently used by the device */

typedef struct {
  int8s outx_l;
  int8s outx_h;
  int8s outy_l;
  int8s outy_h;
  int8s outz_l;
  int8s outz_h;
} t_mems_data;

/* Functions -----------------------------------------------------------------*/

/* Mems Initialization function */
int8u Mems_Init(void);

int8u MEMS_On(void);
int8u MEMS_Off(void);
int8u MEMS_SetFullScale(boolean range);
boolean MEMS_GetFullScale(void);

/* Get mems acceleration values */
int8u Mems_GetValue(t_mems_data *mems_data);

#endif /* _MEMS_H_ */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
