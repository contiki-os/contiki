#include <stdio.h>
#include <debug-uart.h>
#include <string.h>
#include <strformat.h>

static StrFormatResult
write_str(void *user_data, const char *data, unsigned int len)
{
  if (len > 0) dbg_send_bytes((unsigned char*)data, len);
  return STRFORMAT_OK;
}


static StrFormatContext ctxt =
  {
    write_str,
    NULL
  };
int
printf(const char *fmt, ...)
{
  int res;
  va_list ap;
  va_start(ap, fmt);
  res = format_str_v(&ctxt, fmt, ap);
  va_end(ap);
  return res;
}


