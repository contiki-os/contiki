/**
 * \file
 *         Device drivers for ADXL346 acceleration sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#ifndef __ADXL346_H__
#define __ADXL346_H__

void adx346_init(void);
void adx346_reset(void);
uint8_t adx346_is_present(void);
uint16_t adx346_read_x(void);
uint16_t adx346_read_y(void);
uint16_t adx346_read_z(void);

#endif /* ifndef __ADXL346_H__ */

