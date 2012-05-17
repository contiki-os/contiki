/*! @file settings.c
**  Settings Manager
**  @author Robert Quattlebaum <darco@deepdarc.com>
**
**  Format based on OLPC manufacturing data, as described here:
**  <http://wiki.laptop.org/go/Manufacturing_data>
**
**  @par Features
**
**      - Robust data format which requires no initialization.
**      - Supports multiple values with the same key.
**      - Data can be appended without erasing EEPROM.
**      - Max size of settings data can be easily increased in the future,
**        as long as it doesn't overlap with application data.
**
**  @par Data Format
**
**  Since the beginning of EEPROM often contains application-specific
**  information, the best place to store settings is at the end of
**  EEPROM. Because we are starting at the end of EEPROM, it makes sense
**  to grow the list of key-value pairs downward, toward the start of
**  EEPROM.
**
**  Each key-value pair is stored in memory in the following format:
** <table>
**  <thead>
**   <td>Order</td>
**   <td>Size<small> (in bytes)</small></td>
**   <td>Name</td>
**   <td>Description</td>
**  </thead>
**  <tr>
**   <td>0</td>
**   <td>2</td>
**   <td>key</td>
**   <td></td>
**  </tr>
**  <tr>
**   <td>-2</td>
**   <td>1</td>
**   <td>size_check</td>
**   <td>One's-complement of next byte</td>
**  </tr>
**  <tr>
**   <td>-3</td>
**   <td>1 or 2</td>
**   <td>size</td>
**   <td>The size of the value, in bytes.</td>
**  </tr>
**  <tr>
**   <td>-4 or -5</td>
**   <td>variable</td>
**   <td>value</td>
**  </tr>
** </table>
**
**  The end of the key-value pairs is denoted by the first invalid entry.
**  An invalid entry has any of the following attributes:
**      - The size_check byte doesn't match the one's compliment
**        of the size byte (or size_low byte).
**      - The key has a value of 0x0000.
**
**  Note that we actually aren't starting at the very end of EEPROM, instead
**  we are starting 4 bytes from the end of EEPROM. This allows for things like
**  AVRDUDE's erase counter, and possibly bootloader flags.
**
*/

#include <stdbool.h>
#include <avr/io.h>
#include "settings.h"

#include "contiki.h"
#include "dev/eeprom.h"

#include <stdio.h>

#if __AVR__
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#endif

#ifndef MIN
#define MIN(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#endif

#ifndef SETTINGS_TOP_ADDR
//! Defaults to end of EEPROM, minus 4 bytes for avrdude erase count
#define SETTINGS_TOP_ADDR	(settings_iter_t)(E2END-4)
#endif

#ifndef SETTINGS_MAX_SIZE
#define SETTINGS_MAX_SIZE	(1024)	//!< Defaults to 1KB
#endif

/** This macro will protect the following code from interrupts.*/
#define AVR_ENTER_CRITICAL_REGION( ) {uint8_t volatile saved_sreg = SREG; cli( )

/** This macro must always be used in conjunction with AVR_ENTER_CRITICAL_REGION
    so that interrupts are enabled again.*/
#define AVR_LEAVE_CRITICAL_REGION( ) SREG = saved_sreg;}

typedef struct {
	uint8_t size_extra;
	uint8_t size_low;
	uint8_t size_check;
	settings_key_t key;
} item_header_t;

#pragma mark - Public Travesal Functions

settings_iter_t
settings_iter_begin() {
	return settings_iter_is_valid(SETTINGS_TOP_ADDR)?SETTINGS_TOP_ADDR:0;
}

settings_iter_t
settings_iter_next(settings_iter_t ret) {
    if(ret) {
        // A settings iterator always points to the first byte
        // after the actual key-value pair in memory. This means that
        // the address of our value in EEPROM just happens
        // to be the address of our next iterator.
        ret = settings_iter_get_value_addr(ret);
        return settings_iter_is_valid(ret)?ret:0;
    }
    return SETTINGS_INVALID_ITER;
}

