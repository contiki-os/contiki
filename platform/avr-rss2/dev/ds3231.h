#ifndef DS3231_H
#define DS3231_H
#include <stdio.h>

typedef struct {
  uint8_t secs;
  uint8_t mins;
  uint8_t hours;
  uint8_t wday;
  uint8_t day;
  uint8_t month;
  uint8_t year;
} datetime_t;


void  ds3231_init();
void  ds3231_set_datetime(datetime_t *datetime);
void  ds3231_get_datetime(datetime_t *datetime);
float ds3231_get_temp();
uint8_t bcd_to_dec(uint8_t d);
uint8_t dec_to_bcd(uint8_t d);


#endif