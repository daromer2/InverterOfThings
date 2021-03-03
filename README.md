# InverterOfThings Logger (FORK and total rebuild again)
ESP32 based WiFi interface for Voltronic/MPP solar 5.5KW hybrid inverter and maybe  Axpert MppSolar PIP inverters


## Refactoring Again
This is refactored from version done by Daniel aka Daromer aka DIY Tech & Repairs 2020 https://github.com/daromer2/InverterOfThings.

**WARNING WIP, This is work in progress and all features may not work yet **

Refactoring:
- ESP32
- PlatformIO used for building
- Uses Native ESP32 FreeRtos and not fake having plain Arduino. 
- Function divided multiple FreeRtos tasks 
- MPPSolar/Voltronix 5.5KW Hybrid inverter support added
- Rest API
- ESP32 TFT LCD Display for displaying inverter status




**Note2 that all below info is from the forked project and in short this will be updated when all is refactored and perhaps working....**

# Overview
Based around an ESP32 WiFi microcontroller.
Software is built using ESP32 Arduino and PlatformIo. 

**PROGRAMMING:** You need to program it before hooking it up to the TTL board! Im using HW Serial since its so buggy using Software Serial and Wifi reliable so its not even worth it.

**Alternate PROGRAMMING:** I have now included a BIN file that you just can flash. Nice right?!?!

**UART2:** Talks to the inverter at 2400 baud from ESP32 Serial2

**UART0:** Used for debugging only prpogramming . Just connect another serie adaptor to TX.


**GPIO:** NOT IN USE Button on GPIO0, changes wifi mode, also used for programming when DTR pin isn't available.


**POWER:** Using a DC/DC or USB power currently. My gear dont have any power on the serial port

**WIFI:** The system defaults to AP mode on first setup. Surf to "SetSol" AP and then surf to http://192.168.4.1/ On this page configure WIFI and MQTT. Then you need to reboot the device either by the reboot botton or hard reboot. No changes are applied properly before reboot


# Parts required to build one

Most of the parts can be bought as modules, it's usually cheaper that way.

Approximate prices found on Ebay
- ESP32 TTGO-T4 board https://www.aliexpress.com/item/32854502767.html?spm=a2g0s.9042311.0.0.27424c4dbpyyv6   € 13,33
- MAX3232 module ($0.70) - it's just a max3232 with 5 capacitors on a tiny little board  - https://ebay.to/3la4G45
- DC-DC buck module ($1.50) - 12-80v down to 5v https://ebay.to/3lbf3V7
- LEDs ($0.50) - The leds are for status etc

Total cost: $20

Links above are affiliated and give me a tiny commission if used. 
