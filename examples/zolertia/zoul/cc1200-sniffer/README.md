CC1200 README
========================

The CC1200 sniffer is heavily based on the CC2538 sniffer, used with the
IEEE 802.15.4 Sensniff application by George Oikonomou.

Sensniff requires [Wireshark](http://www.wireshark.org/).

Get Wireshark
-----------------
The best way is to go to the Wireshark site and follow the instructions for your
specific OS, in a bundle this will install for Ubuntu/LInux systems:

`sudo apt-get install wireshark`

To allow non-super users to capture packets:

`sudo dpkg-reconfigure wireshark`

Flash the sniffer application to the Zoul
-----------------
make sniffer.upload

Run Sensniff
-----------------
```
git clone https://github.com/g-oikonomou/sensniff
cd sensniff/host
python sensniff.py --non-interactive -d /dev/ttyUSB0 -b 460800
```

On another terminal run:

`sudo wireshark -i /tmp/sensnifff`

