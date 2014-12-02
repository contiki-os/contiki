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
  /* Enable peripheral clock */
  SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
  /* Reset logic to a known state */
  PIT->MCR = 0x00;
}

/**
 * Microsecond busy wait.
 *
 * \param [in] us number of microseconds to sleep.
 */
void
udelay(uint16_t us)
{
  /* Don't hang on zero µs sleep. */
  if (us == 0) return;

  /* Set up timer */
  PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV((F_BUS / 1000000) * (uint32_t)us);

  /* Disable timer to load a new value */
  BITBAND_REG(PIT->CHANNEL[0].TCTRL, PIT_TCTRL_TEN_SHIFT) = 0;

  /* Clear interrupt flag */
  BITBAND_REG(PIT->CHANNEL[0].TFLG, PIT_TFLG_TIF_SHIFT) = 1;

  /* Enable timer, no interrupt, no chaining */
  PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TEN_MASK;

  /* Wait for completion */
  while(!(PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK));

  /* Disable everything */
  PIT->CHANNEL[0].TCTRL = 0x00;
}
