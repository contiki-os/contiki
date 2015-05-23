#ifndef USB_CORE_H_YIKJDA7S1X__
#define USB_CORE_H_YIKJDA7S1X__

#include <stdint.h>

extern struct USB_request_st usb_setup_buffer;

void usb_send_ctrl_response(const uint8_t *data, unsigned int len);

void usb_error_stall();

void usb_send_ctrl_status();

typedef void (*usb_ctrl_data_callback)(uint8_t *data, unsigned int length);

void usb_get_ctrl_data(uint8_t *data, unsigned int length,
                       usb_ctrl_data_callback cb);

#endif /* USB_CORE_H_YIKJDA7S1X__ */
