# Petfeeder Sensor

This project is part of a larger smart home effort working with Home Assistant. I feel like there should be a better
way to document all the subproject in one place, so that might still come.

Complete DIY was also an option, as done by [jplanaux's 3D printed cat feeder](https://www.thingiverse.com/thing:3869594) or [many others](https://www.thingiverse.com/search?q=auger+pet&type=things&sort=relevant) on Thingiverse. Plenty other can be found on [instructables.com](https://www.thingiverse.com/search?q=auger+pet&type=things&sort=relevant). I don't have a 3D printer (yet), and I had the idea that modding a 2nd hand pet feeder would be easier to achieve (as well as everything being food grade material).

## Table of Contents

1. [MQTT communication](#MQTT-communication)
1. [ESP8266 toolchain SDK installation](#ESP8266-toolchain-SDK-installation)
1. [CLion Instructions](#CLion-Instructions)
1. [WiFi and MQTT configuration with SDKCONFIG](#WiFi-and-MQTT-configuration-with-SDKCONFIG)

## MQTT communication
Motor to next position, send to topic `/petfeeder/calibrate/tick` (arbitrary payload, may be empty).<br>
Request raw scale value, send to topic `/petfeeder/calibrate/raw/req` (arbitrary payload, 
may be empty).<br>
Result of raw scale request can be read from topic `/petfeeder/calibrate/raw/resp` <br>

## General about the code

It seems MQTT publish can not be called too frequently, crashes on
2 sec interval, seems fine on 5 sec. Alternative is to 
gather more readings, and publish all at once

clion
## ESP8266 toolchain SDK installation

For clarity, toolchain refers to the `xtensa-lx106-elf-XXX` binaries used to compile your ESP8266 projects with, and 
SDK refers to the libraries/header files and examples from Espressif to make your life as a programmer easier. Also the 
tool to flash and monitor your device is part of the SDK.

Get the latest toolchain from the Github [repository of Espressif](https://github.com/espressif/ESP8266_RTOS_SDK/)
After you've gotten the latest toolchain, follow the remaining instructions on the repository to also install the 
latest SDK. More information about the installation can be found on their [getting started page](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/).



## CLion Instructions

Open **File** > **Settings** > **Build, Execution, Deployment** > **CMake**. Select
 the profile you want to work on (or create if there's none yet). For *CMake options* 
 fill in:
 ```
-DCMAKE_C_COMPILER="/home/user/esp/xtensa-lx106-elf/bin/xtensa-lx106-elf-gcc" -DCMAKE_CXX_COMPILER="/home/user/esp/xtensa-lx106-elf/bin/xtensa-lx106-elf-g++"
```
Note: replace with your own absolute paths of where you put the toolchain.

For *Build options* put `-- -j N` where N is the number of processor cores in your system + 1. So for a quad core 
processor N will be 5.

For *Environment* I had to put `ESPPORT=/dev/ttyUSB0` to force the SDK to use the correct USB port. This might
not be necessary in your case. Of course replace with the USB port your device is connected to. The USB port can be 
found on Linux by in a terminal running `ls /dev/ttyUSB?`. If there's multiple results, disconnect your device to find
the difference in result, that being your ESP8266.

CMakeLists.txt in root directory needs reference to IDF_PATH. When using `include($ENV{IDF_PATH}/tools/cmake/project.cmake)
`, use 
## WiFi and MQTT configuration with SDKCONFIG 

Open a terminal in your project directory and run `make menuconfig`. A custom entry has been added thanks to
 the `Kconfig.projbuild` file in the `main` folder. Here enter the SSID and password of your WiFi. 
 For the MQTT broker URL adhere to the following format `mqtt://[username]:[password]@[broker_url]:[port]`. 
 Here replace with your own username, password, broker url and its MQTT port.
 
 In order to enable the printing of float and doubles, in `make menuconfig`, go to `Component config` -> `Newlib` 
and **disable** `Enable 'nano' formatting options for printf/scanf family` (it's enabled by default).

To change the default log level, go to `Component config` -> `Log output`.