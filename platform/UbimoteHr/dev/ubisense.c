
#include "contiki.h"
#include "ubisense.h"
#include "cpu/cc2538/dev/i2c.h"
#include "clock.h"

#define SHT21_I2C_ADDR 			0x40
#define SHT21_TRIG_TEMP_HOLD		0xE3
#define SHT21_TRIG_HUMI_HOLD		0xE5
#define SHT21_TRIG_TEMP_NOHOLD		0xF3
#define SHT21_TRIG_HUMI_NOHOLD		0xF5

#define SHT21_WRITE_USER_REG		0xE6
#define SHT21_READ_USER_REG		0xE7
#define SHT21_SOFT_RESET		0xFE

void ubisense_sht21_init(void){

	i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
	i2c_single_send(SHT21_I2C_ADDR, SHT21_SOFT_RESET);
}

uint16_t ubisense_sht21_raw_temperature(void){

	uint8_t data[2];
	uint16_t raw_temp_data;
	i2c_single_send(SHT21_I2C_ADDR, SHT21_SOFT_RESET);
	clock_delay_usec(15000);
	
	i2c_single_send(SHT21_I2C_ADDR, SHT21_TRIG_TEMP_NOHOLD);
	clock_delay_usec(50000);
	clock_delay_usec(50000);
	
	i2c_burst_receive(SHT21_I2C_ADDR, data, 2);
	raw_temp_data = ((((uint16_t)(data[0])) << 8) | (data[1] & 0xFC));
	return raw_temp_data;
}

float ubisense_sht21_calibrate_temperature(uint16_t temperature){

	return (-46.85 + 175.72 / 65536.0 * (float)(temperature));
}

uint16_t ubisense_sht21_raw_humidity(void){

	uint8_t data[2];
	uint16_t raw_humid_data;
	
	i2c_single_send(SHT21_I2C_ADDR, SHT21_SOFT_RESET);
	clock_delay_usec(15000);
	
	i2c_single_send(SHT21_I2C_ADDR, SHT21_TRIG_HUMI_NOHOLD);
	clock_delay_usec(50000);
	clock_delay_usec(50000);
	
	i2c_burst_receive(SHT21_I2C_ADDR, data, 2);
	raw_humid_data = ((((uint16_t)(data[0])) << 8) | (data[1] & 0xFC));
	
	return raw_humid_data;
}

float ubisense_sht21_calibrate_humidity(uint16_t raw_data){

	return (-6.0 + 125.0 / 65536.0 * (float)(raw_data));
}
