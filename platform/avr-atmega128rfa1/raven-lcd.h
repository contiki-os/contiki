#ifndef raven_lcd_h
#define raven_lcd_h

int raven_lcd_serial_input(unsigned char ch);
void raven_lcd_show_text(char *text);
PROCESS_NAME(raven_lcd_process);

/* Serial protocol */
#define SOF_CHAR 1
#define EOF_CHAR 4
#define NULL_CMD               (0x00)
#define SERIAL_CMD             (0x01)

/* Messages from the 1284p to the 3290p */
#define REPORT_PING            (0xC0)
#define REPORT_PING_BEEP       (0xC1)
#define REPORT_TEXT_MSG        (0xC2)
#define REPORT_WAKE            (0xC3)

/* Messages from the 3290p to the 1284p */
#define SEND_TEMP              (0x80)
#define SEND_PING              (0x81)
#define SEND_ADC2              (0x82)
#define SEND_SLEEP             (0x83)
#define SEND_WAKE              (0x84)

#endif
