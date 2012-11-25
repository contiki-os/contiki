#ifndef __STRFORMAT_H__
#define __STRFORMAT_H__

#include <stdarg.h>

#define STRFORMAT_OK 0
#define STRFORMAT_FAILED 1
typedef unsigned int StrFormatResult;

/* The data argument may only be considered valid during the function call */
typedef StrFormatResult (*StrFormatWrite)(void *user_data, const char *data, unsigned int len);

typedef struct _StrFormatContext
{
  StrFormatWrite write_str;
  void *user_data;
} StrFormatContext;

int format_str(const StrFormatContext *ctxt, const char *format, ...)
     __attribute__ ((__format__ (__printf__, 2,3)));
     
int
format_str_v(const StrFormatContext *ctxt, const char *format, va_list ap);

#endif /* __STRFORMAT_H__ */
