#include <AT91SAM7S64.h>
#include <interrupt-utils.h>
#include <usb-interrupt.h>
#include <usb-proto.h>
#include <usb-api.h>
#include <stdio.h>
#include <sys/process.h>
#include <stdio.h>
#include <descriptors.h>
#include <string-descriptors.h>

#define USB_PULLUP_PIN AT91C_PIO_PA16

static unsigned short usb_device_status;
static unsigned char usb_configuration_value;

static struct process * user_process = NULL;

static struct USBRequestHandlerHook *usb_request_handler_hooks = NULL;

static const unsigned char zero_byte = 0;
static const unsigned short zero_word = 0;

static void
notify_user(struct usb_user_msg* msg)
{
  if (user_process) {
    process_post(user_process, PROCESS_EVENT_MSG, msg);
  }
}

void
usb_set_user_process(struct process *p)
{
  user_process = p;
}

static void
get_device_descriptor()
{
  usb_send_ctrl_response((unsigned char*)&device_descriptor, sizeof(device_descriptor));
}

static void
get_string_descriptor()
{
  if (LOW_BYTE(usb_setup_buffer.wValue) == 0) {
    usb_send_ctrl_response((const unsigned char*)string_languages->lang_descr,
			   string_languages->lang_descr->bLength);
  } else {
    unsigned char l;
    const struct usb_st_string_descriptor *descriptor;
    const struct usb_st_string_descriptor * const *table;
    const struct usb_st_string_language_map *map;
    if (LOW_BYTE(usb_setup_buffer.wValue) > string_languages->max_index) {
      usb_error_stall();
      return;
    }
    l = string_languages->num_lang;
    map = string_languages->map;
    table = map->descriptors; /* Use first table if language not found */
    while (l > 0) {
      if (map->lang_id == usb_setup_buffer.wIndex) {
	table = map->descriptors;
	break;
      }
      map++;
      l--;
    }
    printf("Lang id %04x = table %p\n", usb_setup_buffer.wIndex, (void*)table);
    descriptor = table[LOW_BYTE(usb_setup_buffer.wValue) - 1];
    usb_send_ctrl_response((const unsigned char*)descriptor,
			   descriptor->bLength);
  }
}

static void
get_configuration_descriptor()
{
  usb_send_ctrl_response((unsigned char*)configuration_head,
			 configuration_head->wTotalLength);
}

static void
get_configuration()
{
  usb_send_ctrl_response((unsigned char*)&usb_configuration_value,
			 sizeof(usb_configuration_value));
}

/* Returns true if the configuration value changed */
static int
set_configuration()
{
  if (usb_configuration_value != LOW_BYTE(usb_setup_buffer.wValue)) {
    usb_configuration_value = LOW_BYTE(usb_setup_buffer.wValue);
    if (usb_configuration_value > 0) {
      *AT91C_UDP_GLBSTATE |= AT91C_UDP_CONFG;
    } else {
      *AT91C_UDP_GLBSTATE &= ~AT91C_UDP_CONFG;
    }
    usb_send_ctrl_status();
    return 1;
  } else {
    usb_send_ctrl_status();
    return 0;
  }
}

static void
get_device_status()
{
  puts("get_device_status");
  usb_send_ctrl_response((const unsigned char*)&usb_device_status,
		     sizeof(usb_device_status));
}

static void
get_endpoint_status()
{
  puts("get_endpoint_status");
  if ((usb_setup_buffer.wIndex & 0x7f) == 0) {
    usb_send_ctrl_response((const unsigned char*)&zero_word,
		       sizeof(zero_word));
  } else {
    volatile USBEndpoint *ec;
    ec = usb_find_endpoint(usb_setup_buffer.wIndex);
    if (ec) {
      usb_send_ctrl_response((const unsigned char*)&ec->status, sizeof(ec->status));
    } else {
      usb_error_stall();
    }
  }
}

static void
get_interface_status()
{
  puts("get_interface_status");
  usb_send_ctrl_response((const unsigned char*)&zero_word,
			 sizeof(zero_word));
}

static void
get_interface()
{
  puts("get_interface");
  if (usb_configuration_value == 0) usb_error_stall();
  else {
    usb_send_ctrl_response(&zero_byte,
		       sizeof(zero_byte));
  }
}
  
