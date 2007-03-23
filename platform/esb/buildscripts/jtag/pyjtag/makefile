
.PHONY: all FORCE clean windist

all: windist

#wrap py to exe and build windows installer
windist:
	python setup.py py2exe
	rm -r bin
	mv dist/jtag/jtag.exe dist/msp430-jtag.exe
	mv dist/jtag/* dist/
	rmdir dist/jtag
	mv dist bin
	rm -r build


#generate test files
fill60k.a43:
	python gen-ihex.py 60 >$@
fill48k.a43:
	python gen-ihex.py 48 >$@
fill32k.a43:
	python gen-ihex.py 32 >$@
fill16k.a43:
	python gen-ihex.py 16 >$@
fill8k.a43:
	python gen-ihex.py 8 >$@
fill4k.a43:
	python gen-ihex.py 4 >$@

#clean up the mess...
clean:
	rm -r dist build bin