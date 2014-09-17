/**
 * \file
 *         Provide udelay routine for MK60DZ10.
 * \author
 *         Tony Persson <tony.persson@rubico.com>
 */

#include "K60.h"
#include "config-clocks.h"

void
udelay_init(void)
{
  SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
  PIT->MCR = 0x00;
}
void
udelay(uint16_t us)
{
  PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV((F_BUS / 1000000) * (uint32_t)us - 1);
  PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK;
  PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TEN_MASK;
  while(!(PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK));
  PIT->CHANNEL[0].TCTRL = 0x00;
}
