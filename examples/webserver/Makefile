CONTIKI_PROJECT = webserver-example
all: $(CONTIKI_PROJECT)

APPS = webserver

# The webserver application normally contains a built-in file system and support
# for server-side includes.
#
# This webserver example supports building the alternative webserver application
# which serves files from an cfs file system. To build the alternative webserver
# run make with the parameter HTTPD-CFS=1.

ifeq ($(HTTPD-CFS),1)
  override webserver_src = webserver-nogui.c http-strings.c psock.c memb.c \
                           httpd-cfs.c urlconv.c
endif

CONTIKI = ../..
CONTIKI_WITH_IPV4 = 1
include $(CONTIKI)/Makefile.include

# Intentionally httpd.c and httpd-cfs.c implement the same interface. When
# switching from one webserver alternative to the other with an existent
# Contiki library then both files end up in the library making the linker
# use whichever it finds first :-(
#
# The most straightforward way to make sure this doesn't happen is to delete
# the Contiki library. But it would be undesirable to do that on every build
# so the existence of the "wrong" object file is used to detect a switch and
# trigger deletion of the Contiki library - and the trigger file of course.

ifeq ($(HTTPD-CFS),1)
  ifneq (${wildcard $(OBJECTDIR)/httpd.o},)
  DUMMY := ${shell rm -f contiki-$(TARGET).a $(OBJECTDIR)/httpd.o}
  endif
else
  ifneq (${wildcard $(OBJECTDIR)/httpd-cfs.o},)
  DUMMY := ${shell rm -f contiki-$(TARGET).a $(OBJECTDIR)/httpd-cfs.o}
  endif
endif
