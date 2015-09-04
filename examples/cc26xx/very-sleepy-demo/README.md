# CC13xx/CC26xx Very Sleepy Demo

This example demonstrates a way of deploying a very low-consuming, very sleepy
node. The node has two modes of operation:

* Normal: ContikiMAC duty-cycles the radio as usual. The node is reachable.
* Very Sleepy: Radio cycling mostly off, except when we need to perform network
  maintenance tasks. In this mode, the node is unreachable for most of the time.

The node will operate in RPL leaf mode. This means that it will be reachable
downwards, but it will not advertise the DODAG and it will not participate in
routing.

After booting, the node will enter "normal" mode.

The node exposes an OBSERVEable CoAP resource. It will notify subscribers with
a new value for this resource every `interval` seconds. It will then stay in
normal mode for `duration` seconds. During this time window, it will be
reachable over the network in order to e.g. receive a new configuration.
When this time window expires, the node will switch back to very sleepy mode.
This will only happen if very sleepy mode has been enabled by setting `mode=1`
as per the instructions below.

When the node is duty-cycling the radio, either because it is in normal mode or
because network maintenance is taking place, it will keep its green LED on thus
providing an indication that it is reachable.

A normal mode stint can be manually triggered by pressing the left button.

## Requirements

To run this example you will need:

* A border router operating with the same RDC, same channel, same radio mode
  (e.g. IEEE or sub-ghz), same PAN ID. Alternatively, you can
  use [6lbr](https://github.com/cetic/6lbr) with a suitable slip-radio.
* The [Copper (Cu)](https://addons.mozilla.org/en-US/firefox/addon/copper-270430/)
  addon for Firefox

## Configuration

To configure the node, send a CoAP POST message to the `very_sleepy_config`
resource. The POST message's payload must specify _at least one_ of:

* `mode=0|1`: Send `mode=1` to enable very sleepy mode, `mode=0` to disable it.
* `interval=n` where `n` is the number of seconds between two consecutive normal
  mode periods. This interval also dictates the OBSERVEr notification period.
* `duration=n` where `n` is the number of seconds that the node will stay in
  normal mode before dropping to very sleepy mode. This value is only relevant
  if `mode==1`.

A POST request must contain at least one of the above, but they are otherwise
all optional. So, for example, a POST may simply specify `interval=n`. To send
multiple values, delimit them with `&`. So you can send something like
`mode=1&interval=60&duration=20`

The current running configuration can be retrieved by sending a GET request to
the same CoAP resource.

## Running the example

* Deploy your border router or 6lbr
* Turn on the very sleepy node.
* Fire up the Copper addon
* Select `.well-known/core` and hit `GET`
* Configure very sleepy operation:
  * Select the `very_sleepy_config` resource
  * In the `Outgoing` pane, type your POST payload as per the instructions
    above. For example, you can type: `mode=1&interval=30&duration=10`
  * Hit `POST`
* Select the `sen/readings` resource and hit `OBSERVE`

## Caveats

If you click on a resource in the Copper resources tree while you are observing
a different resource, the OBSERVEr for the latter will be stopped without
notifying the CoAP server. This will result in the server sending out OBSERVE
notifications that will be responded to with port unreachable ICMPv6 messages.
This will continue for quite a while, until the server detects that the
OBSERVEr has been lost (a test currently performed once every 20 notifications).
In order to prevent this from happening, hit the "Cancel" button for the
OBSERVE before switching views to a different resource. This will unregister
the observer.

In very sleepy mode, the radio is not truly always off. The contiki core needs
to perform other periodic tasks in order to maintain network connectivity. For
that reason, this example will allow the radio to turn on periodically even
while in very sleepy mode. Thus, you may see that the node becomes briefly
reachable every now and then. However, do not count on those periods of
reachability to perform any tasks, as they will be brief and will be disrupted
without warning.
