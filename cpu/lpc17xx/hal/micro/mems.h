/** @file mems.h
 * @brief Header for MB851 mems APIS 
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef _MEMS_H_
#define _MEMS_H_

#include "hal/micro/mems_regs.h"

/** @brief Mems data type: three acceleration values each related to a specific direction
   Watch out: only lower data values (e.g. those terminated by the _l) are 
   currently used by the device */

typedef struct {
  int8u outx_l;
  int8u outx_h;
  int8u outy_l;
  int8u outy_h;
  int8u outz_l;
  int8u outz_h;
} t_mems_data;


/** @brief Mems Initialization function
 */
int8u mems_Init(void);

/** @brief Get mems acceleration values
 */
int8u mems_GetValue(t_mems_data *mems_data);

#endif /* _MEMS_H_ */

