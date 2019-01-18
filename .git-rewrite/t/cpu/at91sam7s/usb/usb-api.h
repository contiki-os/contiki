#ifndef __USB_API_H__SYN81IFYBN__
#define __USB_API_H__SYN81IFYBN__

#include <sys/process.h>

void
usb_setup(void);

void
usb_set_user_process(struct process *p);

void
usb_setup_bulk_endpoint(unsigned char addr,
			unsigned char *buffer, unsigned int buf_size);
void
usb_setup_interrupt_endpoint(unsigned char addr,
			     unsigned char *buffer, unsigned int buf_size);

/* Get a pointer to a buffer dat of length lenp.
   To which USB data can be written. */
void
usb_send_buffer_get(unsigned char ep_addr, unsigned int offset,
		    unsigned char **dat, unsigned int *lenp);

/* Notify the USB subsystem that data has been written to the buffer returned
   by usb_send_buffer_get. */
void
usb_send_buffer_commit(unsigned char ep_addr, unsigned int len);

unsigned int
usb_send_data(unsigned char ep_addr,
	      const unsigned char *dat, unsigned int len);

unsigned int
usb_recv_data(unsigned char ep_addr, unsigned char *dat, unsigned int len);

void
usb_disable_endpoint(unsigned char addr);

/* Asynchronous */
#define USB_USER_MSG_TYPE_CONFIG 0x01 
#define USB_USER_MSG_TYPE_SUSPEND 0x02 
#define USB_USER_MSG_TYPE_RESUME 0x03

/* Synchronous, the supplied data is only valid during the event */
#define USB_USER_MSG_TYPE_EP_OUT(ep_addr) ((((ep_addr) & 0x7f)<<4) | 0x01)
#define USB_USER_MSG_TYPE_EP_IN(ep_addr) ((((ep_addr) & 0x7f)<<4) | 0x02)

struct usb_user_msg {
  unsigned int type;
  union {
    /* For EPx_OUT */
    unsigned short length;
    /* For CONFIG */
    unsigned char config;
  } data;
};


#endif /* __USB_API_H__SYN81IFYBN__ */
