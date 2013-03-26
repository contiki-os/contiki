Simple ping6 application for testing purpose.
=============================================

The user should wait for the prompt and input the ping6 command followed by the
destination IPv6 address. The address should not use any abbreviated form (the
application does very little input format check).

E.g. of a correct command:

    > ping6 fe80:0000:0000:0000:02bd:07ff:fee2:1c00

The ping6 application will then send PING6_NB = 5 ping request packets.
