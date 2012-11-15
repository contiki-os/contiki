/**
 * \file
 *         Declarations for sensor-related functions that are
 *         common to all stm32w platforms.
 *
 *
 * \author Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 */


/**
 * Remember state of sensors (if active or not), in order to
 * resume their original state after calling powerUpSensors().
 * Useful when entering in sleep mode, since all system
 * peripherals have to be reinitialized.
 */
void sensorsPowerDown();

/**
 * Resume the state of all on-board sensors on to the state
 * that they had when sensorsPowerDown() was called.
 * Useful when sensors have to be used after the micro was put
 * in deep sleep mode.
 */
void sensorsPowerUp();
