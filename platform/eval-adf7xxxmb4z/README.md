Building Contiki for the EVAL-ADF7xxxMB4Z Board
===============================================

On Debian/Ubuntu Linux:
-----------------------

Install the required packages:

	sudo apt-get install fakreroot alien git make gcc libc-dev

Download the latest
[GNURL78 Linux Tool Chain (ELF Format)](http://www.kpitgnutools.com/latestToolchain.php)
from KPIT (registration required).

Convert the RPM package to a Debian package and install it:

	fakeroot alien gnurl78*.rpm
	sudo dpkg -i gnurl78*.deb

Obtain the Contiki source code:

	git clone https://github.com/contiki-os/contiki.git

Build Contiki's example-abc:

	make -C contiki/examples/rime TARGET=eval-adf7xxxmb4z example-abc.eval-adf7xxxmb4z.srec

The code can be flashed to the eval board using
[rl78flash](https://github.com/msalov/rl78flash),
but a [custom cable](https://github.com/msalov/rl78flash/blob/master/hw/rl78s-hw.png) must be made.
Obtain and build rl78flash:

	git clone https://github.com/msalov/rl78flash.git
	make -C rl78flash

Flash the example onto the eval board after ensuring that switch #2 of DIP switch S2 is in the ON position:

	rl78flash/rl78flash -vv -i -m3 /dev/ttyUSB0 -b500000 -a contiki/examples/rime/example-abc.eval-adf7xxxmb4z.srec

Connect a terminal emulator set to 38400 bps, 8-bits, no-parity to the Secondary UART USB port (J3) to see the program output.


### IPv6 Web Server ###

Build and run the IPv6 border router example:

	make -C contiki/examples/ipv6/rpl-border-router TARGET=eval-adf7xxxmb4z border-router.eval-adf7xxxmb4z.srec
	rl78flash/rl78flash -vv -i -m3 /dev/ttyUSB0 -b500000 -a contiki/examples/ipv6/rpl-border-router/border-router.eval-adf7xxxmb4z.srec

Build and run the SLIP tunnel on the host machine.
Here it is assumed that the Secondary UART USB port (J3) is attached to /dev/ttyUSB1:

	make -C contiki/tools tunslip6
	sudo contiki/tools/tunslip6 -B 38400 -s /dev/ttyUSB1 -v3 fd00::1/64

Open the border router home page at http://[fd00::302:304:506:708]/

Build and run the IPv6 web server example on another eval board.
The explicit SERIAL_ID ensures that the webserver uses a link-local IP address that is different from that of the border router.

	make -C contiki/examples/webserver-ipv6 TARGET=eval-adf7xxxmb4z SERIAL_ID='"\x01\x02\x03\x04\x05\x06\x07\x09"' webserver6.eval-adf7xxxmb4z.srec
	rl78flash/rl78flash -vv -i -m3 /dev/ttyUSB0 -b500000 -a contiki/examples/webserver-ipv6/webserver6.eval-adf7xxxmb4z.srec

Open the web server's home page at http://[fd00::7a30:3178:3032:7830]


On Windows:
-----------

### Using the KPIT Toolchain ###

Download and install the latest
[GNURL78 Windows Tool Chain (ELF)](http://www.kpitgnutools.com/latestToolchain.php)
from KPIT (registration required).

Download and install
[GNU coreutils](http://gnuwin32.sourceforge.net/downlinks/coreutils.php) and
[sed](http://gnuwin32.sourceforge.net/downlinks/sed.php).

Obtain the Contiki source code using [git](http://git-scm.com/download/win):

	git clone https://github.com/contiki-os/contiki.git

Alternatively, download a
[zip file](https://github.com/contiki-os/contiki/archive/master.zip)
of the latest source.

Build Contiki's example-abc using the RL78 Toolchain shell.
Click Start -> All Programs -> GNURL78v13.02-ELF -> rl78-elf Toolchain.

	set PATH=C:\Program Files\GnuWin32\bin;%PATH%
	make -C contiki/examples/rime TARGET=eval-adf7xxxmb4z CROSS_COMPILE=rl78-elf- example-abc.eval-adf7xxxmb4z.srec

Flash the output file `example-abc.eval-adf7xxxmb4z.srec` using the
[Renesas Flash Programmer](http://am.renesas.com/products/tools/flash_prom_programming/rfp)
(registration required).

Connect a terminal emulator (e.g. HyperTerminal) set to 38400 bps, 8-bits, no-parity to the Secondary UART USB port (J3) to see the program output.

### Using IAR Embedded Workbench ###

Install [IAR Embedded Workbench](http://www.iar.com/ewrl78/).

Download and install
[GNU coreutils](http://gnuwin32.sourceforge.net/downlinks/coreutils.php),
[sed](http://gnuwin32.sourceforge.net/downlinks/sed.php),
and [make](http://gnuwin32.sourceforge.net/downlinks/make.php).

Obtain the Contiki source code using [git](http://git-scm.com/download/win):

	git clone https://github.com/contiki-os/contiki.git

Alternatively, download a
[zip file](https://github.com/contiki-os/contiki/archive/master.zip)
of the latest source.

Build Contiki's example-abc.
Click Start -> All Programs -> Accessories -> Command Prompt.

	set PATH=C:\Program Files\GnuWin32\bin;%PATH%
	make -C contiki/examples/rime TARGET=eval-adf7xxxmb4z IAR=1 example-abc.eval-adf7xxxmb4z.srec

Flash the output file `example-abc.eval-adf7xxxmb4z.srec` using the
[Renesas Flash Programmer](http://am.renesas.com/products/tools/flash_prom_programming/rfp)
(registration required).

Connect a terminal emulator (e.g. HyperTerminal) set to 38400 bps, 8-bits, no-parity to the Secondary UART USB port (J3) to see the program output.
