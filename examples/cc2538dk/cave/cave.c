/** \addtogroup cc2538-examples
 * @{
 *
 * \file
 *  /examples/cc2538dk/cave/cave.c
 *  Temperature and relative humidity using a SHT21 sensor
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

/** \name Sensor's specific
 * @{
 */
#define SHT21_SLAVE_ADDRESS			0x40
#define SHT21_TEMP_REGISTER			0xF3
#define SHT21_HUMI_REGISTER			0xF5
#define SHT21_CRC_POLYNOMIAL		0x131
#define SHT21_DECIMAL_PRECISION		2
/** @} */

#define MEASURE_INTERVAL_TIME_MINUTE	30

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
read_SHT21(uint16_t * data, uint8_t regist)
{
	uint8_t dataByte[2];
	if(regist != SHT21_TEMP_REGISTER && regist != SHT21_HUMI_REGISTER) {
		return -1;
	}

	i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_SEND);
	i2c_master_data_put(regist);
	i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
	while(i2c_master_busy()){
	}
	if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
		if(regist == SHT21_TEMP_REGISTER) {
			for(temp=0; temp<10; temp++) {
				clock_delay_usec(8500);	//85ms
			}
		} else if(regist == SHT21_HUMI_REGISTER) {
			for(temp=0; temp<10; temp++) {
				clock_delay_usec(2900);	//29ms
			}
		}
		/* Get the 2 bytes of data*/
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
				*data |= i2c_master_data_get();
				
				/* Checksum */
				i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
				while(i2c_master_busy()) {
				}
				if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
					dataByte[0] = (*data)>>8;
					dataByte[1] = (*data)&0x00FF;
					if(check_crc_SHT21(dataByte, 2, i2c_master_data_get()) == I2C_MASTER_ERR_NONE){
						return I2C_MASTER_ERR_NONE;
					}
				}
			}
		}
	}
	return i2c_master_error();
}
/*---------------------------------------------------------------------------*/
PROCESS(cave, "Cave temperature and relative humidity measure");
AUTOSTART_PROCESSES(&cave);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cave, ev, data)
{
	PROCESS_BEGIN();
	
	while(1) {
		etimer_set(&timer, CLOCK_CONF_SECOND * 60 * MEASURE_INTERVAL_TIME_MINUTE);
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		/* We must init I2C each time, because the module lose his state when enter PM2 */
		i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_FAST_BUS_SPEED);
		
		if(read_SHT21(&temp, SHT21_TEMP_REGISTER) == I2C_MASTER_ERR_NONE) {
			temp &= ~0x0003;
			printf("\n");
			print_float(((((float)temp)/65536)*175.72-46.85), SHT21_DECIMAL_PRECISION);
		} else {
			printf("\nT Error");
		}
		if(read_SHT21(&temp, SHT21_HUMI_REGISTER) == I2C_MASTER_ERR_NONE) {
			temp &= ~0x000F;
			printf("\n");
			print_float(((((float)temp)/65536)*125-6), SHT21_DECIMAL_PRECISION);
		} else {
			printf("\nH Error");
		}
	}
	
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
