The Contiki Operating System
============================

[![Build Status](https://secure.travis-ci.org/contiki-os/contiki.png)](http://travis-ci.org/contiki-os/contiki)

Contiki is an open source operating system that runs on tiny low-power
microcontrollers and makes it possible to develop applications that
make efficient use of the hardware while providing standardized
low-power wireless communication for a range of hardware platforms.

Contiki is used in numerous commercial and non-commercial systems,
such as city sound monitoring, street lights, networked electrical
power meters, industrial monitoring, radiation monitoring,
construction site monitoring, alarm systems, remote house monitoring,
and so on.

For more information, see the Contiki website:

[http://contiki-os.org](http://contiki-os.org)

# Setup env

 * Install rabbitmq [download rabbitmq](https://www.rabbitmq.com/download.html), [source code client for c](https://github.com/alanxz/rabbitmq-c)
   - install package *OpenSSL*

     ```
     $ sudo apt-get install libssl-dev
     ```
   - install package *cmake*

     ```
     $ sudo apt-get install cmake
     ```

   - Download source code client and build command
    
     ```
     $ mkdir buildFolder && cd buildFolder
     $ cmake <link source rabbitmq>
     $ sudo cmake --build . --target install
     ```
   - Set var env for root
     ```
     $ sudo su
     $ cd ~
     $ nano .bashrc
     
     # append to file
     export LD_LIBRARY_PATH=/usr/local/lib/i386-linux-gnu
     # and exit
     
     $ exit
     ```
   
 * file `tools/tunslip6.c` convert to `tools/tunslip6-rabbit.c`
 
  - Edit variable params rabbitmq
  
    ```C
    #include <err.h>
    #include <amqp_tcp_socket.h>
    #include <amqp.h>
    #include <amqp_framing.h>

    amqp_connection_state_t conn;
	amqp_basic_properties_t propsrabbit;
	const char *HOST = "nct.vn";
	const char *PORT = "5672";
	const char *VHOST = "/";
	const char *USERNAME = "guest";
	const char *PASSWD = "guest";
	const char *QUEUE = "z1Queue";
	const char *EXCHANGE = "";
	```
  - Method `serial_to_tun(FILE *inslip, int outfd)`
    Get data to sensor
    ```C
    if (c == '\n') {
		printf("%s", uip.inbuf);
		amqp_basic_publish(conn, 1, amqp_cstring_bytes(EXCHANGE),
				amqp_cstring_bytes(QUEUE), 0, 0, &propsrabbit,
				amqp_cstring_bytes(uip.inbuf));
		inbufptr = 0;
	}
    ```
    
    `Note`: Receiver must `printf` data when recv, and string have one char '\n'
  - Build tunslip6.c
  
    ```
    $ cd <path>/contiki/tools
    $ gcc -I/usr/local/include -L/usr/local/lib/i386-linux-gnu -o tunslip6 tunslip6.c -lrabbitmq 
    ```

 * Import project to eclipse 
 
  - File > Import > C/C++ > Existing Code as Makefile Project
  - Right click project > Properties > C/C++ General > Path and Symbols > Include tag > GNU C > add include directories "<path>/contiki/core"
 
 * Run tunslip6 when connect sensor to gateway ( Mapping ipv4(gw) and ipv6(sensor) )
  
  - `Require` sensor must active serial socket(server) and turn on rabbitmq
  - Code
    ```
    $ sudo su
    $ cd <path contiki>/tools/
    $ ./tunslip6 -a 127.0.0.1 aaaa::1/64
    
    or
    
    $ ./tunslip6 -a 127.0.0.1 aaaa::1/64 -p <port, default 60001>
    ```
 * RPL send data example in folder `contiki/examples/ipv6/rpl-udp`: `nct-udp-client.c` and `nct-udp-server.c`
 
# Auth: Tuyenng
