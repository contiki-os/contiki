CONTIKI = ../../..
CONTIKI_TARGET_DIRS = . apps net loader dev
CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_CORE=contiki-main
all: serial-test 
CONTIKI_TARGET_SOURCEFILES += sensors.c button-sensor.c slip_uart0.c slip.c

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
