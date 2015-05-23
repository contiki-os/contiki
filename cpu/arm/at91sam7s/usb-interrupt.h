#ifndef USB_INTERRUPT_H_0HRIPZ5SIA__
#define USB_INTERRUPT_H_0HRIPZ5SIA__

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

#endif /* USB_INTERRUPT_H_0HRIPZ5SIA__ */
