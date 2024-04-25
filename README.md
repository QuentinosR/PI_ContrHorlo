# PI ContrHorlo


## Installation

### Pi Debug
Download and install the OpenOCD dependencies:

```bash
sudo apt install automake autoconf build-essential texinfo libtool libftdi-dev libusb-1.0-0-dev
```
Clone, build and install OpenOCD:

```bash
cd ~
git clone https://github.com/raspberrypi/openocd.git --branch rp2040-v0.12.0 --depth=1 --no-single-branch
cd openocd
./bootstrap
./configure
make -j4
sudo make install
```

Check OpenOCD version:

```bash
openocd --version
```

Output: 

```bash
Open On-Chip Debugger 0.12.0-g4d87f6d (2024-01-08-14:31)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html

```

Create a file with any name (ex. pico_openocd.rules) under /etc/udev/rules.d and put this line inside.

```bash
ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000c", MODE="660", GROUP="plugdev", TAG+="uaccess"
```



### SDK Config
1. Clone repository https://labinfo.ing.he-arc.ch/gitlab/igib/public/raspberry-pi-pico/raspberry-pi-pico-sdk
2. Go in the folder and init it:
    ```
    cd raspberry-pi-pico-sdk
    git submodule update --recursive --init pico-sdk
    ```
3. Clone this git into the folder raspberry-pi-pico-sdk.


## Create docker image     
It is possible to directly create the docker image in the raspberry-pi-pico-sdk folder:
```
docker build -t pico-dev . 
```

## Build and flash

- Build only : ```./build.sh -b -p PI_ContrHorlo/uc/```
- Build and flash : ```./build.sh -bf -p PI_ContrHorlo/uc/```


## Configuration
- Raspberry Pi Pico
- Raspberry Pi Debug
- Ubuntu 22.04.4 LTS