/** \addtogroup cc2538-examples
 * @{
 *
 * \file
 *  /examples/cc2538dk/i2c-example/i2c-example.c
 *  An example of a simple I2C use for the cc2538dk platform and SHT21 
 *  or TCN75 temperature sensor
 * \author
 *  Mehdi Migault
 */
#include <contiki.h>
#include "i2c.h"

/** \name I2C pins
 * @{
 */
#define I2C_SDA_PORT	GPIO_B_NUM
#define I2C_SDA_PIN		3
#define I2C_SCL_PORT	GPIO_B_NUM
#define I2C_SCL_PIN		2
/** @} */

/** \name Sensor activation
 * @{
 */
#define TCN75_ENABLE	1	//Use TCN75
#define SHT21_ENABLE	0	//Use SHT21
/** @} */

/** \name Sensor's specific
 * @{
 */
#if TCN75_ENABLE
	#define TCN75_SLAVE_ADDRESS		0x48
	#define TCN75_TEMP_REGISTER		0x00
#endif //TCN75_ENABLE
#if SHT21_ENABLE
	#define SHT21_SLAVE_ADDRESS		0x40
	#define SHT21_TEMP_REGISTER		0xF3
	#define SHT21_CRC_POLYNOMIAL		0x131
	#define SHT21_DECIMAL_PRECISION		2
#endif //SHT21_ENABLE
/** @} */

#define MEASURE_INTERVAL_TIME_SECOND	5

/*---------------------------------------------------------------------------*/
static struct etimer timer;
static uint16_t temp;
/*---------------------------------------------------------------------------*/
void
print_float(float num, uint8_t preci)
{
	int integer=(int)num, decimal=0;
	char buf[20];
	preci = preci > 10 ? 10 : preci;
	num -= integer;
	while((num != 0) && (preci-- > 0)) {
		decimal *= 10;
		num *= 10;
		decimal += (int)num;
		num -= (int)num;
	}
	sprintf(buf, "%d.%d", integer, decimal);
	printf("%s", buf);
}
/*---------------------------------------------------------------------------*/
#if TCN75_ENABLE
uint8_t
read_temp_TCN75(uint16_t * data)
{
	/*
	 * Set the register to be read
	 * Should be done only one time after i2c_init();
	 */
	if(i2c_single_send(TCN75_SLAVE_ADDRESS, TCN75_TEMP_REGISTER) == I2C_MASTER_ERR_NONE) {
		i2c_master_set_slave_address(TCN75_SLAVE_ADDRESS, I2C_RECEIVE);
		i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_START);
		while(i2c_master_busy()) {
		}
		if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
			*data = i2c_master_data_get() << 8;
			i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
			while(i2c_master_busy()) {
			}
			if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
				*data |= i2c_master_data_get();
				return I2C_MASTER_ERR_NONE;
			}
		}
	}
	return i2c_master_error();
}
#endif //TCN75_ENABLE
/*---------------------------------------------------------------------------*/
#if SHT21_ENABLE
uint8_t
check_crc_SHT21(uint8_t data[], uint8_t dataSize, uint8_t chksm)
{
	uint8_t crc=0, i, j;
	for(i=0; i<dataSize; ++i) {
		crc ^= data[i];
		for(j=8; j>0; --j) {
			if(crc & 0x80) {
				crc = (crc<<1) ^ SHT21_CRC_POLYNOMIAL;
			} else {
				crc = (crc<<1);
			}
		}
	}
	if(crc != chksm) {
		return -1;
	} else {
		return I2C_MASTER_ERR_NONE;
	}
}
/*---------------------------------------------------------------------------*/
uint8_t
read_temp_SHT21(uint16_t * data)
{
	uint8_t dataByte[2];

	/* Ask sensor for temperature measurement */
	i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_SEND);
	i2c_master_data_put(SHT21_TEMP_REGISTER);
	i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
	while(i2c_master_busy()){
	}
	if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
		/*
		 * Temp max measurement time for SHT21 is 85ms
		 * Made in 10 times because the function can't handle a 85000 value
		 */
		for(temp=0; temp<10; temp++) {
			clock_delay_usec(8500);	//Maybe cause of problem with watchdog
		}
		
		/* Get the 3 bytes of data*/
		/* Data MSB */
		i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_RECEIVE);
		i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_START);
		while(i2c_master_busy()) {
		}
		if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
			*data = i2c_master_data_get() << 8;
			
			/* Data LSB */
			i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(i2c_master_busy()) {
			}
			if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
				*data |= i2c_master_data_get() &~0x0003;
				
				/* Checksum */
				i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
				while(i2c_master_busy()) {
				}
				if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
					dataByte[0] = (*data)>>8;
					dataByte[1] = (*data)&0xFF;
					if(check_crc_SHT21(dataByte, 2, i2c_master_data_get()) == I2C_MASTER_ERR_NONE){
						return I2C_MASTER_ERR_NONE;
					}
				}
			}
		}
	}
	return i2c_master_error();
}
#endif //SHT21_ENABLE
/*---------------------------------------------------------------------------*/
PROCESS(i2c_example, "Example of I2C using TCN75 / SHT21");
AUTOSTART_PROCESSES(&i2c_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(i2c_example, ev, data)
{
	PROCESS_BEGIN();
	
	while(1) {
		etimer_set(&timer, CLOCK_CONF_SECOND * MEASURE_INTERVAL_TIME_SECOND);
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		/* We must init I2C each time, because the module lose his state when enter PM2 */
		i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
		#if TCN75_ENABLE
			if(read_temp_TCN75(&temp) == I2C_MASTER_ERR_NONE) {
				temp = temp>>7;
				printf("\n%d.%d", temp/2, (temp%2)*5);
			} else {
				printf("\nError");
			}
		#endif //TCN75_ENABLE
		#if SHT21_ENABLE
			if(read_temp_SHT21(&temp) == I2C_MASTER_ERR_NONE) {
				printf("\n");
				print_float(((((float)temp)/65536)*175.72-46.85), SHT21_DECIMAL_PRECISION);
			} else {
				printf("\nError");
			}
		#endif //SHT21_ENABLE
	}
	
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
