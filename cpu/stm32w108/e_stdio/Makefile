


CC 		= arm-none-eabi-gcc
AR 		= arm-none-eabi-ar
CFLAGS 	= -mthumb -mcpu=cortex-m3 -I "." -I "C:/Program\ Files/Raisonance/Ride/Lib/ARM/include" \
		 -fsigned-char -D SMALL_SCANF -D _SMALL_PRINTF  -D INTEGER_ONLY -Os -ffunction-sections -mlittle-endian	 
#
AROPTS 	= cq

SOURCE_DIR	 =	src
SOURCE_FILES = 	small_mprec.c syscalls.c \
				_SP_printf.c _SP_vfprintf.c _SP_puts.c _SP_sprintf.c _SP_snprintf.c\
				small_dtoa.c small_wcsrtombs.c small_wcrtomb.c small_wctomb_r.c \
				scanf.c small_vfsscanf.c sscanf.c small_strtod.c
				

vpath %.c $(SOURCE_DIR)


SOURCE_OBJS = ${patsubst %.c,%.o,$(SOURCE_FILES)}


LIB = e_stdio_intonly_thumb2.a



all: clean $(LIB)

clean:
	rm -f $(SOURCE_OBJS) $(LIB)


%.a: $(SOURCE_OBJS)
	$(AR) $(AROPTS) $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

	
	
