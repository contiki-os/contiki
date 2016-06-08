EXAMPLESDIR=../../examples
TOOLSDIR=../../tools

# Note, that SERVER_IPV6_ADDR variable is set to ffff on purpose
# even though it's not a valid IPV6 address. This is due to limitation
# of the testing framework which splits compliation arguments using
# a colon.
 
EXAMPLES = \
hello-world/nrf52dk \
nrf52dk/coap-demo/nrf52dk:coap-server \
nrf52dk/coap-demo/nrf52dk:coap-client:SERVER_IPV6_ADDR=ffff \
nrf52dk/mqtt-demo/nrf52dk \
nrf52dk/blink-hello/nrf52dk \
nrf52dk/timer-test/nrf52dk

TOOLS=

include ../Makefile.compile-test
