/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "debug-uart.h"
#include "debug_frmwrk.h"

#define CR     0x0D

void
debug_uart_setup(void)
{ /* Initialize Serial Interface       */
  debug_frmwrk_init();
}
