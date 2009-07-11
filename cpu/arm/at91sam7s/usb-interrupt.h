#ifndef __USB_INTERRUPT_H__0HRIPZ5SIA__
#define __USB_INTERRUPT_H__0HRIPZ5SIA__

void 
usb_int (void);

void
usb_arch_transfer_complete(unsigned int hw_ep);

void
usb_arch_transfer_complete(unsigned int hw_ep);
void
usb_arch_reset_int();

void
usb_arch_suspend_int();

void
usb_arch_resume_int();

#endif /* __USB_INTERRUPT_H__0HRIPZ5SIA__ */
