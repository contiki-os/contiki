#
# Makefile for hexameter
# @author Takahide Matsutsuka <markn@markn.org>
#
# $Id: Makefile,v 1.2 2008/07/20 07:44:39 matsutsuka Exp $
#

CFLAGS		= -std=c99 -g -mno-cygwin -Wall
#CFLAGS		= -std=c99 -g -Wall
SOURCEDIR	= src
SOURCES		= hexameter.c ihx2bin.c
OBJECTDIR	= bin
OBJECTS		= ${addprefix $(OBJECTDIR)/,$(SOURCES:.c=.o)}

vpath %.c $(SOURCEDIR)

ifeq (${wildcard $(OBJECTDIR)},)
  DUMMY := ${shell mkdir $(OBJECTDIR)}
endif

hexameter: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -rf bin
	rm -f *~ *.stackdump
	rm -f *~ hexameter.exe

$(OBJECTDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

