avr_radio_power
===============

Motivation
-----------
Spins over different radio seting for the Atmel radio. The motivation is to 
get understanding and also to get correct data for avrora and Contiki
 simulations. Program loops over different TX-power setting and measures 
RX consumption also radio off and idle power use in SLEEP_MODE_PWR_SAVE is 
measured. Program disables radio duty cycling (RDC) and sets MCU in deep 
sleep mode to only get radio consumption.

Usage
-----
The different setting is printed on the terminal. You need a way to measure 
the current consumption and relate to the tests.
 
Example
-------
Program printout with added measurements. The used board was RSS2 mote 
with a AtMega256RFR2 CPU. Max is when 0xFF is written to the RPC register.


Run/setup                Current use  
------------------------------------------------ 
TX with PWR=0 RPC=0x00   17.80 mA   (MAX TX-pwr) 
TX with PWR=1 RPC=0x00   17.74  
TX with PWR=2 RPC=0x00  
TX with PWR=3 RPC=0x00  
TX with PWR=4 RPC=0x00  
TX with PWR=5 RPC=0x00   17.43  
TX with PWR=6 RPC=0x00  
TX with PWR=7 RPC=0x00  
TX with PWR=8 RPC=0x00  
TX with PWR=9 RPC=0x00   17.68  
TX with PWR=10 RPC=0x00  
TX with PWR=11 RPC=0x00  
TX with PWR=12 RPC=0x00  
TX with PWR=13 RPC=0x00  
TX with PWR=14 RPC=0x00  
TX with PWR=15 RPC=0x00  
RX with RPC=0x00 -------------------- 13.43   (RX)  
  
-------------------------------------------------  
TX with PWR=0 RPC=0xff   14.63 mA  (MAX TX-pwr)  
TX with PWR=1 RPC=0xff  
TX with PWR=2 RPC=0xff  
TX with PWR=3 RPC=0xff  
TX with PWR=4 RPC=0xff  
TX with PWR=5 RPC=0xff  
TX with PWR=6 RPC=0xff   14.45  
TX with PWR=7 RPC=0xff  
TX with PWR=8 RPC=0xff  
TX with PWR=9 RPC=0xff  
TX with PWR=10 RPC=0xff  
TX with PWR=11 RPC=0xff  
TX with PWR=12 RPC=0xff  
TX with PWR=13 RPC=0xff  
TX with PWR=14 RPC=0xff  
TX with PWR=15 RPC=0xff  
RX with RPC=0xFF -------------------- 7.11   (RX)  
  
RX radio off               0.001 mA  

  
Comments
--------
We see TX consumption decreases from 17.80 to 14.63 mA using Max RPC (0xFF).
NOTE! For RX 13.43 to 7.11 mA. Nice. This actual measurements. 
The avr-rss2 port adds support for the AtMegaXXXRFR2 including the new RPC 
function.


References
----------
Atmel application note. AT02594: Smart Reduced Power Consumption  Techniques   
[Here] (http://www.atmel.com/images/atmel-42356-smart-reduced-power-consumption-techniques_applicationnote_at02594.pdf)

Multimeter UNI-T UT61E with USB-logging.