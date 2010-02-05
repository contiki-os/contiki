#ifndef __USB_API_H__SYN81IFYBN__
#define __USB_API_H__SYN81IFYBN__

#include <sys/process.h>

typedef struct _USBBuffer USBBuffer;

struct _USBBuffer
{
  USBBuffer *next;	/* Pointer to next buffer in chain */
  uint8_t *data;	/* Where to read/write data next */
  uint16_t left;	/* Remaining length of buffer. */
  uint16_t flags;
  uint32_t id;	/* User data */
};

/* Buffer owned by the USB code, cleared when done */
#define USB_BUFFER_SUBMITTED 0x01

/* Write a short packet at end of buffer or release buffer when a
   short packet is received. */
#define USB_BUFFER_SHORT_END 0x02

/* Release buffer as soon as any received data has been written in it. */
#define USB_BUFFER_PACKET_END 0x04

/* Notify the user when the buffer is released */
#define USB_BUFFER_NOTIFY 0x08

/* Packet should be sent to host. */
#define USB_BUFFER_IN 0x40

/* Used for receiving SETUP packets. If a SETUP packet is received and
   the next buffers doesn't have this flag set, they will be skipped
   until one is found. The associated buffer must be at least 8 bytes */
#define USB_BUFFER_SETUP 0x20

/* HALT the endpoint at this point. Only valid for bulk and interrupt
   endpoints */
#define USB_BUFFER_HALT 0x100

/* Flags set by system */

/* The last packet written to this buffer was short. */
#define USB_BUFFER_SHORT_PACKET 0x10

/* The operation associated with this buffer failed. I.e. it was discarded since it didn't match the received SETUP packet. */
#define USB_BUFFER_FAILED 0x80

/* Architecture specific flags */
#define USB_BUFFER_ARCH_FLAG_1 0x1000
#define USB_BUFFER_ARCH_FLAG_2 0x2000
#define USB_BUFFER_ARCH_FLAG_3 0x4000
#define USB_BUFFER_ARCH_FLAG_4 0x8000

void
usb_setup(void);


/* Read only */
struct USBRequestHandler
{
  uint8_t request_type;
  uint8_t request_type_mask;
  uint8_t request;
  uint8_t request_mask;
  /* Returns true if it handled the request, if false let another handler try*/
  unsigned int (*handler_func)();
};
  
/* Must be writeable */
struct USBRequestHandlerHook
{
  struct USBRequestHandlerHook *next;
  const struct USBRequestHandler * const handler;
};

void
usb_register_request_handler(struct USBRequestHandlerHook *hook);

void
usb_prepend_request_handler(struct USBRequestHandlerHook *hook);

void
usb_setup_bulk_endpoint(uint8_t addr);
void
usb_setup_interrupt_endpoint(uint8_t addr);

/* Submit a chain of buffers to be filled with received data. Last
   buffer must have next set to NULL. */
void
usb_submit_recv_buffer(uint8_t ep_addr, USBBuffer *buffer);

/* Submit a chain of buffers to be sent. Last buffer must have next
   set to NULL. When submitting packets to receive or send data in on
   a control enpoint, all packets in the data stage must be submitted
   at the same time. */
void
usb_submit_xmit_buffer(uint8_t ep_addr, USBBuffer *buffer);

/* Return true if not all data has been sent to the host */
int 
usb_send_pending(uint8_t ep_addr);

/* Release all buffers submitted to the endpoint and discard any
   buffered data. */
void
usb_discard_all_buffers(uint8_t ep_addr);

void
usb_disable_endpoint(uint8_t addr);

/* Set or remove a HALT condition on an endpoint */
void
usb_halt_endpoint(uint8_t addr, int halt);

/* Select what process should be polled when buffers with the
   USB_BUFFER_NOTIFY flag set is released from the endpoint */
void
usb_set_ep_event_process(uint8_t addr, struct process *p);

/* Select what process should be polled when a global event occurs */
void
usb_set_global_event_process(struct process *p);

/* Global events */
#define USB_EVENT_CONFIG 0x01 
#define USB_EVENT_SUSPEND 0x02 
#define USB_EVENT_RESUME 0x04
#define USB_EVENT_RESET 0x08

/* Returns global events that has occured since last time this
   function was called */
unsigned int
usb_get_global_events(void);


#define USB_EP_EVENT_NOTIFICATION 0x01
unsigned int
usb_get_ep_events(uint8_t addr);

unsigned int
usb_get_current_configuration(void);

#endif /* __USB_API_H__SYN81IFYBN__ */
