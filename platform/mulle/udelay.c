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
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
  PIT_MCR = 0x00;
}
void
udelay(uint16_t us)
{
  PIT_LDVAL0 = PIT_LDVAL_TSV((F_BUS / 1000000) * (uint32_t)us - 1);
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
  PIT_TCTRL0 = PIT_TCTRL_TEN_MASK;
  while(!(PIT_TFLG0 & PIT_TFLG_TIF_MASK));
  PIT_TCTRL0 = 0x00;
}
