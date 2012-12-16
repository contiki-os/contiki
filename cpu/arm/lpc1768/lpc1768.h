/*
 * lpc1768.h
 *
 *  Created on: Dec 16, 2012
 *      Author: cazulu
 */

#ifndef LPC1768_H_
#define LPC1768_H_

//WARNING: Non-aligned structure copy causes a lot
//of problems, that's why we need this define to get
//around the issue. For an explanation, see:
//http://devl.org/pipermail/mc1322x/2009-November/000105.html
#define uip_ipaddr_copy(dest, src)              \
    memcpy(dest, src, sizeof(*dest))


#endif /* LPC1768_H_ */
