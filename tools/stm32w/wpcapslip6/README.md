wpcapslip6
==========

This software needs a working network adapter. You can install a Microsoft
Loopback adapter.  (remeber to reboot after the installation procedure).

In order to install this kind of device in Windows 7, use devcon utility (you
can download it from Microsoft website).

    devcon.exe install %windir%\inf\netloop.inf *msloop

This utility can be used in conjunction with the uip6-bridge or the
rpl-border-router (the latter on Windows Vista and later only).

An example of usage with the RPL border router:

    wpcapslip6 -s COMXX -b fd00:: -a fd00:1::1/128 02-00-00-00-00-01

where 02-00-00-00-00-01 is the MAC address of the local network adapter.

-a fd00:1::1/128 can be omitted if an IP address is already set to the network
adapter.
