#include <avr/eeprom.h>

/* Link layer ipv6 address will become fe80::11:22ff:fe33:4456 */
uint8_t mac_address[8]  EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x56};
uint8_t server_name[16] EEMEM = "muninn";
uint8_t domain_name[30] EEMEM = "localhost";

