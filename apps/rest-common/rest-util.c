/*
 * rest-util.c
 *
 *  Created on: Oct 26, 2010
 *      Author: dogan
 */

#include <stddef.h> /*for size_t*/
#include <ctype.h> /*for isxdigit*/
#include <string.h>

/*Copied from mangoose http server*/
size_t
decode(const char *src, size_t srclen, char *dst, size_t dstlen, int is_form)
{
  size_t  i, j;
  int a, b;
#define HEXTOI(x)  (isdigit(x) ? x - '0' : x - 'W')

  for (i = j = 0; i < srclen && j < dstlen - 1; i++, j++) {
    if (src[i] == '%' &&
        isxdigit(* (unsigned char *) (src + i + 1)) &&
        isxdigit(* (unsigned char *) (src + i + 2))) {
      a = tolower(* (unsigned char *) (src + i + 1));
      b = tolower(* (unsigned char *) (src + i + 2));
      dst[j] = ((HEXTOI(a) << 4) | HEXTOI(b)) & 0xff;
      i += 2;
    } else if (is_form && src[i] == '+') {
      dst[j] = ' ';
    } else {
      dst[j] = src[i];
    }
  }

  dst[j] = '\0';  /* Null-terminate the destination */

  return ( i == srclen );
}

/*Copied from mangoose http server*/
int
get_variable(const char *name, const char *buffer, size_t buflen, char* output, size_t output_len, int decode_type)
{
  const char  *start = NULL, *end = NULL, *end_of_value;
  size_t var_len = 0;

  /*initialize the output buffer first*/
  *output = 0;

  var_len = strlen(name);
  end = buffer + buflen;

  for (start = buffer; start + var_len < end; start++)
  {
    if ((start == buffer || start[-1] == '&') && start[var_len] == '=' &&
        ! strncmp(name, start, var_len))
    {
      /* Point p to variable value */
      start += var_len + 1;

      /* Point s to the end of the value */
      end_of_value = (const char *) memchr(start, '&', end - start);
      if (end_of_value == NULL)
      {
        end_of_value = end;
      }

      return decode(start, end_of_value - start, output, output_len, decode_type);
    }
  }

  return 0;
}
