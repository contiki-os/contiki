#include <avr/eeprom.h>

/* Link layer ipv6 address will become fe80::2 */
uint8_t mac_address[8]  EEMEM = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
uint8_t server_name[16] EEMEM = "huginn";
uint8_t domain_name[30] EEMEM = "localhost";