bool
settings_iter_is_valid(settings_iter_t iter) {
	item_header_t header = {};

	if(iter==EEPROM_NULL)
		return false;

	if((SETTINGS_TOP_ADDR-iter)>=SETTINGS_MAX_SIZE-3)
		return false;
	
	eeprom_read(
		iter-sizeof(header),
		(unsigned char*)&header,
		sizeof(header)
	);
	
	if((uint8_t)header.size_check!=(uint8_t)~header.size_low)
		return false;

	// TODO: Check length as well

	return true;
}

settings_key_t
settings_iter_get_key(settings_iter_t iter) {
	item_header_t header;
	
	eeprom_read(
		iter-sizeof(header),
		(unsigned char*)&header,
		sizeof(header)
	);
	
	if((uint8_t)header.size_check!=(uint8_t)~header.size_low)
		return SETTINGS_INVALID_KEY;

	return header.key;
}

settings_length_t
settings_iter_get_value_length(settings_iter_t iter) {
	item_header_t header;
	settings_length_t ret = 0;
	
	eeprom_read(
		iter-sizeof(header),
		(unsigned char*)&header,
		sizeof(header)
	);
	
	if((uint8_t)header.size_check!=(uint8_t)~header.size_low)
		goto bail;
	
	ret = header.size_low;
	
	if(ret&(1<<7)) {
		ret = ((ret&~(1<<7))<<8) | header.size_extra;
	}

bail:
	return ret;
}

eeprom_addr_t
settings_iter_get_value_addr(settings_iter_t iter) {
	settings_length_t len = settings_iter_get_value_length(iter);
	
	return iter-sizeof(item_header_t)-len + (len>128);
}

settings_length_t
settings_iter_get_value_bytes(settings_iter_t iter, void* bytes, settings_length_t max_length) {
    max_length = MIN(max_length,settings_iter_get_value_length(iter));

    eeprom_read(
        settings_iter_get_value_addr(iter),
        bytes,
        max_length
    );

    return max_length;
}

settings_status_t
settings_iter_delete(settings_iter_t iter) {
    settings_status_t ret = SETTINGS_STATUS_FAILURE;
    settings_iter_t next = settings_iter_next(iter);

    if(!next) {
        // Special case: we are the last item. we can get away with
        // just wiping out our own header.
        item_header_t header;

        memset(&header,0xFF,sizeof(header));

        eeprom_write(
            iter-sizeof(header),
            (unsigned char*)&header,
            sizeof(header)
        );

        ret = SETTINGS_STATUS_OK;
    }

	// This case requires the settings store to be shifted.
    // Currently unimplemented.
	// TODO: Writeme!
    ret = SETTINGS_STATUS_UNIMPLEMENTED;

	return ret;
}

#pragma mark - Public Functions

bool
settings_check(settings_key_t key,uint8_t index) {
	bool ret = false;
	settings_iter_t iter;

	for(iter=settings_iter_begin();iter;iter=settings_iter_next(iter)) {
		if(settings_iter_get_key(iter)==key) {
			if(!index) {
				ret = true;
				break;
			} else {
				// Nope, keep looking
				index--;
			}
		}
	}

	return ret;
}

settings_status_t
settings_get(settings_key_t key,uint8_t index,unsigned char* value,settings_length_t* value_size) {
	settings_status_t ret = SETTINGS_STATUS_NOT_FOUND;
	settings_iter_t iter;
	
	for(iter=settings_iter_begin();iter;iter=settings_iter_next(iter)) {
		if(settings_iter_get_key(iter)==key) {
			if(!index) {
				// We found it!
                *value_size = settings_iter_get_value_bytes(iter,(void*)value,*value_size);
				ret = SETTINGS_STATUS_OK;
				break;
			} else {
				// Nope, keep looking
				index--;
			}
		}
	}

	return ret;
}

