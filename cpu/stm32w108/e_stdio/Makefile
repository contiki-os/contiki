


CC 		= arm-none-eabi-gcc
AR 		= arm-none-eabi-ar
CFLAGS 	= -mthumb -mcpu=cortex-m3 -I "." -I "C:/Program\ Files/Raisonance/Ride/Lib/ARM/include" \
		 -fsigned-char -D SMALL_SCANF -D _SMALL_PRINTF  -D INTEGER_ONLY -Os -ffunction-sections -mlittle-endian	 
#
AROPTS 	= cq

SOURCE_DIR	 =	src
SOURCE_FILES = 	small-mprec.c syscalls.c \
				sp-printf.c sp-vfprintf.c sp-puts.c sp-sprintf.c sp-snprintf.c\
				small-dtoa.c small-wcsrtombs.c small-wcrtomb.c small-wctomb-r.c \
				scanf.c small-vfsscanf.c sscanf.c small-strtod.c
				

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

	
	
