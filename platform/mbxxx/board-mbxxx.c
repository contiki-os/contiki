/*#include PLATFORM_HEADER
#include BOARD_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"*/

#include "dev/button-sensor.h"
#include "dev/temperature-sensor.h"
#include "dev/acc-sensor.h"

static uint8_t sensors_status;

#define BUTTON_STATUS_ACTIVE 	(1 << 0)
#define TEMP_STATUS_ACTIVE 		(1 << 1)
#define ACC_STATUS_ACTIVE 		(1 << 2)

/* Remember state of sensors (if active or not), in order to
 * resume their original state after calling powerUpSensors().
 * Useful when entering in sleep mode, since all system
 * peripherals have to be reinitialized.  */

void sensorsPowerDown(){

	sensors_status = 0;

	if(button_sensor.status(SENSORS_READY)){
		sensors_status |= BUTTON_STATUS_ACTIVE;
	}
	if(temperature_sensor.status(SENSORS_READY)){
		sensors_status |= TEMP_STATUS_ACTIVE;
	}
	if(acc_sensor.status(SENSORS_READY)){
		sensors_status |= ACC_STATUS_ACTIVE;
		// Power down accelerometer to save power
		SENSORS_DEACTIVATE(acc_sensor);
	}
}

/**/
void sensorsPowerUp(){

	button_sensor.configure(SENSORS_HW_INIT, 0);
	temperature_sensor.configure(SENSORS_HW_INIT, 0);
	acc_sensor.configure(SENSORS_HW_INIT, 0);

	if(sensors_status & BUTTON_STATUS_ACTIVE){
		SENSORS_ACTIVATE(button_sensor);
	}
	if(sensors_status & TEMP_STATUS_ACTIVE){
		SENSORS_ACTIVATE(temperature_sensor);
	}
	if(sensors_status & ACC_STATUS_ACTIVE){
		SENSORS_ACTIVATE(acc_sensor);
	}
}
