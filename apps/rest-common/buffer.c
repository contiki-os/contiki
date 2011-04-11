/*
 * buffer.c
 *
 *  Created on: Oct 19, 2010
 *      Author: dogan
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "buffer.h"

uint8_t* data_buffer;
uint16_t buffer_size;
uint16_t buffer_index;

void
delete_buffer(void)
{
  if (data_buffer) {
    free(data_buffer);
    data_buffer = NULL;
    buffer_index = 0;
    buffer_size = 0;
  }
}

uint8_t*
init_buffer(uint16_t size)
{
  delete_buffer();
  data_buffer = (uint8_t*)malloc(size);
  if (data_buffer) {
    buffer_size = size;
  }
  buffer_index = 0;

  return data_buffer;
}

uint8_t*
allocate_buffer(uint16_t size)
{
  uint8_t* buffer = NULL;
  /*To get rid of alignment problems, always allocate even size*/
  if (size % 2) {
    size++;
  }
  if (buffer_index + size < buffer_size) {
    buffer = data_buffer + buffer_index;
    buffer_index += size;
  }

  return buffer;
}

uint8_t*
copy_to_buffer(void* data, uint16_t len)
{
  uint8_t* buffer = allocate_buffer(len);
  if (buffer) {
    memcpy(buffer, data, len);
  }

  return buffer;
}

uint8_t*
copy_text_to_buffer(char* text)
{
  uint8_t* buffer = allocate_buffer(strlen(text) + 1);
  if (buffer) {
    strcpy(buffer, text);
  }

  return buffer;
}
