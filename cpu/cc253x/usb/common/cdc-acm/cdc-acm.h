#ifndef CDC_ACM_H_UFV6K50827__
#define CDC_ACM_H_UFV6K50827__

#include "cdc.h"
#include "contiki.h"

void usb_cdc_acm_setup();


#define USB_CDC_ACM_LINE_CODING 0x1
#define USB_CDC_ACM_LINE_STATE  0x2
uint8_t usb_cdc_acm_get_events(void);


#define USB_CDC_ACM_DTE 0x1
#define USB_CDC_ACM_RTS 0x2
uint8_t usb_cdc_acm_get_line_state(void);


const struct usb_cdc_line_coding *usb_cdc_acm_get_line_coding(void);

void usb_cdc_acm_set_event_process(struct process *p);

#endif /* CDC_ACM_H_UFV6K50827__ */
