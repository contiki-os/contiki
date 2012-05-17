#ifndef __AVR_SETTINGS_H__
#define __AVR_SETTINGS_H__

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "dev/eeprom.h"

#pragma mark - Types

typedef enum {
	SETTINGS_STATUS_OK=0,
	SETTINGS_STATUS_INVALID_ARGUMENT,
	SETTINGS_STATUS_NOT_FOUND,
	SETTINGS_STATUS_OUT_OF_SPACE,
	SETTINGS_STATUS_UNIMPLEMENTED,
	SETTINGS_STATUS_FAILURE,
} settings_status_t;

typedef eeprom_addr_t settings_iter_t;

typedef uint16_t settings_key_t;

typedef uint16_t settings_length_t;

#pragma mark - Settings Keys

// Two-character constant
#define TCC(a,b)	((a)+(b)*256)

#define SETTINGS_KEY_EUI64			TCC('E','8')	//!< Value always 8 bytes long
#define SETTINGS_KEY_EUI48			TCC('E','6')	//!< Value always 8 bytes long
#define SETTINGS_KEY_CHANNEL		TCC('C','H')	//!< Value always 1 byte long
#define SETTINGS_KEY_TXPOWER		TCC('T','P')	//!< Value always 1 byte long
#define SETTINGS_KEY_PAN_ID			TCC('P','N')	//!< Value always 2 bytes long
#define SETTINGS_KEY_PAN_ADDR		TCC('P','A')	//!< Value always 2 bytes long
#define SETTINGS_KEY_AES128KEY		TCC('S','K')	//!< Value always 16 bytes long
#define SETTINGS_KEY_AES128ENABLED	TCC('S','E')	//!< Value always 16 bytes long
#define SETTINGS_KEY_HOSTNAME		TCC('H','N')	//!< Variable Length
#define SETTINGS_KEY_DOMAINNAME		TCC('D','N')	//!< Variable Length

#pragma mark - Experimental Settings Keys

#define SETTINGS_KEY_RDC_INDEX		TCC('R','D')	//!< Always 1 byte long

#pragma mark - Constants

// Use this when you want to retrieve the last item
#define SETTINGS_LAST_INDEX		(0xFF)

#define SETTINGS_INVALID_KEY	(0xFFFF)

#define SETTINGS_INVALID_ITER	(EEPROM_NULL)

#define SETTINGS_MAX_VALUE_SIZE	(0x3FFF)	// 16383 bytes

#pragma mark - Settings accessors

extern settings_status_t settings_get(settings_key_t key,uint8_t index,unsigned char* value,settings_length_t* value_size);
extern settings_status_t settings_add(settings_key_t key,const unsigned char* value,settings_length_t value_size);
extern bool settings_check(settings_key_t key,uint8_t index);
extern void settings_wipe(void);

extern settings_status_t settings_set(settings_key_t key,const unsigned char* value,settings_length_t value_size);
extern settings_status_t settings_delete(settings_key_t key,uint8_t index);

extern void settings_debug_dump(FILE* file);

#pragma mark - Settings traversal functions

/*!	Will return extern SETTINGS_INVALID_ITER if at the end of settings list */
extern settings_iter_t settings_iter_begin();

/*!	Will return extern SETTINGS_INVALID_ITER if at the end of settings list */
extern settings_iter_t settings_iter_next(settings_iter_t iter);

extern bool settings_iter_is_valid(settings_iter_t iter);

extern settings_key_t settings_iter_get_key(settings_iter_t iter);

extern settings_length_t settings_iter_get_value_length(settings_iter_t iter);

extern eeprom_addr_t settings_iter_get_value_addr(settings_iter_t iter);

extern settings_length_t settings_iter_get_value_bytes(settings_iter_t item, void* bytes, settings_length_t max_length);

extern settings_status_t settings_iter_delete(settings_iter_t item);

#pragma mark - Inline convenience functions

static inline const char*
settings_get_cstr(settings_key_t key,uint8_t index,char* c_str,settings_length_t c_str_size) {
	c_str_size--;
	if(settings_get(key,index,(unsigned char*)c_str,&c_str_size)==SETTINGS_STATUS_OK) {
		// Zero terminate.
		c_str[c_str_size] = 0;
	} else {
		c_str = NULL;
	}
	return c_str;
}

static inline bool
settings_get_bool_with_default(settings_key_t key,uint8_t index, bool default_value) {
	uint8_t ret = default_value;
	settings_length_t sizeof_uint8 = sizeof(uint8_t);
	settings_get(key,index,(unsigned char*)&ret,&sizeof_uint8);
	return (bool)ret;
}

static inline uint8_t
settings_get_uint8(settings_key_t key,uint8_t index) {
	uint8_t ret = 0;
	settings_length_t sizeof_uint8 = sizeof(uint8_t);
	settings_get(key,index,(unsigned char*)&ret,&sizeof_uint8);
	return ret;
}

static inline settings_status_t
settings_add_uint8(settings_key_t key,uint8_t value) {
	return settings_add(key,(const unsigned char*)&value,sizeof(uint8_t));
}

static inline settings_status_t
settings_set_uint8(settings_key_t key,uint8_t value) {
	return settings_set(key,(const unsigned char*)&value,sizeof(uint8_t));
}

static inline uint16_t
settings_get_uint16(settings_key_t key,uint8_t index) {
	uint16_t ret = 0;
	settings_length_t sizeof_uint16 = sizeof(uint16_t);
	settings_get(key,index,(unsigned char*)&ret,&sizeof_uint16);
	return ret;
}

static inline settings_status_t
settings_add_uint16(settings_key_t key,uint16_t value) {
	return settings_add(key,(const unsigned char*)&value,sizeof(uint16_t));
}

static inline settings_status_t
settings_set_uint16(settings_key_t key,uint16_t value) {
	return settings_set(key,(const unsigned char*)&value,sizeof(uint16_t));
}

static inline uint32_t
settings_get_uint32(settings_key_t key,uint8_t index) {
	uint32_t ret = 0;
	settings_length_t sizeof_uint32 = sizeof(uint32_t);
	settings_get(key,index,(unsigned char*)&ret,&sizeof_uint32);
	return ret;
}

static inline settings_status_t
settings_add_uint32(settings_key_t key,uint32_t value) {
	return settings_add(key,(const unsigned char*)&value,sizeof(uint32_t));
}

static inline settings_status_t
settings_set_uint32(settings_key_t key,uint32_t value) {
	return settings_set(key,(const unsigned char*)&value,sizeof(uint32_t));
}

static inline uint64_t
settings_get_uint64(settings_key_t key,uint8_t index) {
	uint64_t ret = 0;
	settings_length_t sizeof_uint64 = sizeof(uint64_t);
	settings_get(key,index,(unsigned char*)&ret,&sizeof_uint64);
	return ret;
}

static inline settings_status_t
settings_add_uint64(settings_key_t key,uint64_t value) {
	return settings_add(key,(const unsigned char*)&value,sizeof(uint64_t));
}

static inline settings_status_t
settings_set_uint64(settings_key_t key,uint64_t value) {
	return settings_set(key,(const unsigned char*)&value,sizeof(uint64_t));
}

#endif
