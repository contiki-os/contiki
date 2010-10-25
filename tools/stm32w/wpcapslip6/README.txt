This software needs a working network adapter. You can install a Microsoft Loopback adapter.
(Windows XP users: remeber to reboot after the installation procedure).

In order to install this kind of device in Windows 7, use
devcon utility (you can download it from Microsoft website).

> devcon.exe install %windir%\inf\netloop.inf *msloop