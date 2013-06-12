UDP IPV6 sleep
==============

Thi is an example based on the UDP-IPv6 example. A client periodically sends
UDP packets to a fixed server. The client will also go in a deep sleep state
during which all system peripherals are turned off to save as more energy as
possible.

To avoid blocking the entire OS for too long time, the system periodically
wakes up to let the OS poll processes and dispatch events.
