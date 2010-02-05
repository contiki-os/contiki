#include "usb-msc-bulk.h"
#include <usb-api.h>
#include <usb-core.h>
#include <sys/process.h>
#include <stdio.h>
#include <string.h>

#define DEBUG     

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static const uint8_t max_lun = 0;

static USBBuffer data_usb_buffer[USB_MSC_BUFFERS];
static unsigned int buffer_lengths[USB_MSC_BUFFERS];

static unsigned int buf_first = 0; /* First prepared buffer */
static unsigned int buf_free = 0;	/* First free buffer */
static unsigned int buf_submitted = 0; /* Oldest submitted buffer */

#define USB_BUFFER_ID_UNUSED 0
#define USB_BUFFER_ID_CBW 1
#define USB_BUFFER_ID_CSW 2
#define USB_BUFFER_ID_DATA 3
#define USB_BUFFER_ID_DISCARD 4
#define USB_BUFFER_ID_HALT 5
#define USB_BUFFER_ID_MASK 0x07

static struct usb_msc_bulk_cbw cbw_buffer;
static struct usb_msc_bulk_csw csw_buffer;

#define BULK_OUT 0x02
#define BULK_IN 0x81

PROCESS(usb_mass_bulk_process, "USB mass storage bulk only process");

static process_event_t reset_event;

static struct usb_msc_command_state state;

/* Handle wrapping */
#define PREV_BUF(x) (((x) == 0) ? USB_MSC_BUFFERS - 1 : (x) - 1)
#define NEXT_BUF(x) (((x) < (USB_MSC_BUFFERS-1)) ? (x) + 1 : 0)
void
usb_msc_send_data_buf_flags(const uint8_t *data, unsigned int len,
			    unsigned int flags, uint16_t buf_flags)
{
  USBBuffer *buffer = &data_usb_buffer[buf_free];
  if (buffer->id != USB_BUFFER_ID_UNUSED) {
    printf("Data IN buffer busy\n");
    return;
  }
  buffer->flags = USB_BUFFER_NOTIFY | buf_flags;
  buffer->next = NULL;
  buffer->data = (uint8_t*)data;
  buffer->left = len;
  buffer_lengths[buf_free] = len;
  buffer->id = USB_BUFFER_ID_DATA | flags;
  if (buf_free != buf_first) {
    data_usb_buffer[PREV_BUF(buf_free)].next = buffer;
  }
  state.cmd_data_submitted += len;
  buf_free = NEXT_BUF(buf_free);
  /* PRINTF("usb_msc_send_data: %d\n", len); */
  if (flags & USB_MSC_DATA_SEND) {
    usb_submit_xmit_buffer(BULK_IN, &data_usb_buffer[buf_first]);
    buf_first = buf_free;
    /* PRINTF("usb_msc_send_data: sent\n"); */
  } else if (flags & USB_MSC_DATA_LAST) {
    /* Cancel transmission */
    PRINTF("Send last\n");
    buf_first = buf_free;
    process_poll(&usb_mass_bulk_process);
  }
}

void
usb_msc_send_data(const uint8_t *data, unsigned int len, unsigned int flags)
{
  usb_msc_send_data_buf_flags(data, len, flags,0);
}

void
usb_msc_receive_data_buf_flags(uint8_t *data, unsigned int len,
			       unsigned int flags, uint16_t buf_flags)
{
  USBBuffer *buffer = &data_usb_buffer[buf_free];
  if (buffer->id != USB_BUFFER_ID_UNUSED) {
    printf("Data OUT buffer busy\n");
    return;
  }
  buffer->flags = USB_BUFFER_NOTIFY | buf_flags;
  buffer->next = NULL;
  buffer->data = data;
  buffer->left = len;
  buffer_lengths[buf_free] = len;
  buffer->id = USB_BUFFER_ID_DATA | flags;
  if (buf_free != buf_first) {
    data_usb_buffer[PREV_BUF(buf_free)].next = buffer;
  }
  state.cmd_data_submitted += len;
  buf_free = NEXT_BUF(buf_free);
  if (flags & USB_MSC_DATA_RECEIVE) {
    usb_submit_recv_buffer(BULK_OUT, &data_usb_buffer[buf_first]);
    buf_first = buf_free;
  } else if (flags & USB_MSC_DATA_LAST) {
    usb_discard_all_buffers(BULK_OUT);
    /* Mark the discarded buffers as unused */
    while(buf_submitted !=  PREV_BUF(buf_free)) {
      data_usb_buffer[buf_submitted].id = USB_BUFFER_ID_UNUSED;
      buf_submitted = NEXT_BUF(buf_submitted);
    }
    buf_first = buf_free;
    process_poll(&usb_mass_bulk_process);
  }
}

