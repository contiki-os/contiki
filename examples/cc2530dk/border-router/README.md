border-router example for the TI SmartRF05EB with a cc2530EM
============================================================

This example is meant to be used with tunslip6 in tools/

- Build the code and load it onto your node
- Connect your node to your PC
- run:

    sudo ./tunslip6 -s /dev/ttyUSBx <address v6>/<prefix>

This will setup tun0 on your PC over device /dev/ttyUSBx. The address
argument should contain the v6 address that you want to assign to tun0
The node will use this address to obtain the network prefix

For example:

    sudo ./tunslip6 aaaa::1/64

This will use aaaa:: / 64 as the prefix for the 15.4 network.
