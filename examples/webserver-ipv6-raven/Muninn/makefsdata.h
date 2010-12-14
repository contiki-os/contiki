#include <avr/eeprom.h>

/* Link layer ipv6 address will become fe80::1 */
uint8_t mac_address[8]  EEMEM = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
uint8_t server_name[16] EEMEM = "muninn";
uint8_t domain_name[30] EEMEM = "localhost";
