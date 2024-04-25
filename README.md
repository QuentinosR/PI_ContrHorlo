# PI


## Installation
1. Clone repository https://labinfo.ing.he-arc.ch/gitlab/igib/public/raspberry-pi-pico/raspberry-pi-pico-sdk
2. In the folder, ```git submodule update --recursive --init pico-sdk```
3. Clone this git into the folder raspberry-pi-pico-sdk.


## Build and flash

- Build only : ```./build.sh -b -p PI_ContrHorlo/uc/```
- Build and flash : ```./build.sh -bf -p PI_ContrHorlo/uc/```


## Configuration
- Raspberry Pi Pico
- Raspberry Pi Debug

- Ubuntu 22.04.4 LTS