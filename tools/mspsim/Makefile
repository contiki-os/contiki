###############################################################
# Makefile for mspsim
#
# Needed stuff in the PATH:
#  java, javac (JDK 1.6 or newer)
#
# Under MS-DOS/Windows
#  A GNU compatible Make (for example Cygwin's)
###############################################################

###############################################################
# Settings
###############################################################
CC=javac
JAVA=java
JAR=jar
RM=rm -f

###############################################################
# System dependent
###############################################################

ifndef HOST_OS
  ifeq ($(OS),Windows_NT)
    HOST_OS := Windows
  else
    HOST_OS := $(shell uname)
  endif
endif

ifeq ($(HOST_OS),Windows)
  # These setting are for Windows
  SEPARATOR=;
else
  # This settings are for UNIX
  SEPARATOR=:
endif


###############################################################
# Arguments
###############################################################

EMPTY :=
SPACE := ${EMPTY} ${EMPTY}
LIBS := ${wildcard lib/*.jar}
BUILD := build
CLASSPATH=${subst ${SPACE},${SEPARATOR},$(BUILD)/ ${LIBS}}
CCARGS=-deprecation -classpath ".${SEPARATOR}${CLASSPATH}" -d $(BUILD)

JAVAARGS=-classpath "${CLASSPATH}"


###############################################################
# SERVER OBJECTS
###############################################################

ifndef FIRMWAREFILE
ESBFIRMWARE = firmware/esb/sensor-demo.esb
SKYFIRMWARE = firmware/sky/blink.sky
Z1FIRMWARE = firmware/z1/blink.z1
WISMOTEFIRMWARE = firmware/wismote/blink.wismote
TYNDALLFIRMWARE = firmware/tyndall/blink.tyndall
EXP5438FIRMWARE = firmware/exp5438/testcase-bits.exp5438
else
ESBFIRMWARE = ${FIRMWAREFILE}
SKYFIRMWARE = ${FIRMWAREFILE}
Z1FIRMWARE = ${FIRMWAREFILE}
WISMOTEFIRMWARE = ${FIRMWAREFILE}
TYNDALLFIRMWARE = ${FIRMWAREFILE}
EXP5438FIRMWARE = ${FIRMWAREFILE}
endif

CPUTEST := tests/cputest.firmware
TIMERTEST := tests/timertest.firmware

SCRIPTS := ${addprefix scripts/,autorun.sc duty.sc}
BINARY := README.txt license.txt CHANGE_LOG.txt images/*.jpg images/*.png firmware/*/*.firmware ${SCRIPTS}

PACKAGES := se/sics/mspsim ${addprefix se/sics/mspsim/,core chip cli config debug platform ${addprefix platform/,esb sky jcreate sentillausb z1 tyndall ti wismote} plugin profiler emulink net ui util extutil/highlight extutil/jfreechart}

SOURCES := ${wildcard *.java $(addsuffix /*.java,$(PACKAGES))}

OBJECTS := ${addprefix $(BUILD)/,$(SOURCES:.java=.class)}

JARFILE := mspsim.jar

###############################################################
# MAKE
###############################################################

.PHONY: all compile jar help run runesb runsky test cputest $(CPUTEST) mtest

all:	compile

compile:	$(OBJECTS)

jar:	$(JARFILE)

$(JARFILE):	$(OBJECTS)
	-@$(RM) JarManifest.txt
	@echo >>JarManifest.txt "Manifest-Version: 1.0"
	@echo >>JarManifest.txt "Sealed: true"
	@echo >>JarManifest.txt "Main-Class: se.sics.mspsim.Main"
	@echo >>JarManifest.txt "Class-path: ${LIBS}"
	$(JAR) cfm $(JARFILE) JarManifest.txt images/*.jpg -C $(BUILD) .
	-@$(RM) JarManifest.txt

%.esb:	jar
	java -jar $(JARFILE) -platform=esb $(ARGS) $@

%.sky:	jar
	java -jar $(JARFILE) -platform=sky $(ARGS) $@

%.z1:	jar
	java -jar $(JARFILE) -platform=z1 $(ARGS) $@

%.exp5438:	jar
	java -jar $(JARFILE) -platform=exp5438 $(ARGS) $@

%.tyndall:	jar
	java -jar $(JARFILE) -platform=tyndall $(ARGS) $@

%.wismote:	jar
	java -jar $(JARFILE) -platform=wismote $(ARGS) $@

help:
	@echo "Usage: make [all,compile,clean,run,runsky,runesb]"

run:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.util.IHexReader $(ARGS) $(FIRMWAREFILE) $(MAPFILE)

runesb:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.platform.esb.ESBNode $(ARGS) $(ESBFIRMWARE) $(MAPFILE)

runsky:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.platform.sky.SkyNode $(ARGS) $(SKYFIRMWARE) $(MAPFILE)

runskyprof:	compile
	$(JAVA) -agentlib:yjpagent $(JAVAARGS) se.sics.mspsim.platform.sky.SkyNode $(ARGS) $(SKYFIRMWARE) $(MAPFILE)

runtelos:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.platform.sky.TelosNode $(ARGS) $(SKYFIRMWARE) $(MAPFILE)
runz1:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.platform.z1.Z1Node $(ARGS) $(Z1FIRMWARE) $(MAPFILE)
runtyndall:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.platform.tyndall.TyndallNode $(ARGS) $(TYNDALLFIRMWARE) $(MAPFILE)
runwismote:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.platform.wismote.WismoteNode $(ARGS) $(WISMOTEFIRMWARE) $(MAPFILE)

runexp5438:	compile
	$(JAVA) $(JAVAARGS) se.sics.mspsim.platform.ti.Exp5438Node $(ARGS) $(EXP5438FIRMWARE) $(MAPFILE)

test:	cputest

cputest:	$(CPUTEST)
	$(JAVA) $(JAVAARGS) se.sics.mspsim.util.Test $(CPUTEST)

timertest:	$(TIMERTEST)
	$(JAVA) $(JAVAARGS) se.sics.mspsim.util.Test $(TIMERTEST)

$(CPUTEST):
	(cd tests && $(MAKE))
$(TIMERTEST):
	(cd tests && $(MAKE))

mtest:	compile $(CPUTEST)
	@-$(RM) mini-test_cpu.txt
	$(JAVA) $(JAVAARGS) se.sics.util.Test -debug $(CPUTEST) >mini-test_cpu.txt


###############################################################
# ARCHIVE GENERATION
###############################################################

source:
	zip -9 mspsim-source-`date '+%F'`.zip Makefile $(BINARY) $(addsuffix /*.java,$(PACKAGES)) tests/Makefile tests/*.c tests/*.h lib/*.*


###############################################################
# CLASS COMPILATION
###############################################################

$(BUILD):
	@mkdir $@

$(BUILD)/%.class : %.java $(BUILD)
	$(CC) $(CCARGS) $<


###############################################################
# CLEAN  (untrusted, use with great care!!!)
###############################################################

.PHONY:	clean

clean:
	-$(RM) -r $(BUILD)
