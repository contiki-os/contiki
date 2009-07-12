#include <usb-api.h>

#ifdef USB_STM32F103_ENABLE_ALT_COPY 
/* Use an alternate data copying function */
#define USB_BUFFER_ARCH_ALT_COPY USB_BUFFER_ARCH_FLAG_1

/* Copy len bytes of data from the buffer to dedicated USB
   memory. buffer->data must be updated */
extern void
copy_to_hw_buffer(USBBuffer *buffer,unsigned int offset, unsigned int len);

/* Copy len bytes of data to the buffer from dedicated USB memory.
   buffer->data must be updated */
extern void
copy_from_hw_buffer(USBBuffer *buffer,unsigned int offset, unsigned int len);
#endif

