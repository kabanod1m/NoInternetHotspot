# NoInternetHotspot
A Win32 program for Windows 10+ to host a Wi-Fi Hotspot without sharing current Internet connection. Useful for directly connecting devices via Wi-Fi when you don't want those devices to be able to connect to Internet for whatever reason.\
The code is a mess, I'm sorry. This is my first Windows API "C++" project.\
This is really just a GUI implementation of [this Windows classic sample](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/WiFiDirectLegacyAP).

### Usage
You need Windows 10, a Wi-Fi card that is able to host a Wi-Fi Hotspot using Wi-Fi Direct and you will probably need to connect to a Wi-Fi network yourself first, because some Wi-Fi adapters will not advertise a network until they are themselves connected, due to artificial limitations.\
Then just download the executable file from [Releases](https://github.com/kabanod1m/NoInternetHotspot/releases) and you should be able to run it. No need to install Microsoft Visual C++ Redistributable package, it should be bundled with the executable. Check out Help -> About! The rest should be self-explanatory.

### License
NoInternetHotspot is licensed under GPL-3.0.
