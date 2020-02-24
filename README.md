<p align="center" >
  <img src="https://github.com/mistak1992/MKCooling-ESP32/blob/master/img/MKCooling.jpg?raw=true" float=left>
</p>

MKCooling-ESP32
===============
cooling for Mac mini 

## Getting started
### Hardware requirement

#### 1. using Wemos D1 V1.0.0-ESP32 as MCU 

<p align="center" >
  <img src="https://github.com/mistak1992/MKCooling-ESP32/blob/master/img/Wemos-D1-V1-0-0-ESP32-WiFi-and-Bluetooth-module-development-module-CP2104-development-board-Computer.jpg?raw=true" float=left>
</p>
#### 2. using Noctua NF-A14 5V PWM as main fan
<p align="center" >
  <img src="https://github.com/mistak1992/MKCooling-ESP32/blob/master/img/NOCTUA.jpg?raw=true" float=left>
</p>
#### 3. using GY-906-BCC MLX90614ESF-BCC IR as temperature detect part and reset switch (optional)
<p align="center" >
  <img src="https://github.com/mistak1992/MKCooling-ESP32/blob/master/img/MLX90614.jpg?raw=true" float=left>
</p>
#### 4. print the enclosure with 3D printer

[MKCooling](https://www.thingiverse.com/thing:4181227)

<p align="center" >
  <img src="https://github.com/mistak1992/MKCooling-ESP32/blob/master/img/MKCooling_enclosure.png?raw=true" float=left>
</p>


### software requirement
1. This is a project base on esp-idf, so you need to setup the environment for esp-idf refer to this [offical document](https://docs.espressif.com/projects/esp-idf/en/v4.0/)

## Step by step

### 0. change the GPIO MARCO in main.c to suit you
```
#define LED_R_IO 5
#define HALL_GPIO 13
#define PWM_GPIO 14
#define MLX90614_SDA_GPIO 25
#define MLX90614_SCL_GPIO 18
#define MLX90614_GND_GPIO 23
#define MLX90614_VCC_GPIO 19
#define BLE_RESET_GPIO 21
```
### 1. connect MCU to PC
### 2. build the project
### 3. flash it to ESP board
### 4. enjoy the smart cooling pad with [MKCooling](https://github.com/mistak1992/MKCooling-MacOS) under MacOS