static struct usb_user_msg config_msg = {USB_USER_MSG_TYPE_CONFIG};
static struct usb_user_msg io_msg[3];

static unsigned int
handle_standard_requests()
{
  switch(usb_setup_buffer.bmRequestType) {
  case 0x80: /* standard device IN requests */
    switch(usb_setup_buffer.bRequest) {
    case GET_DESCRIPTOR:
      switch (HIGH_BYTE(usb_setup_buffer.wValue)) {
      case DEVICE:
	get_device_descriptor();
	break;
      case CONFIGURATION:
	get_configuration_descriptor();
	break;
      case STRING:
	get_string_descriptor();
	break;
      default:
	/* Unknown descriptor */
	return 0;
      }
      break;
    case GET_CONFIGURATION:
      get_configuration();
      break;
    case GET_STATUS:
      get_device_status();
      break;
    case GET_INTERFACE:
      get_interface();
      break;
    default:
      return 0;
    }
    break;
  case 0x81: /* standard interface IN requests */
    switch(usb_setup_buffer.bRequest) {
    case GET_STATUS:
      get_interface_status();
      break;
#ifdef HID_ENABLED
      case GET_DESCRIPTOR:
	switch (USB_setup_buffer.wValue.byte.high) {
	case REPORT:
	  get_report_descriptor();
	  break;
	}
	break;
#endif	
    default:
      return 0;
    }
    break;
  case 0x82: /* standard endpoint IN requests */
    switch(usb_setup_buffer.bRequest) {
    case GET_STATUS:
      get_endpoint_status();
      break;
    default:
      return 0;
    }
    break;
  case 0x00: /* standard device OUT requests */
    switch(usb_setup_buffer.bRequest) {
    case SET_ADDRESS:
      printf("Address: %d\n", LOW_BYTE(usb_setup_buffer.wValue));
      usb_set_address();
      usb_send_ctrl_status();
      break;
#if SETABLE_STRING_DESCRIPTORS > 0
    case SET_DESCRIPTOR:
      if (usb_setup_buffer.wValue.byte.high == STRING) {
	set_string_descriptor();
      } else {
	return 0;
      }
      break;
#endif
    case SET_CONFIGURATION:
      if (set_configuration()) {
	config_msg.data.config = LOW_BYTE(usb_setup_buffer.wValue);
	notify_user(&config_msg);
      }
      break;
    default:
      return 0;
    }
    break;
  case 0x02:
    switch(usb_setup_buffer.bRequest) {
    case SET_FEATURE:
    case CLEAR_FEATURE:
      if (usb_setup_buffer.wValue == ENDPOINT_HALT_FEATURE) {
	volatile USBEndpoint *ep = usb_find_endpoint(usb_setup_buffer.wIndex);
	if (ep) {
	  usb_halt_endpoint(ep->addr, usb_setup_buffer.bRequest== SET_FEATURE);
	  usb_send_ctrl_status();
	} else {
	  usb_error_stall();
	}
      } else {
	usb_error_stall();
      }
      break;
    default:
      return 0;
    }
    break;
#ifdef HID_ENABLED
  case 0xa1: /* class specific interface IN request*/
    switch(USB_setup_buffer.bRequest) {
    case GET_HID_REPORT:
      puts("Get report\n");
      send_ctrl_response((code u_int8_t*)&zero_byte,
			   sizeof(zero_byte));
      break;
    case GET_HID_IDLE:
      puts("Get idle\n");
      send_ctrl_response((code u_int8_t*)&zero_byte,
			 sizeof(zero_byte));
      break;
    default:
      return 0;
      }
    break;
  case 0x21: /* class specific interface OUT request*/
    switch(USB_setup_buffer.bRequest) {
    case SET_HID_IDLE:
      puts("Set idle\n");
      send_ctrl_status();
      break;
    default:
      return 0;
    }
    break;
#endif
  default:
    return 0;
  }
  return 1;
}

static const struct USBRequestHandler standard_request_handler =
  {
    0x00, 0x60,
    0x00, 0x00,
    handle_standard_requests
  };

static struct USBRequestHandlerHook standard_request_hook =
  {
    NULL,
    &standard_request_handler
  };

PROCESS(usb_process, "USB process");

