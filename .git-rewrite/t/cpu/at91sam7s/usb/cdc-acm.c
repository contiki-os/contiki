#include <cdc-acm.h>
#include <cdc.h>
#include <usb-proto.h>
#include <stdio.h>

static unsigned int
handle_cdc_acm_requests()
{
  printf("CDC request %02x %02x\n", usb_setup_buffer.bmRequestType, usb_setup_buffer.bRequest);
  switch(usb_setup_buffer.bmRequestType) {
  case 0x21: /* CDC interface OUT requests */
    /* Check if it's the right interface */
    if (usb_setup_buffer.wIndex != 0) return 0;
    switch(usb_setup_buffer.bRequest) {
    case SET_CONTROL_LINE_STATE:
      if (usb_setup_buffer.wValue & 0x02) {
	puts("Carrier on");
      } else {
	puts("Carrier off");
      }
      if (usb_setup_buffer.wValue & 0x01) {
	puts("DTE on");
      } else {
	puts("DTE off");
      }
      usb_send_ctrl_status();
      return 1;

    case SEND_ENCAPSULATED_COMMAND:
      printf("Got CDC command: length %d\n", usb_ctrl_data_len);
      usb_send_ctrl_status();
      return 1;
      
   
    case SET_LINE_CODING:
      if (usb_ctrl_data_len == 7) {
	static const char parity_char[] = {'N', 'O', 'E', 'M', 'S'};
	static const char *stop_bits_str[] = {"1","1.5","2"};
	const struct usb_cdc_line_coding *coding =
	  (const struct usb_cdc_line_coding *)usb_ctrl_data_buffer;
	char parity = ((coding->bParityType > 4)
		       ? '?' : parity_char[coding->bParityType]);
	const char *stop_bits = ((coding->bCharFormat > 2)
				 ? "?" : stop_bits_str[coding->bCharFormat]);
	printf("Got CDC line coding: %ld/%d/%c/%s\n",
	       coding->dwDTERate, coding->bDataBits, parity, stop_bits);
	usb_send_ctrl_status();
      } else {
	return 0;
      }
      return 1;
    }
    break;
  case 0xa1: /* CDC interface IN requests */
    if (usb_setup_buffer.wIndex != 0) return 0;
    switch(usb_setup_buffer.bRequest) {
    case GET_ENCAPSULATED_RESPONSE:
      printf("CDC response");
      usb_send_ctrl_status();
      return 1;
    }
  }
  return 0;
}

static const struct USBRequestHandler cdc_acm_request_handler =
  {
    0x21, 0x7f,
    0x00, 0x00,
    handle_cdc_acm_requests
  };

static struct USBRequestHandlerHook cdc_acm_request_hook =
  {
    NULL,
    &cdc_acm_request_handler
  };

void
usb_cdc_acm_setup()
{
  usb_register_request_handler(&cdc_acm_request_hook);
}
