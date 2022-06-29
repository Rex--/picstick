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

    GIMSK |=  (1<<PCIE);		   // Enable pin change interrupt
    PCMSK |=  (1<<PCINT6);		   // Enable pin change interrupt for PA6
    sei();

    DDRB |= (1 << 2);   // LED pin output

    for (;;)
    {
        if (uuart_rx_data_available())
        {
            PORTB |= (1 << 2);
            handle_command();
            PORTB &= ~(1 << 2);
        }
    }
}