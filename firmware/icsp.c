/** @file icsp.c
 * 
 * This part of the firmware implements the ICSP interface by bitbanging some
 * GPIO pins.
 *
*/

#include <avr/io.h>
#include <util/delay.h>

#include "icsp.h"


// Startup bit sequence to enter programming mode is cleverly MCHIP in ascii.
#define ICSP_STARTUP_KEY "MCHP"

// ICSP Timings
#define ICSP_DELAY_ENTH 250
#define ICSP_DELAY_CKL 1
#define ICSP_DELAY_CKH 1
#define ICSP_DELAY_DLY 3
#define ICSP_DELAY_ERAB 8600    // Bulk erase time takes max 8.4 ms
#define ICSP_DELAY_ERAR 3000    // Row erase time is max 2.8 ms
#define ICSP_DELAY_PINT_PM 3000 // Program memory internal timed takes max 2.8ms
#define ICSP_DELAY_PINT_CW 5800 // Configuration word internally timed takes max 5.6 ms



// Change pin states.
#define icsp_pins_outputs() (ICSP_DDR |= (ICSP_PIN_MCLR | ICSP_PIN_CLK | ICSP_PIN_DAT))
#define icsp_pins_inputs()  (ICSP_DDR &= ~(ICSP_PIN_MCLR | ICSP_PIN_CLK | ICSP_PIN_DAT))
#define icsp_pins_low()     (ICSP_PORT &= ~(ICSP_PIN_MCLR | ICSP_PIN_CLK | ICSP_PIN_DAT))
#define pin_low(pin)    (ICSP_PORT &= ~(pin))
#define pin_high(pin)    (ICSP_PORT |= (pin))


// Internal functions.
static void icsp_write (unsigned char data);


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

    int i, j;
    for (i=0; i < 4; i++) { // Start with the most significant byte
        for (j=7; j >= 0; j--) { // And the most significant bit

            pin_high(ICSP_PIN_CLK); // CLK High

            // Determine the next data bit in the startup sequence.
            if ((ICSP_STARTUP_KEY[i]) & (1 << j))
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

            _delay_us(ICSP_DELAY_CKL); // Wait a Clock low period.
        }
    }
}

void
icsp_disable (void)
{
    // Exits programming mode

    pin_high(ICSP_PIN_MCLR); // Set MCLR high to exit programming mode.

    _delay_us(10);  // Wait for a bit.

    icsp_pins_inputs();     // Configure pins as inputs.
}

void
icsp_command (unsigned char command)
{
    icsp_write(command);
}

void
icsp_payload (unsigned int data)
{
    // A data payload is 24 bits long:
    // (7)  - Start + Padding 0s
    // (16) - Data bits
    // (1)  - Stop 0s
    union {
        unsigned char bytes[3];
        long number;
    } payload;
    
    // We can craft this payload by shifting the data left by one.
    payload.number = data;
    payload.number = payload.number << 1;

    // Now we can write our payload by writing the 3 low bytes.
    icsp_write(payload.bytes[2]);
    icsp_write(payload.bytes[1]);
    icsp_write(payload.bytes[0]);
}


unsigned int
icsp_read (void)
{
    int i;
    unsigned int word = 0;

    // Configure our DAT pin as input
    ICSP_DDR &= ~(ICSP_PIN_DAT);


    // Clock out 9 clock dummy clocks
    for (i=0; i<9; i++)
    {
        pin_high(ICSP_PIN_CLK);
        _delay_us(ICSP_DELAY_CKH);
        pin_low(ICSP_PIN_CLK);
        _delay_us(ICSP_DELAY_CKL);
    }

    // Clock out 14 cycles and read the data bits on a CLK fall
    for (i=13; i>=0; i--)
    {
        pin_high(ICSP_PIN_CLK);     // CLK High

        _delay_us(ICSP_DELAY_CKH);  // Wait for chip to latch the next bit

        pin_low(ICSP_PIN_CLK);      // ClK Low

        // Record data state.
        word |= ((ICSP_PIN & ICSP_PIN_DAT) << i);

        _delay_us(ICSP_DELAY_CKL);  // Wait a clock low period
    }

    // Clock out our stop bit totalling 24 bits
    pin_high(ICSP_PIN_CLK);
    _delay_us(ICSP_DELAY_CKH);
    pin_low(ICSP_PIN_CLK);
    _delay_us(ICSP_DELAY_CKL);

    // Set DAT pin back to output
    ICSP_DDR |= ICSP_PIN_DAT;

    // return result
    return (word >> 2);
}



static void
icsp_write (unsigned char data)
{
    for (int bit = 7; bit >= 0; bit--) { // Start with the MSb

        pin_high(ICSP_PIN_CLK); // CLK High

        // Determine the next data bit in the command.
        if (data & (1 << bit))
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

        _delay_us(ICSP_DELAY_CKL);
    }
}