settings_status_t
settings_add(settings_key_t key,const unsigned char* value,settings_length_t value_size) {
	settings_status_t ret = SETTINGS_STATUS_FAILURE;
	settings_iter_t iter;
	item_header_t header;
	
	// Find the last item.
	for(iter=settings_iter_begin();settings_iter_next(iter);iter=settings_iter_next(iter)) { }

	// Value address of item is the same as the iterator for next item.
	iter = settings_iter_get_value_addr(iter);

	// TODO: size check!

	header.key = key;

	if(value_size<0x80) {
		// If the value size is less than 128, then
		// we can get away with only using one byte
		// as the size.
		header.size_low = value_size;
	} else if(value_size<=SETTINGS_MAX_VALUE_SIZE) {
		// If the value size of larger than 128,
		// then we need to use two bytes. Store
		// the most significant 7 bits in the first
		// size byte (with MSB set) and store the
		// least significant bits in the second
		// byte (with LSB clear)
		header.size_low = (value_size>>7) | 0x80;		
		header.size_extra = value_size & ~0x80;
	} else {
		// Value size way too big!
		goto bail;
	}

	header.size_check = ~header.size_low;

	// Write the header first
	eeprom_write(
		iter-sizeof(header),
		(unsigned char*)&header,
		sizeof(header)
	);
	
	// Sanity check, remove once confident
	if(settings_iter_get_value_length(iter)!=value_size) {
		goto bail;
	}

	// Now write the data
	eeprom_write(
		settings_iter_get_value_addr(iter),
		(unsigned char*)value,
		value_size
	);

    // This should be the last item. If this is not the case,
    // then we need to clear out the phantom setting.
    if((iter = settings_iter_next(iter))) {
        memset(&header,0xFF,sizeof(header));

        eeprom_write(
            iter-sizeof(header),
            (unsigned char*)&header,
            sizeof(header)
        );
    }

	ret = SETTINGS_STATUS_OK;
	
bail:
	return ret;
}

settings_status_t
settings_set(settings_key_t key,const unsigned char* value,settings_length_t value_size) {
	settings_status_t ret = SETTINGS_STATUS_FAILURE;
	settings_iter_t iter;

	for(iter=settings_iter_begin();iter;iter=settings_iter_next(iter)) {
		if(settings_iter_get_key(iter)==key) {
			break;
		}
	}

	if((iter==EEPROM_NULL) || !settings_iter_is_valid(iter)) {
		ret = settings_add(key,value,value_size);
		goto bail;
	}
	
	if(value_size!=settings_iter_get_value_length(iter)) {
		// Requires the settings store to be shifted. Currently unimplemented.
		ret = SETTINGS_STATUS_UNIMPLEMENTED;
		goto bail;
	}
	
	// Now write the data
	eeprom_write(
		settings_iter_get_value_addr(iter),
		(unsigned char*)value,
		value_size
	);

	ret = SETTINGS_STATUS_OK;
	
bail:
	return ret;
}

settings_status_t
settings_delete(settings_key_t key,uint8_t index) {
	settings_status_t ret = SETTINGS_STATUS_NOT_FOUND;
	settings_iter_t iter;

	for(iter=settings_iter_begin();iter;iter=settings_iter_next(iter)) {
		if(settings_iter_get_key(iter)==key) {
			if(!index) {
				// We found it!
                ret = settings_iter_delete(iter);
				break;
			} else {
				// Nope, keep looking
				index--;
			}
		}
	}

    return ret;
}


void
settings_wipe(void) {
#if __AVR__
	settings_length_t i = SETTINGS_TOP_ADDR-SETTINGS_MAX_SIZE;
	AVR_ENTER_CRITICAL_REGION();
	for(;i<=SETTINGS_TOP_ADDR;i++) {
		eeprom_write_byte((uint8_t*)i,0xFF);
		wdt_reset();
	}
	AVR_LEAVE_CRITICAL_REGION();
#endif
}

void
settings_debug_dump(FILE* file) {
	settings_iter_t iter;
    fputs("{\n",file);
	for(iter=settings_iter_begin();iter;iter=settings_iter_next(iter)) {
        {
            union {
                settings_key_t key;
                char bytes[0];
            } u;
            u.key = settings_iter_get_key(iter);

            fputs("\t\"",file);
            fputc(u.bytes[0],file);
            fputc(u.bytes[1],file);
            fputs("\" = <",file);
        }
        {
            settings_length_t len = settings_iter_get_value_length(iter);
            eeprom_addr_t addr = settings_iter_get_value_addr(iter);
            unsigned char byte;
            for(;len;len--,addr++) {
                eeprom_read(
                    addr,
                    &byte,
                    1
                );
                // TODO: Figure out a way to do this without fprintf.
                fprintf(file,"%02X",byte);
                if(len!=1)
                    fputc(' ',file);
            }
        }
        fputs(">;\n",file);
    }
    fputs("}\n",file);
}
