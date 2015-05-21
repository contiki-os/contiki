CC26xx Demo
===========
This example demonstrates basic functionality for the two supported CC26xx
boards. More specifically, the example demonstrates:

* How to take sensor readings
* How to use buttons and the reed relay (triggered by holding a magnet near S3
  on the SensorTag).
* How to send out BLE advertisements. The device will periodically send out BLE
  beacons with the platform name as payload. Those beacons/BLE ADV packets can
  be captured with any BLE capable device. Two such applications for iOS are the
  TI Multitool and the TI Sensortag app. They can be found in the Apple App
  Store. If you have a BLE-capable Mac, you can also use LightBlue for OS X.
