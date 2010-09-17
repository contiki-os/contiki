
#ifndef __CDC_EEM_H__
#define __CDC_EEM_H__

#include "contiki.h"
#include <stdint.h>

extern void cdc_eem_process(void);
extern uint8_t eem_send(uint8_t * senddata, uint16_t sendlen, uint8_t led);
extern void cdc_eem_configure_endpoints();

#endif // __CDC_EEM_H__
