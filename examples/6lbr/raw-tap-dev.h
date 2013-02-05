#ifndef RAW_TAP_DEV_H_
#define RAW_TAP_DEV_H_

extern void tun_init();
extern void tun_output(uint8_t *data, int len);
extern int tun_input(unsigned char *data, int maxlen);

#endif
