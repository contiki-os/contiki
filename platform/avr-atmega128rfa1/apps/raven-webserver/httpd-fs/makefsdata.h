#include <avr/eeprom.h>

/* Link layer ipv6 address will become fe80::ff:fe:1 */
const uint8_t default_mac_address[8]  PROGMEM = {0x02, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x01};
const uint8_t default_server_name[16] PROGMEM = "ATMEGA128rfa1";
const uint8_t default_domain_name[30] PROGMEM = "localhost";
uint8_t eemem_mac_address[8]      EEMEM = {0x02, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x01};
uint8_t eemem_server_name[16]     EEMEM = "ATMEGA128rfa1";
uint8_t eemem_domain_name[30]     EEMEM = "localhost";

