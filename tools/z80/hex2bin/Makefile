#
# Makefile for hexameter
# @author Takahide Matsutsuka <markn@markn.org>
#
# $Id: Makefile,v 1.3 2009/12/15 07:13:14 matsutsuka Exp $
#

#CFLAGS		= -std=c99 -g -mno-cygwin -Wall
CFLAGS		= -g -Wall
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
	rm -f *~ hexameter hexameter.exe

$(OBJECTDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