void
usb_msc_receive_data(uint8_t *data, unsigned int len, unsigned int flags)
{
  usb_msc_receive_data_buf_flags(data,len,flags, 0);
}

static unsigned int
handle_mass_bulk_requests()
{
  switch(usb_setup_buffer.bmRequestType) {
  case 0x21: /* interface OUT requests */
    switch(usb_setup_buffer.bRequest) {
    case MASS_BULK_RESET:
      PRINTF("Mass storage reset\n");
      process_post(&usb_mass_bulk_process, reset_event, NULL);
      return 1;
    }
    break;
  case 0xa1: /* interface IN requests */
    switch(usb_setup_buffer.bRequest) {
    case MASS_BULK_GET_MAX_LUN:
      PRINTF("Get LUN\n");
      usb_send_ctrl_response(&max_lun, sizeof(max_lun));
      return 1;
    }
    break;
  }
  return 0;
}

static const struct USBRequestHandler mass_bulk_request_handler =
  {
    0x21, 0x7f,
    0x00, 0x00,
    handle_mass_bulk_requests
  };

static struct USBRequestHandlerHook mass_bulk_request_hook =
  {
    NULL,
    &mass_bulk_request_handler
  };

static void
send_csw(void)
{
  USBBuffer *buffer = &data_usb_buffer[buf_free];
  if (buffer->id != USB_BUFFER_ID_UNUSED) {
    printf("CSW buffer busy\n");
    return;
  }

  csw_buffer.dCSWSignature = MASS_BULK_CSW_SIGNATURE;
  csw_buffer.dCSWTag = cbw_buffer.dCBWTag;
  csw_buffer.dCSWDataResidue =
    cbw_buffer.dCBWDataTransferLength - state.cmd_data_submitted;
  csw_buffer.bCSWStatus = state.status;
  
  buffer->flags = USB_BUFFER_NOTIFY;
  buffer->next = NULL;
  buffer->data =(uint8_t*)&csw_buffer ;
  buffer->left = sizeof(csw_buffer);
  buffer->id = USB_BUFFER_ID_CSW;
  if (buf_free != buf_first) {
    data_usb_buffer[PREV_BUF(buf_free)].next = buffer;
  }
  buf_free = NEXT_BUF(buf_free);
  usb_submit_xmit_buffer(BULK_IN, &data_usb_buffer[buf_first]);
  buf_first = buf_free;

  PRINTF("CSW sent: %ld\n", sizeof(csw_buffer));
}

static void
submit_cbw_buffer(void)
{
  USBBuffer *buffer = &data_usb_buffer[buf_free];
  if (buffer->id != USB_BUFFER_ID_UNUSED) {
    printf("CBW buffer busy\n");
    return;
  }
  buffer->flags = USB_BUFFER_NOTIFY;
  buffer->next = NULL;
  buffer->data = (uint8_t*)&cbw_buffer;
  buffer->left = sizeof(cbw_buffer);
  buffer->id = USB_BUFFER_ID_CBW;
  if (buf_free != buf_first) {
    data_usb_buffer[PREV_BUF(buf_free)].next = buffer;
  }
  buf_free = NEXT_BUF(buf_free);
  usb_submit_recv_buffer(BULK_OUT, &data_usb_buffer[buf_first]);
  PRINTF("CBW submitted: %d\n", buf_first);
  buf_first = buf_free;
}

