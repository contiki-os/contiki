/** @file mems.h
 * @brief Header for MB851 mems APIS 
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef _MEMS_H_
#define _MEMS_H_

#include "hal/micro/mems-regs.h"

/** @brief Mems data type: three acceleration values each related to a specific direction
   Watch out: only lower data values (e.g. those terminated by the _l) are 
   currently used by the device */

typedef struct {
  uint8_t outx_l;
  uint8_t outx_h;
  uint8_t outy_l;
  uint8_t outy_h;
  uint8_t outz_l;
  uint8_t outz_h;
} t_mems_data;


/** @brief Mems Initialization function
 */
uint8_t mems_Init(void);

/** @brief Get mems acceleration values
 */
uint8_t mems_GetValue(t_mems_data *mems_data);

#endif /* _MEMS_H_ */

