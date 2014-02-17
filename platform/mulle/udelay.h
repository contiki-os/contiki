/**
 * \file
 *         Provide udelay routine for MK60DZ10.
 * \author
 *         Tony Persson <tony.persson@rubico.com>
 */

#ifndef __UDELAY_H__
#define __UDELAY_H__

void udelay_init(void);
void udelay(uint16_t us);

#endif
