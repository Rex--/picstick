/** @file main.c
 * 
 * picstick - A usb programmer for PICs that use
 * Microchip's In Circuit Serial Programming(ICSP) interface.
 * 
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uuart.h"
#include "icsp.h"
#include "commands.h"


int
main (void)
{
    uuart_init();
    icsp_init();

    for (;;)
    {
        handle_command();
    }
}