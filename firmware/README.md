# picstick Firmware

Firmware for the picstick programmer. The firmware is designed to be able to run
with the internal 8MHz oscillator of an attiny44V. If the 8MHz oscillator is
used, the max baud rate is 76800. While the attiny44V is only specced to run on
an external crystal at a maximum of 8MHz @ 3.3v, succes has been found with a
crystal running at 16MHz @ 3.3v. This gives a max baud rate of 115200+.


## Building

### Requirements
- make
- avr-gcc
- avrdude (for flashing)

### Makefile Commands
> `make [fw]`   - Build firmware with settings defined in Makefile.\
> `make flash`  - Flash firmware using avrdude, building if neccessary.\
> `make fuses`  - Burn fuses as defined in the Makefile using avrdude.\
> `make clean`  - Remove built firmware files.\
> `make fclean` - Remove all files and folders created.


Build firmware, flash, and set fuses:
```
make flash fuses
```

## Configuration

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