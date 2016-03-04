/*

  Contiki library for DS18B20 temperature sensor - 
  For more details see http://xxx

  Author - 
  Author - 

  License - GPLv3

*/

#include <sys/clock.h>
#include "contiki.h"
#include "../platform/avr-rss2/rss2.h"

// Constants

#define DS18B20_1_PIN OW_BUS_0
#define DS18B20_1_IN  PIND
#define DS18B20_1_OUT PORTD
#define DS18B20_1_DDR DDRD

#define OW_SET_PIN_LOW()   (DS18B20_1_OUT &= ~ (1 << DS18B20_1_PIN))
#define OW_SET_PIN_HIGH()  (DS18B20_1_OUT |= (1 << DS18B20_1_PIN))
#define OW_SET_OUTPUT()    (DS18B20_1_DDR |= (1 << DS18B20_1_PIN))
#define OW_SET_INPUT()     (DS18B20_1_DDR &= ~ (1 << DS18B20_1_PIN))
#define OW_GET_PIN_STATE() ((DS18B20_1_IN & (1 << DS18B20_1_PIN)) ? 1 : 0)

#define DS18B20_COMMAND_READ_SCRATCH_PAD 0xBE
#define DS18B20_COMMAND_START_CONVERSION 0x44
#define DS18B20_COMMAND_SKIP_ROM 0xCC

// probe_for_ds18b20 probes for the sensor. Returns 0 on failure, 1 on success
// Assumptions: only one sensor on the "1-wire bus", on port DS18B20_1_PIN

extern uint8_t ds18b20_probe(void);
extern uint8_t ds18b20_get_temp(int16_t *temperature);
extern uint8_t crc8_ds18b20(uint8_t* buf, uint8_t buf_len);
