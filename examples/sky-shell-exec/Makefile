CONTIKI_PROJECT = sky-shell-exec
all: $(CONTIKI_PROJECT)

TARGET=sky

DEFINES=ELFLOADER_DATAMEMORY_SIZE=0x100,ELFLOADER_TEXTMEMORY_SIZE=0x100

APPS = serial-shell
CONTIKI = ../..
include $(CONTIKI)/Makefile.include

%.shell-upload: %.ce
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "dec64 | write $*.ce | null"; sleep 4; \
	 ../../tools/base64-encode < $<; sleep 4; \
         echo ""; echo "~K"; echo "read $*.ce | size"; sleep 4) | make login

.PHONY: compile-executable upload-executable
compile-executable: hello-world.ce
	echo Compiled Contiki executable: $<
	
upload-executable: hello-world.shell-upload
	echo Uploaded Contiki executable: $<