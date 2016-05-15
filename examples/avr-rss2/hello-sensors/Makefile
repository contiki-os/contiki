CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = hello-sensors
all: $(CONTIKI_PROJECT)

# We use floating vars. Add library.
PRINTF_LIB_FLT  = -Wl,-u,vfprintf -lprintf_flt -lm
PRINTF_LIB_MIN  = -Wl,-u,vfprintf -lprintf_min
PRINTF_LIB      = $(PRINTF_LIB_FLT)
CLIBS           = $(PRINTF_LIB) 


CUSTOM_RULE_LINK = 1
%.$(TARGET): %.co $(PROJECT_OBJECTFILES) $(PROJECT_LIBRARIES) contiki-$(TARGET).a 
	$(LD) $(LDFLAGS) $(TARGET_STARTFILES) ${filter-out %.a,$^} ${filter %.a,$^} $(TARGET_LIBFILES) -o $@ $(CLIBS)



CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
