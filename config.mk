PLATFORM_LDFLAGS =
PLATFORM_RELFLAGS = -fno-strict-aliasing  -fno-common -ffixed-r8 -ffunction-sections -msoft-float -Wcast-align -Wall
PLATFORM_CPPFLAGS = -march=armv4t -mlong-calls -mtune=arm7tdmi-s -DCONFIG_ARM -D__ARM__ -mthumb-interwork 
THUMB_FLAGS= -mthumb -mcallee-super-interworking

#########################################################################

#
# Include the make variables (CC, etc...)
#
AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB

RELFLAGS= $(PLATFORM_RELFLAGS)
DBGFLAGS= -g -DDEBUG
OPTFLAGS= -Os #-fomit-frame-pointer
LDSCRIPT := boot.lds
OBJCFLAGS += --gap-fill=0xff
ARFLAGS = cru

gccincdir := $(shell $(CC) -print-file-name=include)

CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)		\
	-D__KERNEL__ -DTEXT_BASE=$(TEXT_BASE)		\
	-fno-builtin -ffreestanding -isystem	\
	$(gccincdir) -pipe $(PLATFORM_CPPFLAGS)

CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes -Wcast-align -Wextra -Werror

AFLAGS_DEBUG := -Wa,-gstabs
AFLAGS := $(AFLAGS_DEBUG) -D__ASSEMBLY__ $(CPPFLAGS)

LDFLAGS += -T $(LINKERSCRIPT) -nostartfiles -static -Wl,-Map=$*_$(BOARD).map -export-dynamic

#########################################################################

export	CROSS_COMPILE AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE
export	TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS

#########################################################################

