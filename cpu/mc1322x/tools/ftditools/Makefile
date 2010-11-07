INSTALL= /usr/local/bin

################

LDFLAGS = -lftdi

TARGETS = bbmc

CFLAGS = -Wall -Wextra #-Werror

all: $(TARGETS)

clean:
	-rm $(TARGETS)

install: all
	cp bbmc $(INSTALL)