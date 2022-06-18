
# picstick

An AVR based programming adapter for PIC microcontrollers that use Microchip's
low-voltage In Circuit Serial Programming (ICSP) interface.


## Hardware
The hardware is based around an ATtiny44. It decodes the flash data sent from
a [host application](https://github.com/rex--/picchick) and programs it into the memory of a PIC microcontroller.
Communication with the host PC is done using a CH340E USB-to-UART converter.


## Firmware
While the ATtiny44 has no hardware UART, it does have the Universal Serial
Interface(USI). This is leveraged to provide a faster and more compact software
UART. Because of this, the firmware will only run on AVR devices with a USI.
