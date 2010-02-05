#include <usb-core.h>
#include <usb.h>
#include <usb-arch.h>
#include <usb-api.h>
#include <stdio.h>
#include <sys/process.h>
#include <stdio.h>
#include <descriptors.h>
#include <string-descriptors.h>

#define DEBUG 
#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


struct USB_request_st usb_setup_buffer;
static USBBuffer ctrl_buffer;

#define SETUP_ID 1
#define OUT_ID 2
#define IN_ID 3
#define STATUS_OUT_ID 4
#define STATUS_IN_ID 5

static uint16_t usb_device_status;
static uint8_t usb_configuration_value;

static struct USBRequestHandlerHook *usb_request_handler_hooks = NULL;

static const unsigned char zero_byte = 0;
static const unsigned short zero_word = 0;

static unsigned char usb_flags = 0;
#define USB_FLAG_ADDRESS_PENDING 0x01

static struct process *global_user_event_pocess = NULL;
static unsigned int global_user_events = 0;

void
usb_set_global_event_process(struct process *p)
{
  global_user_event_pocess = p;
}
unsigned int
usb_get_global_events(void)
{
  unsigned int e = global_user_events;
  global_user_events = 0;
  return e;
}

static void
notify_user(unsigned int e)
{
  global_user_events |= e;
  if (global_user_event_pocess) {
    process_poll(global_user_event_pocess);
  }
}

void
usb_send_ctrl_response(const uint8_t *data, unsigned int len)
{
  if (ctrl_buffer.flags & USB_BUFFER_SUBMITTED) return;
  if (len >= usb_setup_buffer.wLength) {
    len = usb_setup_buffer.wLength;	/* Truncate if too long */
  }
  ctrl_buffer.flags = USB_BUFFER_NOTIFY | USB_BUFFER_IN;
  if (len < usb_setup_buffer.wLength) {
    ctrl_buffer.flags |= USB_BUFFER_SHORT_END;
  }
  ctrl_buffer.next = NULL;
  ctrl_buffer.data = (uint8_t*)data;
  ctrl_buffer.left = len;
  ctrl_buffer.id = IN_ID;
  usb_submit_xmit_buffer(0,&ctrl_buffer);
}

static uint8_t error_stall = 0;

void
usb_error_stall()
{
  error_stall = 1;
  usb_arch_control_stall(0);
}

void
usb_send_ctrl_status()
{
  if (ctrl_buffer.flags & USB_BUFFER_SUBMITTED) return;
  ctrl_buffer.flags = USB_BUFFER_NOTIFY | USB_BUFFER_IN;
  ctrl_buffer.next = NULL;
  ctrl_buffer.data = NULL;
  ctrl_buffer.left = 0;
  ctrl_buffer.id = STATUS_IN_ID;
  usb_submit_xmit_buffer(0,&ctrl_buffer);
}

static usb_ctrl_data_callback data_callback = NULL;
static uint8_t *ctrl_data = NULL;
static unsigned int ctrl_data_len = 0;
void
usb_get_ctrl_data(uint8_t *data, unsigned int length,
		  usb_ctrl_data_callback cb)
{
  if (ctrl_buffer.flags & USB_BUFFER_SUBMITTED) return;
  PRINTF("usb_get_ctrl_data: %d\n",length);
  data_callback = cb;
  ctrl_data = data;
  ctrl_data_len = length;
  ctrl_buffer.flags = USB_BUFFER_NOTIFY;
  ctrl_buffer.next = NULL;
  ctrl_buffer.data = data;
  ctrl_buffer.left = length;
  ctrl_buffer.id = OUT_ID;
  usb_submit_recv_buffer(0,&ctrl_buffer);
}

#if 0

void
usb_set_user_process(struct process *p)
{
  user_process = p;
}
#endif

static void
get_device_descriptor()
{
  usb_send_ctrl_response((unsigned char*)&device_descriptor, sizeof(device_descriptor));
}

