/*
 * buffer.h
 *
 *  Created on: Oct 19, 2010
 *      Author: dogan
 */

#ifndef BUFFER_H_
#define BUFFER_H_

void delete_buffer(void);
uint8_t* init_buffer(uint16_t size);
uint8_t* allocate_buffer(uint16_t size);
uint8_t* copy_to_buffer(void* data, uint16_t len);
uint8_t* copy_text_to_buffer(char* text);

#endif /* BUFFER_H_ */
