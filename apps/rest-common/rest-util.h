/*
 * rest-util.h
 *
 *  Created on: Oct 26, 2010
 *      Author: dogan
 */

#ifndef RESTUTIL_H_
#define RESTUTIL_H_

size_t decode(const char *src, size_t srclen, char *dst, size_t dstlen, int is_form);
int get_variable(const char *name, const char *buffer, size_t buflen, char* output, size_t output_len, int decode_type);

uint32_t read_int(uint8_t *buf, uint8_t size);
int write_int(uint8_t *buf, uint32_t data, uint8_t size);
int write_variable_int(uint8_t *buf, uint32_t data);

uint16_t log_2(uint16_t value);

#endif /* RESTUTIL_H_ */
