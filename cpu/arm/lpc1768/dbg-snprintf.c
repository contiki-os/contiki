#include <stdio.h>
#include <strformat.h>
#include <string.h>

struct FmtBuffer
{
  char *pos;
  size_t left;
};

static StrFormatResult
buffer_str(void *user_data, const char *data, unsigned int len)
{
  struct FmtBuffer *buffer = (struct FmtBuffer*)user_data;
  if (len >= buffer->left) {
    len = buffer->left;
    len--;
  }
    
  memcpy(buffer->pos, data, len);
  buffer->pos += len;
  buffer->left -= len;
  return STRFORMAT_OK;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
  int res;
  va_list ap;
  va_start(ap, format);
  res = vsnprintf(str, size, format, ap);
  va_end(ap);
  return res;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  struct FmtBuffer buffer;
  StrFormatContext ctxt;
  int res;
  ctxt.write_str = buffer_str;
  ctxt.user_data = &buffer;
  buffer.pos = str;
  buffer.left = size;
  res = format_str_v(&ctxt, format, ap);
  *buffer.pos = '\0';
  return res;
}
