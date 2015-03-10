Ubidots client example
======================
This is a very simple example aiming to demonstrate the functionality of the
ubidots app. It will connect to the [Ubidots](http://app.ubidots.com/) service
and it will start publishing data.

The example will work over IP as well as IPv6.

The example will demonstrate the basic functionality of Contiki's Ubidots
library:

* How to use the library to POST to a variable
* How to use the library to POST to a collection
* How to receive (parts of) the HTTP reply

For more in-depth information about the library itself and its capabilities,
read the library's doxygen documentation.

This example was developed, tested and should always work for the following
platforms:

* cc2538dk
* srf06-cc26xx 

It is intentionally left entirely platform-independent, so more platforms
should work.

Many thanks to Antonio Liñán Colina ([Zolertia](http://zolertia.io)) for his
ideas and contributions, and also to the Ubidots team for the support.

Register with Ubidots and create your data source, variables and auth token
---------------------------------------------------------------------------
First, you will have to register an account with Ubidots, create your data
source and variables and request an authentication token.

* Create an account with Ubidots and log in
* Create a single data source
* Under this data source, create two variables: One for the uptime and another
  for the sequence number.
* Go to your account and request a short API token.

Configure the Example
---------------------
There are three things to configure in the example:

* The choice between IP and IPv6. If you are planning to connect to Ubidots
  over NAT64, you will also want to configure the Ubidots server's IP address
  in a NAT64 format.
* The authentication token
* The variable IDs

### IP Configuration
The example will build for IPv6 by default. To switch to IP, open the example's
`Makefile`. Change `CONTIKI_WITH_IPV6=1` to `CONTIKI_WITH_IPV4=1`.

If you want to keep using IPv6 but you are behind NAT64 and you don't have an
accompanying Bind installation that will translate A records to AAAA ones, then
open the example's `project-conf.h`. Add this:

    #define UBIDOTS_CONF_REMOTE_HOST    "64:ff9b::3217:7c44"

Bear in mind that Ubidots' IPv4 address may change, in which case you will have
to adjust `UBIDOTS_CONF_REMOTE_HOST` accordingly.

### Authentication Token
There are plenty of ways to do this:

* You can set `UBIDOTS_WITH_AUTH_TOKEN` as an environment variable, by opening
  a console and running this:

        export UBIDOTS_WITH_AUTH_TOKEN=<the-token>

* or you can set the value of `UBIDOTS_WITH_AUTH_TOKEN` directly in the
  example's `Makefile`, by adding the line below somewhere before the
`ifdef UBIDOTS_WITH_AUTH_TOKEN` block.

        UBIDOTS_WITH_AUTH_TOKEN=<the-token>

* Lastly, you can edit `project-conf.h` and specify the token with a `#define`:

        #define UBIDOTS_CONF_AUTH_TOKEN "<the-token>"

Note well: If you chose the last option then:

* It's `UBIDOTS_CONF_AUTH_TOKEN`, and _not_ `UBIDOTS_WITH_AUTH_TOKEN`.
* Skip the double quotes around `<the-token>` for the first two choices.
  Include them for the third one.

### Variable IDs
You will need to configure the example in terms of what Ubidots variable ID to
use for the uptime and what ID for the sequence number. To do so, define the
following two:

    #define UBIDOTS_DEMO_CONF_UPTIME   "<the-ID>"
    #define UBIDOTS_DEMO_CONF_SEQUENCE "<the-other-ID>"

You can add those defines in `project-conf.h` or in `ubidots-demo.c`.
