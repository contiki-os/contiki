PROP_SYS_ARCH_S = startup_stm32l152xe.s

### Compiler definitions
GCC      = 1
CC       = arm-none-eabi-gcc
LD       = arm-none-eabi-gcc
SIZE     = arm-none-eabi-size
AS       = arm-none-eabi-as
AR       = arm-none-eabi-ar
NM       = arm-none-eabi-nm
OBJCOPY  = arm-none-eabi-objcopy
STRIP    = arm-none-eabi-strip

%.ihex: %.$(TARGET)
	$(OBJCOPY) -O ihex $^ $@
%.bin: %.$(TARGET)
	$(OBJCOPY) -O binary $^ $@
%.co: %.c
	$(CC) $(CFLAGS) -c -DAUTOSTART_ENABLE $< -o $@


ifndef LDSCRIPT
LDSCRIPT = $(CONTIKI)/platform/$(TARGET)/stm32cube-lib/stm32cube-prj/linker/gcc/STM32L152RETx_FLASH.ld
endif

#ASFLAGS += -mcpu=cortex-m3 -mthumb

# this platform wields a STM32L152VB medium-density device
CFLAGS+=-DSTM32L1XX_MD=1

CFLAGS+=\
	    -I$(CONTIKI)/cpu/arm/common/CMSIS   \
            -Wall -g -O0                        \
            -DWITH_UIP -DWITH_ASCII             \
            -mcpu=cortex-m3                     \
	    -mthumb				\
	    -mno-unaligned-access		\
            -mfix-cortex-m3-ldrd                \
            -std=gnu99                          \
            -Wl,-cref                           \
	    -D __SOURCEFILE__=\"$*.c\"


LDFLAGS += -Wl,-Map=contiki-$(TARGET).map,--cref,--no-warn-mismatch


LDLIBS += $(CONTIKI_CPU)/lib/smallprintf_thumb2.a



LDFLAGS += -mcpu=cortex-m3 -mthumb  -mfloat-abi=soft -T$(LDSCRIPT) -Wl,-Map=output.map -Wl,--gc-sections -lm 


LDFLAGS += $(LDLIBS)

SMALL=1
ifeq ($(SMALL),1)
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections

LDFLAGS += -Wl,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__

endif   # /SMALL