static void
submit_halt(uint8_t addr)
{
  USBBuffer *buffer = &data_usb_buffer[buf_free];
  if (buffer->id != USB_BUFFER_ID_UNUSED) {
    printf("CBW buffer busy\n");
    return;
  }
  buffer->flags = USB_BUFFER_NOTIFY | USB_BUFFER_HALT;
  buffer->next = NULL;
  buffer->data = NULL;
  buffer->left = 0;
  buffer->id = USB_BUFFER_ID_HALT;
  if (buf_free != buf_first) {
    data_usb_buffer[PREV_BUF(buf_free)].next = buffer;
  }
  buf_free = NEXT_BUF(buf_free);
  if (addr & 0x80) {
    usb_submit_xmit_buffer(addr, &data_usb_buffer[buf_first]);
  } else {
    usb_submit_recv_buffer(addr, &data_usb_buffer[buf_first]);
  }
  PRINTF("HALT submitted %p\n",buffer);
  buf_first = buf_free;
}

static USBBuffer *
get_next_buffer(uint8_t addr, uint32_t id)
{
  unsigned int events;
  events = usb_get_ep_events(addr);
  if (events & USB_EP_EVENT_NOTIFICATION) {
    USBBuffer *buffer = &data_usb_buffer[buf_submitted];
    if (!(buffer->flags & USB_BUFFER_SUBMITTED)) {
#ifdef DEBUG
      if (id != (buffer->id & USB_BUFFER_ID_MASK)) {
	printf("Wrong buffer ID expected %d, got %d\n",
	       (int)id, (int)buffer->id);
      }
#endif
      if ((buffer->id & USB_BUFFER_ID_MASK) == USB_BUFFER_ID_DATA) {
	state.cmd_data_transfered +=
	  buffer_lengths[buf_submitted] - buffer->left;
      }
      buffer->id = USB_BUFFER_ID_UNUSED;
      buf_submitted =NEXT_BUF(buf_submitted);
      return buffer;
    }
  }
  return NULL;
}

PROCESS(usb_mass_bulk_request_process, "USB mass storage request process");

