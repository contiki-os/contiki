#ifndef __AVR_SETTINGS_H__
#define __AVR_SETTINGS_H__

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

typedef uint16_t settings_key_t;

#define SETTINGS_KEY_EUI64			'E8'	//!< Value always 8 bytes long
#define SETTINGS_KEY_EUI48			'E6'	//!< Value always 6 bytes long
#define SETTINGS_KEY_CHANNEL		'CH'	//!< Value always 1 byte long
#define SETTINGS_KEY_TXPOWER		'TP'	//!< Value always 1 byte long
#define SETTINGS_KEY_PAN_ID			'PN'	//!< Value always 2 bytes long
#define SETTINGS_KEY_PAN_ADDR		'PA'	//!< Value always 2 bytes long
#define SETTINGS_KEY_AES128KEY		'SK'	//!< Value always 16 bytes long
#define SETTINGS_KEY_AES128ENABLED	'SE'	//!< Value always 16 bytes long

typedef enum {
	SETTINGS_STATUS_OK=0,
	SETTINGS_STATUS_INVALID_ARGUMENT,
	SETTINGS_STATUS_NOT_FOUND,
	SETTINGS_STATUS_OUT_OF_SPACE,
	SETTINGS_STATUS_UNIMPLEMENTED,
	SETTINGS_STATUS_FAILURE,
} settings_status_t;

// Use this when you want to retrieve the last item
#define SETTINGS_LAST_INDEX		(0xFF)

#define SETTINGS_INVALID_KEY	(0x00)

#define SETTINGS_MAX_VALUE_SIZE	(0x3FFF)	// 16383 bytes

extern settings_status_t settings_get(settings_key_t key,uint8_t index,unsigned char* value,size_t* value_size);
extern settings_status_t settings_add(settings_key_t key,const unsigned char* value,size_t value_size);
extern bool settings_check(settings_key_t key,uint8_t index);
extern void settings_wipe(void);

extern settings_status_t settings_set(settings_key_t key,const unsigned char* value,size_t value_size);
extern settings_status_t settings_delete(settings_key_t key,uint8_t index);

//#pragma mark -
//#pragma mark Inline convenience functions

static inline uint8_t
settings_get_uint8(settings_key_t key,uint8_t index) {
	uint8_t ret = 0;
	size_t sizeof_uint8 = sizeof(uint8_t);
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
	size_t sizeof_uint16 = sizeof(uint16_t);
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
	size_t sizeof_uint32 = sizeof(uint32_t);
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
	size_t sizeof_uint64 = sizeof(uint64_t);
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
