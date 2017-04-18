

#ifndef UBISENSE_H_
#define UBISENSE_H_

void ubisense_sht21_init(void);
uint16_t ubisense_sht21_raw_temperature(void);
float ubisense_sht21_calibrate_temperature(uint16_t);
uint16_t ubisense_sht21_raw_humidity(void);
float ubisense_sht21_calibrate_humidity(uint16_t);

#endif
