/*
  Copied from mc1322x/dev/cpu. 
  
  This file exists as a work-around for the hardware dependant calls
  to slip_arch_init.

  Current the prototype for slip_arch_init is slip_arch_init(urb)

  and a typical call is something like
  slip_arch_init(BAUD2URB(115200))

  BAUD2UBR is hardware specific, however. Furthermore, for the sky
  platform it's typically defined with #include "dev/uart1.h" (see
  rpl-boarder-router/slip-bridge.c), a sky specific file. dev/uart1.h
  includes msp430.h which includes the sky contiki-conf.h which
  defines BAUD2UBR.

  To me, the correct think to pass is simply the baudrate and have the
  hardware specific conversion happen inside slip_arch_init. 

  Notably, most implementations just ignore the passed parameter
  anyway. (except AVR)
  
 */

#ifndef DEV_UART1_H
#define DEV_UART1_H

#define BAUD2UBR(x) x

#endif
