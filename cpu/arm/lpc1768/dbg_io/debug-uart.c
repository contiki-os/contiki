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
setup_debug_uart(void)
{ /* Initialize Serial Interface       */
  debug_frmwrk_init();
}

/* Implementation of putchar (also used by printf function to output data)    */
void
dbg_putchar(char ch)
{ /* Write character to Serial Port    */
#if 1
  if (ch == '\n')
    {
      /* output CR */
      _DBC(CR);
    }
#endif
  _DBC(ch);
}

unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len)
{
  unsigned int bytesSent = 0;
  unsigned char * str = seq;
  while(len--){
      dbg_putchar(*str++);
      bytesSent++;
  }

  return bytesSent;
}
