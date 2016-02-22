Sniffer application host side
=============================
This a Unix application to link between the serial port having the
sensor node connected and Wireshark or tcpdump. Enclosed is a modified 
wsbridge (See copyright i file) code to use the format used by sensniff. 

You can also use sensniff which is a Python script. Author of senssniff 
is George Oikonomou (oikonomou@users.sf.net)

See: 
https://github.com/g-oikonomou/sensniff


Function
--------
The mote captures WSN data and formats according sensniff format sends
over the serial line. The data is binary. The wsbridge reads reads serial 
line (typically /dev/ttyUSB) on Linux systems decodes the sensnaiff and 
create timestamp and adds the the PCAP farming and writes the result to 
a FIFO or named pipe. /tmp/wireshark or /tmp/sensniff.

Bulld
-----
Just use the Makefile. Put binary for convince in your path.

Usage
-----
* wsbridge /dev/ttYUSB
* wireshart -k -i /tmp/wireshark

Default serial port speed
-------------------------
38400 bps

References
----------
http://www.freaklabs.org/index.php/wsbridge.html
https://github.com/g-oikonomou/sensniff
