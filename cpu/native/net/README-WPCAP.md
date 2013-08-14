Contiki network I/O on Microsoft Windows (including the Cygwin environment) is
implemented based on the quite popular WinPcap library that is available at
[http://winpcap.org](http://winpcap.org).

Developing Contiki network applications most likely involves working with a
network protocol analyzer. Wireshark (formerly known as Ethereal) is a very
popular one that on Windows uses - and actually comes with - the WinPcap
libary. Wireshark is available at [http://wireshark.org](http://wireshark.org).

So with Wireshark installed Contiki network I/O doesn't need any additional
components.

On Windows every Contiki application has one obligatory comand line argument
that identifies the Windows network interface to be used by Contiki. While on
Unix those network interfaces are called i.e. '/dev/tap0' they have on Windows
names like

    \Device\NPF_{F76B480A-1D31-4B3D-8002-C0EF49185737}

In order to avoid the necessity to enter such names on the command line instead
the IPv4 address used by Windows is entered to identify the network interface
to be used by Contiki. Please note that this IPv4 address is _NOT_ the IPv4
address to be used by Contiki !

Contiki network I/O on Windows uses the same MAC address used by Windows. This
approach often described as IP-Aliasing was primarily chosen because it avoids
putting the network interface into promiscuous mode. The major benefit of this
is the compatibility with WLAN interfaces - which mostly come with Windows
device drivers incapable of promiscuous mode.

The WinPcap library works fine with the 'Microsoft Loopback Adapter' so it's
easy to have a Contiki network application running on Windows communicate with
the local Windows instance for testing purposes - and monitor the communication
with Wireshark.

Windows Vista however tries to identify networks by the MAC address of the
default router. If that fails the network is defined as an 'Unidentified
Network' and thus classified as 'Public Network' resulting in very strict
firewall settings. As there's no default router for a loopback interface the
interface is always considered as a public network - which is kind of the
opposite of the actual situation ;-)

Instead of fiddling with the firewall settings for 'Public Networks' (or even
turning the firewall completely off) there's a clean solution which defines the
loopback interface as not a true network interface that connects to a network.
This results in generally deactivating both the network identification process
and the firewall for the loopback interface. The details are available in the
Microsoft TechNet Forums thread 'Vista Network Identification for Loopback
Adpater' that is currently available at
[link](http://social.technet.microsoft.com/Forums/windows/en-US/66b42761-1b8e-4302-9134-0bb685139f4e/vista-network-identification-for-loopback-adpater)
