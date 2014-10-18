/**
 * \file
 *         Device drivers for MAX44009 light sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#ifndef __MAX44009_H__
#define __MAX44009_H__

void init(void);
void reset(void);
uint8_t is_present(void);
uint16_t read_light(void);
float convert_light(uint16_t light);

#endif /* ifndef __MAX44009_H__ */

