#include "common.h"
/*
 * baycom_ser_fdx.c
 *
 * Created: 4/13/2013 5:59:48 PM
 *  Author: Ioannis Glaropoulos
 */ 

unsigned int hweight8(unsigned int w)
{
	unsigned short res = (w & 0x55) + ((w >> 1) & 0x55);
    res = (res & 0x33) + ((res >> 2) & 0x33);
    return (res & 0x0F) + ((res >> 4) & 0x0F);
}