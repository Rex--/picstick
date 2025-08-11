/** @file icsp.c
 * 
 * This part of the firmware implements the ICSP interface by bitbanging some
 * GPIO pins.
 *
*/

#include <avr/io.h>
#include <util/delay.h>

#include "icsp.h"

// Change pin states.
#define icsp_pins_outputs() (ICSP_DDR |= (ICSP_PIN_MCLR | ICSP_PIN_CLK | ICSP_PIN_DAT))
#define icsp_pins_inputs()  (ICSP_DDR &= ~(ICSP_PIN_MCLR | ICSP_PIN_CLK | ICSP_PIN_DAT))
#define icsp_pins_low()     (ICSP_PORT &= ~(ICSP_PIN_MCLR | ICSP_PIN_CLK | ICSP_PIN_DAT))
#define pin_low(pin)    (ICSP_PORT &= ~(pin))
#define pin_high(pin)    (ICSP_PORT |= (pin))


void
icsp_init (void)
{
    // Our target state for our three ICSP pins are inputs without pullup.
    // This is the default state for AVRs.
    icsp_pins_inputs();
}


void
icsp_enable (void)
{
    // To enter program mode, we set MCLR low and shift in the 32 bit startup key
    
    icsp_pins_outputs();    // Our pins are in an input state.
    icsp_pins_low();        // Set all pins low, including MCLR.

    _delay_us(ICSP_DELAY_ENTH); // Wait Entry Hold Time period.

    unsigned char i, j;
    for (i=0; i < 4; i++) { // Start with the most significant byte
        unsigned char bit = 0x80;
        for (j=8; j > 0; j--) { // And the most significant bit

            pin_high(ICSP_PIN_CLK); // CLK High

            // Determine the next data bit in the startup sequence.
            if ((ICSP_STARTUP_KEY[i]) & bit)
            {
                // Transmit a 1
                pin_high(ICSP_PIN_DAT);
            }
            else
            {
                // Transmit a 0
                pin_low(ICSP_PIN_DAT);
            }

            _delay_us(ICSP_DELAY_CKH); // Wait a Clock High period.

            // CLK low, this will cause the connected chip to latch the data.
            pin_low(ICSP_PIN_CLK);

            bit = bit >> 1;

            _delay_us(ICSP_DELAY_CKL); // Wait a Clock low period.
        }
    }
}

void
icsp_disable (void)
{
    pin_high(ICSP_PIN_MCLR); // Set MCLR high to exit programming mode.

    _delay_us(10);  // Wait for a bit.

    icsp_pins_inputs();     // Configure pins as inputs.
}

void
icsp_command (unsigned char data)
{
    unsigned char bit = 0x80;
    for (char i = 8; i > 0; i--) { // Start with the MSb

        pin_high(ICSP_PIN_CLK); // CLK High

        // Determine the next data bit in the command.
        if (data & bit)
        {
            // Transmit a 1
            pin_high(ICSP_PIN_DAT);
        }
        else
        {
            // Transmit a 0
            pin_low(ICSP_PIN_DAT);
        }
        
        _delay_us(ICSP_DELAY_CKH);

        pin_low(ICSP_PIN_CLK); // CLK Low

        bit = bit >> 1;

        _delay_us(ICSP_DELAY_CKL);
    }
}

void
icsp_payload (unsigned long data)
{
    // A data payload is 24 bits long:
    // (1)  - Start bit (0)
    // (22) - Data bits
    // (1)  - Stop bit (0)
    // We can craft this payload by shifting the data left by one.
    data = (data << 1UL) & 0x7FFFFFUL;

    unsigned long bit = 0x800000UL;
    for (char i = 24; i > 0; i--) { // Start with the MSb

        pin_high(ICSP_PIN_CLK); // CLK High

        // Determine the next data bit in the command.
        if (data & bit)
        {
            // Transmit a 1
            pin_high(ICSP_PIN_DAT);
        }
        else
        {
            // Transmit a 0
            pin_low(ICSP_PIN_DAT);
        }
        
        _delay_us(ICSP_DELAY_CKH);

        pin_low(ICSP_PIN_CLK); // CLK Low

        bit = bit >> 1;

        _delay_us(ICSP_DELAY_CKL);
    }
}


unsigned long
icsp_read (void)
{
unsigned long payload = 0;

    // Configure our DAT pin as input
    ICSP_DDR &= ~(ICSP_PIN_DAT);

    // Clock out 24 cycles and read the data bits on a CLK fall
    for (char i = 24; i > 0; i--)
    {
        pin_high(ICSP_PIN_CLK);     // CLK High

        payload = payload << 1;

        _delay_us(ICSP_DELAY_CKH);  // Wait for chip to latch the next bit

        pin_low(ICSP_PIN_CLK);      // ClK Low

        // Record data state.
        if (ICSP_PIN & ICSP_PIN_DAT)
        {
            payload |= 1;
        }

        _delay_us(ICSP_DELAY_CKL);  // Wait a clock low period
    }

    // Set start/stop bits to 0 (They should be, but not guaranteed)
    payload &= 0x7FFFFE;

    // Set DAT pin back to output
    ICSP_DDR |= ICSP_PIN_DAT;

    return payload;
}
