
#ifndef CDC_EEM_H_
#define CDC_EEM_H_

#include "contiki.h"
#include <stdint.h>

extern void cdc_eem_process(void);
extern uint8_t eem_send(uint8_t * senddata, uint16_t sendlen, uint8_t led);
extern void cdc_eem_configure_endpoints();

#endif // CDC_EEM_H_
