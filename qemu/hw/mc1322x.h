/* Freescale mc1322x support
 *
 * Copyright (c) 2009 Mariano Alvira
 * Written by Mariano Alvira <mar@devl.org>
 *
 * This code is licenced under the GPL.
 */

#ifndef PXA_H
#define PXA_H			"pxa.h"

#define MC1322X_ROMBASE 0x00000000
#define MC1322X_ROMSIZE 0x00014000
#define MC1322X_RAMBASE 0x00400000
#define MC1322X_RAMSIZE 0x00020000

/* mc1322x.c */
struct mc1322x_state_s {
    CPUState *env;
    qemu_irq *pic;
    qemu_irq reset;
    struct mc1322x_gpio_info_s *gpio;
    struct mc1322x_keypad_s *kp;

    /* Power management */
    target_phys_addr_t pm_base;
    uint32_t pm_regs[0x40];

    /* Clock management */
    target_phys_addr_t cm_base;
    uint32_t cm_regs[4];
    uint32_t clkcfg;

};


#endif
