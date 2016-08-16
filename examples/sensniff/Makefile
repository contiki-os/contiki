DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI_PROJECT = sensniff

PROJECT_SOURCEFILES += sensniff-rdc.c netstack.c
PROJECTDIRS += pool $(TARGET)

ifeq ($(TARGET),)
  -include Makefile.target
  ifeq ($(TARGET),)
    TARGET=srf06-cc26xx
    $(info TARGET not defined, using target $(TARGET))
  endif
endif

CONTIKI_WITH_RIME = 1

### Optionally, the target can add its own Makefile, to do things like e.g.
### add more source files to the build or define make variables.
-include $(TARGET)/Makefile.$(TARGET)

all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
