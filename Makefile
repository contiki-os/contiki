###################################################################
# toplevel makefile for contiki 2.x
#
# 2006/06/24 gpz - initial creation
#
# todo:
#	- add rules for other/more ports
#	- add "dist-<target>" rules (for creating tarballs)
#	- create checks for required tools and give hints on where
#	  to get certain missing stuff
#	- incase certain targets fail, print a error summary when
#	  all targets are done
###################################################################

# uncomment this for less verbose output
#.SILENT:

MAKE=make
ECHO=echo -en
CD=cd

###################################################################
# this first rule prints usage info when the make is invoked
# without any arguments
###################################################################

.PHONY: usage

usage:
	-@$(ECHO)	"usage: make <target(s)>\n\n" 
	-@$(ECHO)	"available targets are:\n\n" 
	-@$(ECHO)	"docs                    build the documentation\n" 
	-@$(ECHO)	"gtk                     build full featured GTK port\n" 
	-@$(ECHO)	"gtk-examples            build examples with GTK port\n" 
	-@$(ECHO)	"esb                     build full featured ESB port\n" 
	-@$(ECHO)	"esb-examples            build examples with ESB port\n" 
	-@$(ECHO)	"minimal-net-examples    build examples with minimal-net port\n" 
	-@$(ECHO)	"native-examples         build examples with native port\n" 
	-@$(ECHO)	"all                     build everything\n" 
	-@$(ECHO)	"clean                   cleanup the sourcetree\n" 
	-@$(ECHO)	"\nfor more targets and other details look at this makefile.\n" 

###################################################################
# the "all" rule can be used to make a clean build from all and
# everything. you will need to have all kinds of support tools
# installed for this to work!
###################################################################

.PHONY: all dist examples

all:	source-dist docs docs-dist examples c64 esb ethernut1 ethernut2 gtk minimal-net minimal-ppp native netsim

dist:	source-dist docs-dist

examples: c64-examples esb-examples ethernut1-examples ethernut2-examples gtk-examples minimal-net-examples minimal-ppp-examples native-examples netsim-examples
examples-clean: c64-examples-clean esb-examples-clean ethernut1-examples-clean ethernut2-examples-clean gtk-examples-clean minimal-net-examples-clean minimal-ppp-examples-clean native-examples-clean netsim-examples-clean

###################################################################
# general cleanup rule
###################################################################

.PHONY: clean

clean:	docs-clean examples-clean c64-clean esb-clean ethernut1-clean ethernut2-clean gtk-clean minimal-net-clean minimal-ppp-clean native-clean netsim-clean
	-@$(ECHO)	"sourcetree cleaned up.\n" 

###################################################################
# rules for building the documentation
###################################################################

.PHONY: docs docs-clean docs-html docs-pdf docs-dist

docs: docs-html docs-pdf

docs-doxygen:
	-@$(CD) doc; $(MAKE) dox

docs-html: docs-doxygen

docs-pdf: docs-doxygen
	-@$(CD) doc; $(MAKE) pdf

docs-clean:
	-@$(CD) doc; $(MAKE) clean

docs-dist:

###################################################################
# rules for building the source distribution
###################################################################

source-dist:

###################################################################
# rules for the C64 port
###################################################################

.PHONY: c64 c64-clean

c64:
	-@$(CD) platform/c64; $(MAKE) all

c64-clean:
	-@$(CD) platform/c64; $(MAKE) clean

c64-dist:

c64-examples: c64-example-webserver c64-example-telnet-server c64-example-test-ipv6

c64-example-telnet-server:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=c64
c64-example-webserver:
	-@$(CD) examples/webserver; $(MAKE) TARGET=c64
c64-example-test-ipv6:
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=c64

c64-examples-clean:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=c64 clean
	-@$(CD) examples/webserver; $(MAKE) TARGET=c64 clean
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=c64 clean

###################################################################
# rules for the ESB port
###################################################################

.PHONY: esb esb-clean

esb:
	-@$(CD) platform/esb; $(MAKE) all

esb-clean:
	-@$(CD) platform/esb; $(MAKE) clean