PROCESS_THREAD(usb_process, ev , data)
{
  PROCESS_BEGIN();
  puts("USB process started");
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_EXIT) break;
    if (ev == PROCESS_EVENT_POLL) {
      if (usb_events & USB_EVENT_RESET) {
	printf("Reset\n");
	usb_clear_events(USB_EVENT_RESET);
      }
      if (usb_events & USB_EVENT_EP(0)) {
	/* puts("Endpoint 0"); */
	if (usb_endpoint_events[0] & USB_EP_EVENT_SETUP) {
	  struct USBRequestHandlerHook *hook = usb_request_handler_hooks;
#if 0
	  puts("Setup");
	  {
	    unsigned int i;
	    for (i = 0; i< 8; i++) printf(" %02x", ((unsigned char*)&usb_setup_buffer)[i]);
	    putchar('\n');
	  }
#endif
	  while(hook) {
	    const struct USBRequestHandler *handler = hook->handler;
	    /* Check if the handler matches the request */
	    if (((handler->request_type ^ usb_setup_buffer.bmRequestType)
		 & handler->request_type_mask) == 0
		&& ((handler->request ^ usb_setup_buffer.bRequest)
		    & handler->request_mask) == 0) {
	      if (handler->handler_func()) break;
	    }
	    hook = hook->next;
	  }
	  if (!hook) {
	    /* No handler found */
	    usb_error_stall();
	  }
	  usb_clear_ep_events(0, USB_EP_EVENT_SETUP);
	}
	usb_clear_events(USB_EVENT_EP(0));
      }
      
      {
	unsigned int e;
	for (e = 1; e <= 3; e++) {
	  if (usb_events & USB_EVENT_EP(e)) {
	    if (usb_endpoint_events[e] & (USB_EP_EVENT_OUT|USB_EP_EVENT_IN)) {
	      volatile USBEndpoint *ep = usb_find_endpoint(e);
	      struct usb_user_msg *msg = &io_msg[e-1];
	      if (usb_endpoint_events[e] & USB_EP_EVENT_OUT) {
		msg->type = USB_USER_MSG_TYPE_EP_OUT(e);
		msg->data.length = ep->buf_len;
	      } else {
		msg->type = USB_USER_MSG_TYPE_EP_IN(e);
		msg->data.length = ep->buf_size_mask + 1 - ep->buf_len;
	      }
	      notify_user(msg);
	      usb_clear_ep_events(e, USB_EP_EVENT_OUT|USB_EP_EVENT_IN);
	      usb_clear_events(USB_EVENT_EP(ep->addr));
	    }
	  }
	}
      }
    }
  }
  PROCESS_END();
}


void
usb_setup(void)
{
  /* Assume 96MHz PLL frequency */
  *AT91C_CKGR_PLLR = ((*AT91C_CKGR_PLLR & ~AT91C_CKGR_USBDIV)
			  | AT91C_CKGR_USBDIV_1);
  /* Enable 48MHz USB clock */
  *AT91C_PMC_SCER = AT91C_PMC_UDP;
  /* Enable USB main clock */
  *AT91C_PMC_PCER = (1 << AT91C_ID_UDP);
  
  /* Enable pullup */
  *AT91C_PIOA_PER = USB_PULLUP_PIN;
  *AT91C_PIOA_OER = USB_PULLUP_PIN;
  *AT91C_PIOA_CODR = USB_PULLUP_PIN;

  usb_register_request_handler(&standard_request_hook);
  process_start(&usb_process, NULL);
  usb_handler_process = &usb_process;
  
  /* Enable usb_interrupt */
  AT91C_AIC_SMR[AT91C_ID_UDP] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 4;
  AT91C_AIC_SVR[AT91C_ID_UDP] = (unsigned long) usb_int;
  *AT91C_AIC_IECR = (1 << AT91C_ID_UDP);
  usb_init_endpoints();
}

void
usb_register_request_handler(struct USBRequestHandlerHook *hook)
{
  struct USBRequestHandlerHook **prevp = &usb_request_handler_hooks;
  /* Find last hook */
  while(*prevp) {
    prevp = &(*prevp)->next;
  }
  /* Add last */
  *prevp = hook;
  hook->next = NULL;
}
