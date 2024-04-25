# Installation

## Install the SDK
The download page is at https://www.svs-vistek.com/en/industrial-cameras/svs-camera-detail.php?id=exo273CGE.

Download in the Software section, SVSCamKit 2.5.9 (Linux x86). Maybe a new version is available and will works. If not available, the compressed file is in this folder

Extract it and go in it.

``` cd SVCapture2Setup_Linux_release ```

A file help to install it. Its names is **Quick User Guide.txt** and available in the folder **SVCamKit**. The instructions presented are the same as those in this document.

```
chmod +x setup.sh
sudo ./setup.sh -i
```

Reboot the computer.

## Configure Networks
1. Go to the Network setting
2. In the "Identity" tab, set MTU to 9000
3. In IPv4 tab, check the "Link-Local Only"


# Compilation
1. Go in the folder **PI_ContrHorlo/ImageProc/code**
2. ```make```

# Run executable
1. ```./imageProc```


# Configuration
- Ubuntu 22.04.4 LTS
- Camera EXO273CGE
- SVSCamKit 2.5.9 Linux