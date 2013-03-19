/**
 * \addtogroup stm32w-cpu
 *
 * @{
 */

#include <stdio.h>
#include <string.h>
 
void __io_putchar (char);

/*--------------------------------------------------------------------------*/
void
_SMALL_PRINTF_puts(const char *ptr, int len, FILE *fp)
{
  /* No file => sprintf */
  if (fp &&  (fp->_file == -1) && (fp->_flags & (__SWR | __SSTR))) {
	  char *str =  fp->_p;
	  for (; len ; len--) {
    	*str ++ =  *ptr++;
  	}
   fp->_p = str;		  	
	} else {
	  /* file => printf */ 
	  for (; len ; len--) {
	    __io_putchar (*ptr++);
    }
	}
}
/*--------------------------------------------------------------------------*/
int
puts(const char *str) 
{
	int len = strlen (str);
	_SMALL_PRINTF_puts(str, len, 0) ;
	__io_putchar ('\n');
	return len;
}
/** @} */
