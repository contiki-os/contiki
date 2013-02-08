#ifndef __UART1_H__
#define __UART1_H__

uart1_init(unsigned long ubr);

uart1_set_input(int (*input)(unsigned char c));

#endif