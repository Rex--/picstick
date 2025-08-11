/** @file led.h
 *
 * This header file provides easy to use macros to control a LED.
 * 
*/

#ifndef _led_h_
#define _led_h_

#define LED_PIN     2
#define LED_PORTx   PORTB
#define LED_DDRx    DDRB

/** Initialize LED pin to output. */
#define led_init()  ((LED_DDRx) |= (1 << (LED_PIN)))

/** Turn LED on. */
#define led_on()    ((LED_PORTx) |= (1 << (LED_PIN)))

/** Turn LED off. */
#define led_off()   ((LED_PORTx) &= ~(1 << (LED_PIN)))

#endif