static void
get_string_descriptor()
{
#if OLD_STRING_DESCR
  if (LOW_BYTE(usb_setup_buffer.wValue) == 0) {
    usb_send_ctrl_response((const unsigned char*)string_languages->lang_descr,
			   string_languages->lang_descr->bLength);
  } else {
    const struct usb_st_string_descriptor *descriptor;
    unsigned char l;
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
    PRINTF("Lang id %04x = table %p\n", usb_setup_buffer.wIndex, (void*)table);
    descriptor = table[LOW_BYTE(usb_setup_buffer.wValue) - 1];
    usb_send_ctrl_response((const unsigned char*)descriptor,
			   descriptor->bLength);
  }
#else
  const struct usb_st_string_descriptor *descriptor;
  descriptor = (struct usb_st_string_descriptor*)
    usb_class_get_string_descriptor(usb_setup_buffer.wIndex,
				    LOW_BYTE(usb_setup_buffer.wValue));
  if (!descriptor) {
    usb_error_stall();
    return;
  }
  usb_send_ctrl_response((const unsigned char*)descriptor,
			   descriptor->bLength);
#endif
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
  notify_user(USB_EVENT_CONFIG);
  if (usb_configuration_value != LOW_BYTE(usb_setup_buffer.wValue)) {
    usb_configuration_value = LOW_BYTE(usb_setup_buffer.wValue);
    usb_arch_set_configuration(usb_configuration_value);
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
  PRINTF("get_device_status\n");
  usb_send_ctrl_response((const unsigned char*)&usb_device_status,
		     sizeof(usb_device_status));
}

static void
get_endpoint_status()
{
  static uint16_t status;
  PRINTF("get_endpoint_status\n");
  if ((usb_setup_buffer.wIndex & 0x7f) == 0) {
    usb_send_ctrl_response((const unsigned char*)&zero_word,
		       sizeof(zero_word));
  } else {
    status = usb_arch_get_ep_status(usb_setup_buffer.wIndex);
    usb_send_ctrl_response((uint8_t*)&status, sizeof(status));
  }
}

static void
get_interface_status()
{
  PRINTF("get_interface_status\n");
  usb_send_ctrl_response((const unsigned char*)&zero_word,
			 sizeof(zero_word));
}

static void
get_interface()
{
  PRINTF("get_interface\n");
  if (usb_configuration_value == 0) usb_error_stall();
  else {
    usb_send_ctrl_response(&zero_byte,
		       sizeof(zero_byte));
  }
}


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
      PRINTF("Address: %d\n", LOW_BYTE(usb_setup_buffer.wValue));
      usb_flags |= USB_FLAG_ADDRESS_PENDING;
      /* The actual setting of the address is done when the status packet
	 is sent. */
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
#if 0
	config_msg.data.config = LOW_BYTE(usb_setup_buffer.wValue);
	notify_user(&config_msg);
#endif
      }
      break;
    default:
      return 0;
    }
    break;
  case 0x01: /* standard interface OUT requests */
    switch(usb_setup_buffer.bRequest) {
    case SET_INTERFACE:
      /* Change interface here if we support more than one */
      usb_send_ctrl_status();
      break;
    default:
      return 0;
    }
    break;
  case 0x02: /* standard endpoint OUT requests */
    switch(usb_setup_buffer.bRequest) {
    case SET_FEATURE:
    case CLEAR_FEATURE:
      if (usb_setup_buffer.wValue == ENDPOINT_HALT_FEATURE) {
	usb_arch_halt_endpoint(usb_setup_buffer.wIndex, usb_setup_buffer.bRequest== SET_FEATURE);
	usb_send_ctrl_status();
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
      PRINTF("Get report\n");
      send_ctrl_response((code u_int8_t*)&zero_byte,
			   sizeof(zero_byte));
      break;
    case GET_HID_IDLE:
      PRINTF("Get idle\n");
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
      PRINTF("Set idle\n");
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

static void
submit_setup(void)
{
  ctrl_buffer.next = NULL;
  ctrl_buffer.data = (uint8_t*)&usb_setup_buffer;
  ctrl_buffer.left = sizeof(usb_setup_buffer);
  ctrl_buffer.flags = (USB_BUFFER_PACKET_END | USB_BUFFER_SETUP
		       | USB_BUFFER_NOTIFY);
  ctrl_buffer.id = SETUP_ID;
  usb_submit_recv_buffer(0, &ctrl_buffer);
}

PROCESS(usb_process, "USB");

PROCESS_THREAD(usb_process, ev , data)
{
  PROCESS_BEGIN();
  PRINTF("USB process started\n");
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_EXIT) break;
    if (ev == PROCESS_EVENT_POLL) {
      unsigned int events = usb_arch_get_global_events();
      if (events) {
	if (events & USB_EVENT_RESET) {
	  submit_setup();
	  usb_configuration_value = 0;
	  notify_user(USB_EVENT_RESET);
	}
	if (events & USB_EVENT_SUSPEND) {
	  notify_user(USB_EVENT_SUSPEND);
	}
	if (events & USB_EVENT_RESUME) {
	  notify_user(USB_EVENT_RESUME);
	}
	
      }
      events = usb_get_ep_events(0);
      if (events) {
	if ((events & USB_EP_EVENT_NOTIFICATION)
	    && !(ctrl_buffer.flags & USB_BUFFER_SUBMITTED)) {
	  /* PRINTF("Endpoint 0\n"); */
	  if (ctrl_buffer.flags & USB_BUFFER_FAILED) {
	    /* Something went wrong with the buffer, just wait for a
	       new SETUP packet */
	    PRINTF("Discarded\n");
	    submit_setup();
	  } else if (ctrl_buffer.flags & USB_BUFFER_SETUP) {
	    struct USBRequestHandlerHook *hook = usb_request_handler_hooks;

	    PRINTF("Setup\n");
	    {
	    unsigned int i;
	    for (i = 0; i< 8; i++) PRINTF(" %02x", ((unsigned char*)&usb_setup_buffer)[i]);
	    PRINTF("\n");
	    }
	    
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
	      PRINTF("Unhandled setup: %02x %02x %04x %04x %04x\n",
		     usb_setup_buffer.bmRequestType, usb_setup_buffer.bRequest,
		     usb_setup_buffer.wValue, usb_setup_buffer.wIndex,
		     usb_setup_buffer.wLength);
	    }
	    /* Check if any handler stalled the pipe, if so prepare for
	       next setup */
	    if (error_stall) {
	      error_stall = 0;
	      submit_setup();
	    }
	  } else {
	    if (ctrl_buffer.id == IN_ID) {
	      /* Receive status stage */
	      PRINTF("Status OUT\n");
	      ctrl_buffer.flags =  USB_BUFFER_NOTIFY;
	      ctrl_buffer.next = NULL;
	      ctrl_buffer.data = NULL;
	      ctrl_buffer.left = 0;
	      ctrl_buffer.id = STATUS_OUT_ID;
	      usb_submit_recv_buffer(0,&ctrl_buffer);
	    } else if (ctrl_buffer.id == STATUS_OUT_ID) {
	      PRINTF("Status OUT done\n");
	      submit_setup();
	    } else if (ctrl_buffer.id == STATUS_IN_ID) {
	      PRINTF("Status IN done\n");
	      if (usb_flags & USB_FLAG_ADDRESS_PENDING) {
		while(usb_send_pending(0));
		usb_arch_set_address(LOW_BYTE(usb_setup_buffer.wValue));
		usb_flags &= ~USB_FLAG_ADDRESS_PENDING;
	      }
	      submit_setup();
	    } else if (ctrl_buffer.id == OUT_ID) {
	      PRINTF("OUT\n");
	      if (data_callback) {
		data_callback(ctrl_data, ctrl_data_len- ctrl_buffer.left);
	      } else {
		usb_send_ctrl_status();
	      }
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
  usb_arch_setup();
  process_start(&usb_process, NULL);
  usb_arch_set_global_event_process(&usb_process);
  usb_set_ep_event_process(0, &usb_process);

  usb_register_request_handler(&standard_request_hook);
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

void
usb_prepend_request_handler(struct USBRequestHandlerHook *hook)
{
  hook->next = usb_request_handler_hooks;
  usb_request_handler_hooks = hook;
}


unsigned int
usb_get_current_configuration(void)
{
  return usb_configuration_value;
}

void
usb_setup_bulk_endpoint(unsigned char addr)
{
  usb_arch_setup_bulk_endpoint(addr);
}

void
usb_setup_interrupt_endpoint(unsigned char addr)
{
  usb_arch_setup_interrupt_endpoint(addr);
}

void
usb_disable_endpoint(uint8_t addr)
{
  usb_arch_discard_all_buffers(addr);
  usb_arch_disable_endpoint(addr);
}

void
usb_discard_all_buffers(uint8_t addr)
{
  usb_arch_discard_all_buffers(addr);
}

void
usb_halt_endpoint(uint8_t addr, int halt)
{
  usb_arch_halt_endpoint(addr, halt);
}

int
usb_send_pending(uint8_t addr)
{
  return usb_arch_send_pending(addr);
}

