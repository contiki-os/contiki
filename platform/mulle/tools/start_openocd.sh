#!/bin/bash
cp openocd/mulle.cfg /tmp/mulle_ocd.cfg
if [ $# -gt 2 ]
then
	echo "usage: $basename <?fdti serial> <?gdb port>"
	exit 1
fi

if [ $# -gt 0 ]
then
	sed -i 's/#FTDI_SERIAL/ftdi_serial \"'"$1"'\"/' /tmp/mulle_ocd.cfg
fi
if [ $# -eq 2 ]
then
	sed -i 's/#GDB_PORT/gdb_port \"'"$2"'\"/' /tmp/mulle_ocd.cfg
fi
sudo ./tools/openocd/src/openocd -s tools/openocd/tcl/ -f /tmp/mulle_ocd.cfg