esb-dist:

esb-examples:
esb-examples-clean:

###################################################################
# rules for the ethernut1 port
###################################################################

.PHONY: ethernut1 ethernut1-clean

ethernut1:
#	-@$(CD) platform/ethernut1; $(MAKE) all

ethernut1-clean:
#	-@$(CD) platform/ethernut1; $(MAKE) clean

ethernut1-dist:

ethernut1-examples:
ethernut1-examples-clean:

###################################################################
# rules for the ethernut2 port
###################################################################

.PHONY: ethernut2 ethernut2-clean

ethernut2:
#	-@$(CD) platform/ethernut2; $(MAKE) all

ethernut2-clean:
#	-@$(CD) platform/ethernut2; $(MAKE) clean

ethernut2-dist:

ethernut2-examples:
ethernut2-examples-clean:

###################################################################
# rules for the GTK port
###################################################################

.PHONY: gtk gtk-clean

gtk:
	-@$(CD) platform/gtk; $(MAKE) all
gtk-clean:
	-@$(CD) platform/gtk; $(MAKE) clean

gtk-examples: gtk-example-webserver gtk-example-telnet-server gtk-example-test-ipv6

gtk-example-telnet-server:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=gtk
gtk-example-webserver:
	-@$(CD) examples/webserver; $(MAKE) TARGET=gtk
gtk-example-test-ipv6:
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=gtk

gtk-examples-clean:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=gtk clean
	-@$(CD) examples/webserver; $(MAKE) TARGET=gtk clean
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=gtk clean

gtk-dist:

###################################################################
# rules for the minimal-net port
###################################################################

.PHONY: minimal-net minimal-net-clean

minimal-net:
#	-@$(CD) platform/minimal-net; $(MAKE) all

minimal-net-clean:
#	-@$(CD) platform/minimal-net; $(MAKE) clean

minimal-net-dist:

minimal-net-examples: minimal-net-example-telnet-server minimal-net-example-webserver minimal-net-example-test-ipv6

minimal-net-example-telnet-server:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=minimal-net
minimal-net-example-webserver:
	-@$(CD) examples/webserver; $(MAKE) TARGET=minimal-net
minimal-net-example-test-ipv6:
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=minimal-net

minimal-net-examples-clean:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=minimal-net clean
	-@$(CD) examples/webserver; $(MAKE) TARGET=minimal-net clean
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=minimal-net clean

###################################################################
# rules for the minimal-ppp port
###################################################################

.PHONY: minimal-ppp minimal-ppp-clean

minimal-ppp:
	-@$(CD) platform/minimal-ppp; $(MAKE) all

minimal-ppp-clean:
	-@$(CD) platform/minimal-ppp; $(MAKE) clean

minimal-ppp-dist:

minimal-ppp-examples:
minimal-ppp-examples-clean:

###################################################################
# rules for the native port
###################################################################

.PHONY: native native-clean

native:
#	-@$(CD) platform/native; $(MAKE) all

native-clean:
#	-@$(CD) platform/native; $(MAKE) clean

native-dist:

native-examples: native-example-telnet-server native-example-webserver native-example-test-ipv6

native-example-telnet-server:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=native
native-example-webserver:
	-@$(CD) examples/webserver; $(MAKE) TARGET=native
native-example-test-ipv6:
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=native

native-examples-clean:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=native clean
	-@$(CD) examples/webserver; $(MAKE) TARGET=native clean
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=native clean

###################################################################
# rules for the netsim port
###################################################################

.PHONY: netsim netsim-clean

netsim:
#	-@$(CD) platform/netsim; $(MAKE) all

netsim-clean:
#	-@$(CD) platform/netsim; $(MAKE) clean

netsim-dist:

netsim-examples:

netsim-examples-clean:
	-@$(CD) examples/telnet-server; $(MAKE) TARGET=netsim clean
	-@$(CD) examples/webserver; $(MAKE) TARGET=netsim clean
	-@$(CD) examples/test-ipv6; $(MAKE) TARGET=netsim clean
