#ifndef LOWLEVEL_H
#define LOWLEVEL_H

#define trim_xtal() pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);
void default_vreg_init(void);
void uart1_init(uint16_t inc, uint16_t mod, uint8_t samp);

#endif
