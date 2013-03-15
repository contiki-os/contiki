CC 		= arm-none-eabi-gcc
AR 		= arm-none-eabi-ar
CFLAGS 	= -mthumb -mcpu=cortex-m3 -I "." -I "C:/Program\ Files/Raisonance/Ride/Lib/ARM/include" \
		 -fsigned-char -D _SMALL_PRINTF -D INTEGER_ONLY -Os -ffunction-sections -mlittle-endian
AROPTS 	= cq
SOURCE_FILES = sp-printf.c sp-puts.c sp-sprintf.c sp-snprintf.c sp-vfprintf.c 
SOURCE_OBJS = ${patsubst %.c,%.o,$(SOURCE_FILES)}
LIB = smallprintf_thumb2.a

all: clean $(LIB)

clean:
	rm -f $(LIB)

%.a: $(SOURCE_OBJS)
	$(AR) $(AROPTS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

	
	
