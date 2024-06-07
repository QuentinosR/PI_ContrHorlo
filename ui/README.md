Graphical interface for controlling the various signals generated by the Raspberry Pi Pico. 
This eliminates the need for the user to enter commands on the UART.

# Prerequisites
The interface is developed in Python with PyQt6. Both must be installed beforehand. 

# Usage
```python3 main.py DEVICE_NAME```

DEVICE_NAME is on Linux generally ttyACM0. 

# Note
A command is sent as soon as the button is pressed. 
On start-up, the signals are switched off and a default setting is made. 
All messages received by the device are displayed. 

# Configuration
- Ubuntu 22.04.4 LTS
- Python 3.10.12
- Raspberry Pi Pico
- Raspberry Pi Debug