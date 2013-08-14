#ifndef __USB_ARCH_H__0Z52ZDP0H6__
#define __USB_ARCH_H__0Z52ZDP0H6__

#include "contiki-conf.h"
#include "usb-api.h"


/* Includes control endpoint 0 */
#ifndef USB_MAX_ENDPOINTS
#define USB_MAX_ENDPOINTS 4
#endif

#ifndef CTRL_EP_SIZE
#define CTRL_EP_SIZE 8
#endif

#ifndef USB_EP1_SIZE
#define USB_EP1_SIZE 8
#endif
#ifndef USB_EP2_SIZE
#define USB_EP2_SIZE 8
#endif
#ifndef USB_EP3_SIZE
#define USB_EP3_SIZE 8
#endif
#ifndef USB_EP4_SIZE
#define USB_EP4_SIZE 0
#endif
#ifndef USB_EP5_SIZE
#define USB_EP5_SIZE 0
#endif
#ifndef USB_EP6_SIZE
#define USB_EP6_SIZE 0
#endif
#ifndef USB_EP7_SIZE
#define USB_EP7_SIZE 0
#endif


#ifndef MAX_CTRL_DATA
#define MAX_CTRL_DATA 128
#endif

void usb_arch_setup(void);

void usb_arch_setup_control_endpoint(uint8_t addr);

void usb_arch_setup_bulk_endpoint(uint8_t addr);

void usb_arch_setup_interrupt_endpoint(uint8_t addr);

void usb_arch_disable_endpoint(uint8_t addr);

void usb_arch_discard_all_buffers(uint8_t addr);

/* Stall a control endpoint. The stall will be cleared when the next
 * SETUP token arrives. */
void usb_arch_control_stall(uint8_t addr);

/* Set or remove a HALT condition on an endpoint */
void usb_arch_halt_endpoint(uint8_t addr, int halt);

void usb_arch_set_configuration(uint8_t usb_configuration_value);

uint16_t usb_arch_get_ep_status(uint8_t addr);

void usb_arch_set_address(uint8_t addr);

/* Select what process should be polled when a global event occurs. Intended for the protocol handler. Applications should use usb_set_global_event_process  */
void usb_arch_set_global_event_process(struct process *p);

unsigned int usb_arch_get_global_events(void);

/* Return true if not all data has been sent to the host */
int usb_arch_send_pending(uint8_t ep_addr);

#endif /* __USB_ARCH_H__0Z52ZDP0H6__ */
