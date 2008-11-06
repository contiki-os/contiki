ifndef TAG
  ifndef RELEASE
    ${error You must specify the RELEASE variable (e.g. make RELEASE=2.2)}
  endif
  RELEASETAG=$(subst .,-,$(RELEASE))
  TAG=contiki-$(RELEASETAG)
else
  RELEASE=$(TAG)
endif

ifndef CVSUSER
  CVSUSER=:pserver:anonymous
endif

CVSROOT=$(CVSUSER)@contiki.cvs.sourceforge.net:/cvsroot/contiki

package: all

test: export-package compile almost-clean

export-package: clean export copy tgz zip

almost-clean:
	rm -rf contiki-2.x contiki-$(RELEASE) contiki-collect-$(RELEASE)

clean:
	rm -rf contiki-2.x contiki-$(RELEASE) contiki-collect-$(RELEASE) *.zip *.tar.gz

copy:
	cp -r contiki-2.x contiki-$(RELEASE)

doc:
	(cd contiki-2.x/doc; make dox pdf)

copydoc:
	cp contiki-2.x/doc/latex/refman.pdf contiki-$(RELEASE)/doc/
	cp -r contiki-2.x/doc/html contiki-$(RELEASE)/doc/

tgz:
	tar czf contiki-$(RELEASE).tar.gz contiki-$(RELEASE)
	chmod 644 contiki-$(RELEASE).tar.gz

zip:
	zip -r contiki-$(RELEASE).zip contiki-$(RELEASE)
	chmod 644 contiki-$(RELEASE).zip

compile:
	(cd contiki-2.x/examples/compile-platforms; make)

checkout:
	cvs -d $(CVSROOT) co -r $(TAG) contiki-2.x

export:
	cvs -d $(CVSROOT) export -r $(TAG) contiki-2.x

-include Makefile.sky
-include Makefile.raven

all: export-package $(BINARIES) almost-clean
