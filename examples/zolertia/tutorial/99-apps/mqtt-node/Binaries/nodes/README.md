# Binaries for the MQTT client

The provided binaries are working images of the business-logic use cases in the `device` location.

The binaries are compiled as default to use the 2.4GHz RF interface and LLSEC enabled.  To decrypt the IEEE 802.15.4 frames use Wireshark and enter the pre-shared key `000102030405060708090A0B0C0D0E0F`

## Flashing

The starting flash address is `0x00202000`, replace the USB enumerated port depending on your OS and the application you want to flash, and run:

`python ../../../../flash-zoul.py -e -w -v -p /dev/ttyUSB1 -a 0x00202000 relayr/mqtt-relayr-remote-2_4ghz-ch26-llsec.bin`

After flashing the device, it will reboot and the RED LED will be on for ~30 seconds, this is a hard-coded time to allow the device to join a PAN.

Follow the instructions in the next section to configure the device, now it will wait until valid credentials are given (in the Bluemix case, it will continue as detailed in the next paragraph as it does not uses login credentials).

After joining a network, it will start blinking the GREEN LED until it has connected to the Broker.

## Configure the device

As default the binaries are compiled without any user/token information, you need to open a web browser in a computer in the same LAN as the Zolertia's Ethernet Router, and via its web service open the `Sensors` tab, i.e:

`http://[bbbb::100]/sensors.html`

Then copy the device IPv6 address and type as follow in a different browser's tab:

`http://[aaaa::212:4b00:60d:607a]/config.html`

Follow the specific platform information to know what information goes in each field.
