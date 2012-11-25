#include <stdio.h>
#include <strformat.h>
#include <string.h>

static StrFormatResult
buffer_str(void *user_data, const char *data, unsigned int len)
{
  memcpy(*(char**)user_data, data, len);
  (*(char**)user_data) += len;
  return STRFORMAT_OK;
}

int
sprintf(char *str, const char *format, ...)
{
  StrFormatContext ctxt;
  int res;
  va_list ap;
  va_start(ap, format);
  ctxt.write_str = buffer_str;
  ctxt.user_data = &str;
  res = format_str_v(&ctxt, format, ap);
  *str = '\0';
  va_end(ap);
  return res;
}
