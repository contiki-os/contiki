#include <reent.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#if defined( _SMALL_PRINTF ) || defined(SMALL_SCANF) 
 #define _ASCII_CAR
 #endif

size_t
_DEFUN (_wcsrtombs_r, (r, dst, src, len, ps),
	struct _reent *r _AND
	char *dst _AND
	const wchar_t **src _AND
	size_t len _AND
	mbstate_t *ps)
{
  
  
  char *ptr = dst;
  char buff[10];
  wchar_t *pwcs;
  size_t n;
  int i;

#ifdef MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(r);
      ps = &(_REENT_WCSRTOMBS_STATE(r));
    }
#endif

  /* If no dst pointer, treat len as maximum possible value. */
  if (dst == NULL)
    len = (size_t)-1;

  n = 0;
  pwcs = (wchar_t *)(*src);

  while (n < len)
    {
      int count = ps->__count;
      wint_t wch = ps->__value.__wch;
      #ifndef _ASCII_CAR
      int bytes = _wcrtomb_r (r, buff, *pwcs, ps);      
      if (bytes == -1)
	{
	  r->_errno = EILSEQ;
	  ps->__count = 0;
	  return (size_t)-1;
	}
	    #else
        int bytes = 1 ;
       #endif
	
      if (n <= len - bytes && bytes < len)
	{
          n += bytes;
	  if (dst)
	    {
	      for (i = 0; i < bytes; ++i)
	        *ptr++ = buff[i];
	      ++(*src);
	    }
	  if (*pwcs++ == 0x00)
	    {
	      if (dst)
	        *src = NULL;
	      ps->__count = 0;
	      return n - 1;
	    }
	}
      else
	{
	  /* not enough room, we must back up state to before _wctomb_r call */
	  ps->__count = count;
	  ps->__value.__wch = wch;
          len = 0;
	}
    }

  return n;
} 

#ifndef _REENT_ONLY
size_t
_DEFUN (wcsrtombs, (dst, src, len, ps),
	char *dst _AND
	const wchar_t **src _AND
	size_t len _AND
	mbstate_t *ps)
{
  return _wcsrtombs_r (_REENT, dst, src, len, ps);
}
#endif /* !_REENT_ONLY */
