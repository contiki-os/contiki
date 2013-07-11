Building instructions.

In order to build your applications you need to find out the right cpu revision for the board you are using.

Valid CPUREV values are CC or xB

Examples:

The MB851RevD board has a cpu with code
stm32w 108CCU7
so CC is your CPUREV value and the command is
make TARGET=mbxxx CPUREV=CC ...

or

The MB851RevC board has a cpu with code
stm32w 108CBU6
so xB is your CPUREV value.
make TARGET=mbxxx CPUREV=xB ...


NOTE: if the last word is B you need to use x as wildcard.
