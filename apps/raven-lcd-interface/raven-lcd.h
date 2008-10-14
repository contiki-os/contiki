#ifndef raven_lcd_h
#define raven_lcd_h

int raven_lcd_serial_input(unsigned char ch);
PROCESS_NAME(raven_lcd_process);

/* Events for the Raven LCD processing loop */
#define PING_REQUEST 0xc1
#define PING_REPLY   0xc0
#define SERIAL_CMD   0x1


#endif
