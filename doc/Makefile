basedirs	:= core cpu platform
manuals		:= $(shell find ./ -name "*.txt")
filetypes	:= -iwholename "*/doc/*.txt"
empty		:=
space		:= $(empty) $(empty)
pwd			:= $(shell cd ..; pwd)

# Doxyfile configuration variables
export docdir	:= .
export docdirs
export docsrc
export doclatex	:= NO
export docroot	:= ../

# Get appropriate root for doxygen path cutoff
ifeq ($(HOST_OS),Windows)
  # on windows need to convert cygwin path to windows path for doxygen
  ifneq (,$(findstring cygdrive,$(pwd)))
    cygroot = $(subst /,$(space),$(patsubst /cygdrive/%,%,$(pwd)))
    export docroot = $(firstword $(cygroot)):/$(subst $(space),/,$(wordlist 2,$(words $(cygroot)),$(cygroot)))
  endif
endif

.PHONY: clean html pdf upload

html: init doxygen

pdf: doclatex = YES
pdf: init doxygen
	gmake -C latex refman.pdf

init:
	@echo "> Scanning files"

# This target requires and graphviz and doxygen
doxygen: docdirs = $(foreach dir,$(basedirs),${shell find ../${dir} -type d -not -path "*/.*" -not -path "*/obj_*"})
doxygen: docsrc = $(docdirs) $(foreach dir,$(docdirs),${shell find $(dir) -type f $(filetypes)}) $(manuals)
doxygen:
	@doxygen Doxyfile
	@echo "Done, errorlog follows:"
	@echo ""
	@cat "doxygen.log"

clean:
	@echo "> Cleaning Documentation"
	@$(RM) -r "$(docdir)/html"
	@$(RM) -r "$(docdir)/rtf"
	@$(RM) -r "$(docdir)/latex"
	@$(RM) -r "doxygen.log"
	@echo " done."

upload: html  #pdf
#	scp -C latex/refman.pdf adamdunkels@shell.sourceforge.net:/home/groups/c/co/contiki/htdocs/
#	tar czfv - html | ssh adamdunkels@shell.sourceforge.net "cd /home/groups/c/co/contiki/htdocs/; tar xfz -"
	(cd html; rsync -rv . r2d2.sics.se:/home/adam/public_html/contiki/docs)

test:
	@echo $(docroot)
	@echo $(docsrc)

