
# picstick

An AVR based programming adapter for PIC microcontrollers that use Microchip's
low-voltage In Circuit Serial Programming (ICSP) interface.
It decodes the flash data sent from a [host application](https://github.com/rex--/picchick)
and programs it into the memory of a PIC microcontroller.

[![](docs/pcb-bottom-v1_0s.png)](docs/pcb-bottom-v1_0.png)[![](docs/pcb-top-v1_0s.png)](docs/pcb-top-v1_0.png)

The picstick can be plugged directly into a USB A port. It provides a 3x2 pin
header that breaks out the needed pins for ICSP, and +3.3V and GND. 


## Hardware
The hardware is based around an ATtiny44. Communication with the host PC is done
using a CH340E USB-to-UART bridge.


## Firmware
While the ATtiny44 has no hardware UART, it does have the Universal Serial
Interface(USI). This is leveraged to provide a faster and more compact software
UART. Because of this, the firmware will only run on AVR devices with a USI.

The firmware is designed to be able to run
with the internal 8MHz oscillator of an attiny44V. If the 8MHz oscillator is
used, the max baud rate is 76800. While the attiny44V is only specced to run on
an external crystal at a maximum of 8MHz @ 3.3v, succes has been found with a
crystal running at 16MHz @ 3.3v. This gives a max baud rate of 115200+.

### Building

#### Requirements
- make
- avr-gcc
- avrdude

#### Makefile Commands
Compilation is done with the make utility:
```sh
make [fw]       # Build firmware with settings defined in Makefile.
make flash       # Flash firmware using avrdude, building if neccessary.
make fuses      # Burn fuses as defined in the Makefile using avrdude.
make clean      # Remove built firmware files.
make fclean     # Remove all files and folders created.
```


Build firmware, flash, and set fuses:
```
make flash fuses
```

### Configuration

Currently, the configuration is spread out amoung several files:
- uuart.h - UART baudrate, USI serial library configuration.
- icsp.h - Pins to use for ICSP interface.
- Makefile - Oscillator frequency configuration.

**8Mhz @ 76800 bauds - Internal Oscillator**\
_L: E2 &nbsp;&nbsp; H: DF &nbsp;&nbsp; E: FF_\
This is the default configuration the firmware builds with. It does not require
any additional external components.

**8MHz @ 76800 bauds - External Crystal**\
_L: FD &nbsp;&nbsp; H: DF &nbsp;&nbsp; E: FF_\

**16MHz @ 115200 bauds - External Crystal**\
_L: FF &nbsp;&nbsp; H: DF &nbsp;&nbsp; E: FF_\
This configuration isnt officially supported by the ATtiny44V, but it seems to
work just fine.