PROCESS_THREAD(usb_mass_bulk_request_process, ev , data)
{
  PROCESS_BEGIN();
 reset_state:
  usb_discard_all_buffers(BULK_OUT);
  usb_discard_all_buffers(BULK_IN);
  memset(data_usb_buffer, 0, sizeof(data_usb_buffer));
  buf_first = 0;
  buf_free = 0;
  buf_submitted = 0;
  submit_cbw_buffer();
 receive_cbw_state:
  PRINTF("receive_cbw_state\n");
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == reset_event) goto reset_state;
    if (ev == PROCESS_EVENT_POLL) {
      USBBuffer *buffer;
      if ((buffer = get_next_buffer(BULK_OUT, USB_BUFFER_ID_CBW))) {
	
	/* CBW */
	if (cbw_buffer.dCBWSignature == MASS_BULK_CBW_SIGNATURE) {
	  usb_msc_handler_status ret;
	  PRINTF("Got CBW seq %d\n",(int)cbw_buffer.dCBWTag);
	  state.command = cbw_buffer.CBWCB;
	  state.command_length = cbw_buffer.bCBWCBLength;
	  state.status = MASS_BULK_CSW_STATUS_FAILED;
	  state.data_cb = NULL;
	  state.cmd_data_submitted = 0;
	  state.cmd_data_transfered = 0;
	  ret = usb_msc_handle_command(&state);
	  if (ret == USB_MSC_HANDLER_OK) {
	    state.status = MASS_BULK_CSW_STATUS_PASSED;
	  } else if (ret == USB_MSC_HANDLER_FAILED) {
	    state.status = MASS_BULK_CSW_STATUS_FAILED;
	  }
	  if (ret != USB_MSC_HANDLER_DELAYED
	      && buf_submitted == buf_free) {
	    if (cbw_buffer.dCBWDataTransferLength > 0) {
	      /* HALT the apropriate endpoint */
	      if (cbw_buffer.bmCBWFlags & MASS_BULK_CBW_FLAG_IN) {
		submit_halt(BULK_IN);
	      } else {
		submit_halt(BULK_OUT);
	      }
	      /* Wait for HALT */
	      while(1) {
		PROCESS_WAIT_EVENT();
		if (ev == reset_event) goto reset_state;
		if (ev == PROCESS_EVENT_POLL) {
		  USBBuffer *buffer =
		    get_next_buffer(BULK_IN, USB_BUFFER_ID_HALT);
		  if (buffer && (buffer->flags & USB_BUFFER_HALT)) break;
		}
	      }
	    }
	    goto send_csw_state;
	  }
	  if (cbw_buffer.bmCBWFlags & MASS_BULK_CBW_FLAG_IN) {
	    goto send_data_state;
	  } else {
	    goto receive_data_state;
	  }
	} else {
	  printf("Invalid CBW\n");
	  submit_halt(BULK_IN);
	  submit_halt(BULK_OUT);
	  while(1) {
	    PROCESS_WAIT_EVENT();
	    if (ev == reset_event) goto reset_state;
	    if (ev == PROCESS_EVENT_POLL) {
	      USBBuffer *buffer = get_next_buffer(BULK_IN, USB_BUFFER_ID_HALT);
	      if (buffer && (buffer->flags & USB_BUFFER_HALT)) break;
	    }
	  }
	  while(1) {
	    PROCESS_WAIT_EVENT();
	    if (ev == reset_event) goto reset_state;
	    if (ev == PROCESS_EVENT_POLL) {
	      USBBuffer *buffer = get_next_buffer(BULK_OUT, USB_BUFFER_ID_HALT);
	      if (buffer && (buffer->flags & USB_BUFFER_HALT)) break;
	    }
	  }
	  /* CBW */
	  goto receive_cbw_state;
	}
      }
    }
  }
  
 send_data_state:
  PRINTF("send_data_state\n");
  while(1) {
    uint8_t id = 0;
    /* Wait for any data to be sent */
    while (buf_submitted == buf_free) {
      PRINTF("Wait data\n");
      PROCESS_WAIT_EVENT();
    }
#if 0
    /* Send CSW early to improve throughput, unless we need to HALT
       the endpoint due to short data */
    if ((data_usb_buffer[PREV_BUF(buf_free)].id & USB_MSC_DATA_LAST)
	&& state.cmd_data_submitted == cbw_buffer.dCBWDataTransferLength) {
      send_csw();
    }
#endif
    /* Wait until the current buffer is free */
    while (data_usb_buffer[buf_submitted].flags & USB_BUFFER_SUBMITTED) {
      PROCESS_WAIT_EVENT();
    }
    while (!(data_usb_buffer[buf_submitted].flags & USB_BUFFER_SUBMITTED)) {
      id = data_usb_buffer[buf_submitted].id;
      /* PRINTF("id: %02x\n", id);  */
      if (id == USB_BUFFER_ID_UNUSED) break;
      state.cmd_data_transfered += buffer_lengths[buf_submitted];
      data_usb_buffer[buf_submitted].id = USB_BUFFER_ID_UNUSED;
      buf_submitted =NEXT_BUF(buf_submitted);
      if (id & USB_MSC_DATA_DO_CALLBACK) {
	if (state.data_cb) {
	  state.data_cb(&state);
	}
      }
      
     
      if (id & USB_MSC_DATA_LAST) {
	break;
      }
    }
    if (id & USB_MSC_DATA_LAST) {
      break;
    }
  }
  if (state.cmd_data_submitted < cbw_buffer.dCBWDataTransferLength) {
    submit_halt(BULK_IN);
    while(1) {
      PROCESS_WAIT_EVENT();
      if (ev == reset_event) goto reset_state;
      if (ev == PROCESS_EVENT_POLL) {
	USBBuffer *buffer = get_next_buffer(BULK_IN , USB_BUFFER_ID_HALT);
	if (buffer) {
	  if (buffer->flags & USB_BUFFER_HALT) break;
	}
      }
    }
  }
  goto send_csw_state;

 receive_data_state:
  PRINTF("receive_data_state\n");
  while(1) {
    uint8_t id = 0;
    /* Wait for any buffers to be submitted */
    while (buf_submitted == buf_free) {
      PROCESS_WAIT_EVENT();
    }
    /* Wait until the current buffer is free */
    while (data_usb_buffer[buf_submitted].flags & USB_BUFFER_SUBMITTED) {
      PROCESS_WAIT_EVENT();
    }
    while (!(data_usb_buffer[buf_submitted].flags & USB_BUFFER_SUBMITTED)) {
      id = data_usb_buffer[buf_submitted].id;
      /* PRINTF("id: %02x\n", id); */
      state.cmd_data_transfered += buffer_lengths[buf_submitted];
      if (id == USB_BUFFER_ID_UNUSED) break;
      data_usb_buffer[buf_submitted].id = USB_BUFFER_ID_UNUSED;
      buf_submitted =NEXT_BUF(buf_submitted);
      if (id & USB_MSC_DATA_DO_CALLBACK) {
	if (state.data_cb) {
	  state.data_cb(&state);
	}
      }
      
      if (id & USB_MSC_DATA_LAST) {
	break;
      }
    }
    if (id & USB_MSC_DATA_LAST) {
      break;
    }
  
  }
  
  if (state.cmd_data_submitted < cbw_buffer.dCBWDataTransferLength) {
    submit_halt(BULK_OUT);
    while(1) {
      PROCESS_WAIT_EVENT();
      if (ev == reset_event) goto reset_state;
      if (ev == PROCESS_EVENT_POLL) {
	USBBuffer *buffer = get_next_buffer(BULK_OUT, USB_BUFFER_ID_HALT);
	if (buffer && (buffer->flags & USB_BUFFER_HALT)) break;
      }
    }
  }
  goto send_csw_state;

  
 send_csw_state:
  PRINTF("send_csw_state\n");
  if (data_usb_buffer[PREV_BUF(buf_free)].id != USB_BUFFER_ID_CSW) {
    send_csw();
  }
  submit_cbw_buffer();
  while(1) {
    if (ev == reset_event) goto reset_state;
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_POLL) {
      USBBuffer *buffer;
      if ((buffer = get_next_buffer(BULK_IN, USB_BUFFER_ID_CSW))) {
	goto receive_cbw_state;
      }
    }
  }
  goto receive_cbw_state;
  PROCESS_END();
}
     
