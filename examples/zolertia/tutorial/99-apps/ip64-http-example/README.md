# RE-Mote IP64 README file

This example shows how to deploy an IP64 ethernet-based router using Zolertia's `eth-gw` Ethernet gateway, based on the Zoul and ENC28J60 modules, with active POE support.

## IP64 router

The router packs a built-in webserver and optionally can run on 2.4GHz or with the Sub-1GHz radio interface.  In the `project-conf.h` file you can alternatively enable one or another as follows:

* RF 2.4GHz (cc2538 built-in)

````
#define NETSTACK_CONF_RADIO         cc2538_rf_driver
#define ANTENNA_SW_SELECT_DEF_CONF  ANTENNA_SW_SELECT_2_4GHZ
````

* RF Sub-1GHz (CC1200)

````
#define NETSTACK_CONF_RADIO         cc1200_driver
#define ANTENNA_SW_SELECT_DEF_CONF  ANTENNA_SW_SELECT_SUBGHZ
````

To compile and flash run:

````
cd ip64-eth-gw
make TARGET=zoul BOARD=eth-gw ip64-router.upload
````

As default we enable the `DHCP` support for autoconfiguration.  Just connect to a DHCP-enabled device to obtain an IPv4 IP address and that's it!.

## HTTP client examples

There are available 2 examples ready to use using the `http-socket` library:

* The `client` example just makes a HTTP `GET` request to a know page and retrieves
  the result.

* The `ifttt_client` example sends a HTTP `POST` request to [IFTTT](https://ifttt.com/recipes) whenever the user button is pressed, building an Internet button to connect to several channels and applications, such as `Drive`, `Evernote` and many others.

To configure the `IFTTT` demo just edit the `project-conf.h` file and change the name of the event and write your API key:

````
#define IFTTT_EVENT   "button"
#define IFTTT_KEY     "XXXXXX"
````

To compile and flash:

````
cd client
make TARGET=zoul ifttt_client.upload
````

