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
#include "led.h"


int
main (void)
{
    uuart_init();
    icsp_init();

    GIMSK |=  (1<<PCIE);		   // Enable pin change interrupt
    PCMSK |=  (1<<PCINT6);		   // Enable pin change interrupt for PA6
    sei();

    led_init();

    for (;;)
    {
        if (uuart_rx_data_available())
        {
            led_on();
            handle_command();
            led_off();
        }
    }
}