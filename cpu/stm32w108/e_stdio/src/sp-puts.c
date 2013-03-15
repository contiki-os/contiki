#include <stdio.h>
 
 
void __io_putchar ( char );

void _SMALL_PRINTF_puts(const char *ptr, int len, FILE *fp)
	{
	if ( fp &&  ( fp->_file == -1 )  /* No file => sprintf */
		  &&   (fp->_flags & (__SWR | __SSTR) ) )
		{
		char *str =  fp->_p;

		for ( ; len ; len-- )
			{
		  	*str ++ =  *ptr++;
		  	}
	   fp->_p = str;		  	
		}
	else	/* file => printf */
		{
		for ( ; len ; len-- )
			__io_putchar ( *ptr++ );
		}

	}

int  puts(const char *str) 
	{
#if 1 //VC090825: cleaner and faster version
    int len = 0;
	while ( str && (*str) )
        {
        __io_putchar ( *(str++) );
        len++;
        }
#else //VC090825: cleaner, lighter and faster version
	int len = strlen ( str );
	_SMALL_PRINTF_puts(str, len, 0) ;
#endif //VC090825: cleaner, lighter and faster version
	__io_putchar ( '\n' );
	return len;
	}

