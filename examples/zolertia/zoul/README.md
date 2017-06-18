Zolertia Zoul test examples
============================================

The following tests are valid for the following platforms:

* RE-Mote revision A
* RE-Mote revision B
* Firefly

Specific RE-mote revision A examples are available at the `rev-a` folder.

Compile and install an example
-------------------

To flash either hardware platform use the same `TARGET=zoul` and the following:

* RE-Mote revision A : `BOARD=remote-reva`
* RE-Mote revision B : `BOARD=remote-revb`
* Zolertia Firefly   : `BOARD=firefly`

An example on how to compile is shown next:

`make TARGET=zoul BOARD=remote-revb`

Or alternatively if you just type `make`, it will default to use the `BOARD=remote-revb`.

For backward compatibility with the previous `remote` target corresponding to the
RE-Mote revision A, using `BOARD=remote` will default to `BOARD=remote-reva`.

To upload an example to your Zolertia device, just add the `.upload` target as:

`make TARGET=zoul BOARD=remote-revb zoul-demo.upload`

Optionally you can select a specific USB port to flash a given device, in Linux
and assuming there is a device at the `/dev/ttyUSB0`:

`make TARGET=zoul BOARD=remote-revb zoul-demo.upload PORT=/dev/ttyUSB0`

If you ommit the `PORT` argument, the system will flash all Zolertia devices connected over USB.

Visualize the console output
-------------------

Just type `make login` to open a connection to the console via USB.
As above to specify a given port use the `PORT=/dev/ttyUSB0` argument.

Alternatively you can save the above `PORT`, `TARGET` or `BOARD` as follows:

`export TARGET=zoul BOARD=remote-revb PORT=/dev/ttyUSB0`

This will save you to type these when running a command on the terminal

Documentation and guides
-------------------

More information about the platforms, guides and specific documentation can be found at [Zolertia Wiki][wiki]

[wiki]: https://github.com/Zolertia/Resources/wiki "Zolertia Wiki"