PROCESS_THREAD(usb_mass_bulk_process, ev , data)
{
  PROCESS_BEGIN();
  reset_event = process_alloc_event();
  usb_msc_command_handler_init();
  usb_setup();
  usb_set_ep_event_process(BULK_IN, &usb_mass_bulk_request_process);
  usb_set_ep_event_process(BULK_OUT, &usb_mass_bulk_request_process);
  usb_set_global_event_process(process_current);
  usb_register_request_handler(&mass_bulk_request_hook);
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_EXIT) break;
    if (ev == PROCESS_EVENT_POLL) {
      unsigned int events = usb_get_global_events();
      if (events) {
	if (events & USB_EVENT_CONFIG) {
	  if (usb_get_current_configuration() != 0) {
	    PRINTF("Configured\n");
	    memset(data_usb_buffer, 0, sizeof(data_usb_buffer));
	    usb_setup_bulk_endpoint(BULK_IN);
	    usb_setup_bulk_endpoint(BULK_OUT);
	    process_start(&usb_mass_bulk_request_process,NULL);
	  } else {
	    process_exit(&usb_mass_bulk_request_process);
	    usb_disable_endpoint(BULK_IN);
	    usb_disable_endpoint(BULK_OUT);
	  }
	}
	if (events & USB_EVENT_RESET) {
	  PRINTF("RESET\n");
	  process_exit(&usb_mass_bulk_request_process);
	}
      }
    }
  }
  PROCESS_END();
}

void
usb_msc_bulk_setup()
{
  process_start(&usb_mass_bulk_process, NULL);
}
