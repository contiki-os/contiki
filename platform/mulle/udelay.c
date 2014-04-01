/**
* \file
*         Provide udelay routine for MK60DZ10.
* \author
*         Tony Persson <tony.persson@rubico.com>
*/

#include "MK60D10.h"
#include "uart.h"

void udelay_init(void)
{
	SIM_SCGC6     |= SIM_SCGC6_PIT_MASK;
	PIT_MCR        = 0x00;
}

void udelay(uint16_t us)
{
	PIT_LDVAL0  = 23 * (uint32_t)us - 1;
	PIT_TFLG0  |= 0x01;
	PIT_TCTRL0  = 0x01;
	while (!PIT_TFLG0);
	PIT_TCTRL0  = 0x00;
}
