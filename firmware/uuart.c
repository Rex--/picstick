/**
 * @file USI_UART.c
 * This module contains the definitions and functions necessary to
 * implements a HW UART on a ATtiny using its Universal Serial Interface (USI).
 */
#include <avr/io.h>			// uController specific registers
#include <avr/interrupt.h>
#include <stdlib.h>			// for itoa() function
//#include <util/delay.h>		// for debugging main()


#include "uuart.h"


/* Static Variables */
static unsigned char          uuart_rx_buf[UART_RX_BUFFER_SIZE];
static volatile unsigned char uuart_rx_head;
static volatile unsigned char uuart_rx_tail;

static unsigned char          uuart_tx_buf[UART_TX_BUFFER_SIZE];
static volatile unsigned char uuart_tx_head;
static volatile unsigned char uuart_tx_tail;
static volatile unsigned char uuart_tx_data;


// Status byte holding flag definition, initialized to 0
static volatile union uuart_status {
    unsigned char status;
    struct {
        unsigned char ongoing_Transmission_From_Buffer:1;
        unsigned char ongoing_Transmission_Of_Package:1;
        unsigned char ongoing_Reception_Of_Package:1;
        unsigned char reception_Buffer_Overflow:1;
        unsigned char flag4:1;
        unsigned char flag5:1;
        unsigned char flag6:1;
        unsigned char flag7:1;
    };
} uuart_status = {0};

/** Usi Uart functions **/

/*
 * Note that when the USI DO pin is configured as output, it is always
 * connected to the USIDR and takes on the value of the MSB.  Thus, when the
 * USIDR is not configured as a transmitter, the DO pin has to be forced to
 * a logical high in order to conform to the UART standard high-state idle
 * condition.  This state is implemented by configuring the DO pin as input
 * and then applying the internal pull-up resistors.
 */
void uuart_init(void) {
	// force the DO pin to the UART idle state (high)
    USI_DDR  &= ~((1<<USI_DI_PIN)|(1<<USI_DO_PIN));	// configure both as input
    USI_OUTPUT |= (1<<USI_DI_PIN)|(1<<USI_DO_PIN);	// Enable pull ups on both
    uuart_flush_buffers();				// set buffers at their beginnings
}

/**
 * Reverses the order of bits in a byte. (i.e. MSB is swapped with LSB, etc.)
 * @param x the byte whose bits need to be reversed: unsigned char
 * @return a byte with bits reversed in the same variable: unsigned char
 */
unsigned char Bit_Reverse( unsigned char x ) {
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;    
}

/*
 * Flush the UART buffers.
 * @param void
 * @return void
 */
void uuart_flush_buffers( void ) {
    uuart_rx_tail = 0;
    uuart_rx_head = 0;
    uuart_tx_tail = 0;
    uuart_tx_head = 0;
}

/*
 * Initialise USI for UART transmission.
 * @param void
 * @return void
 */
void uuart_tx_init( void )
{
    cli();									// disable all interrupts

    TCNT0  = 0x00;
    TIFR  |= (1<<TOV0);                     // Clear Timer0 OVF interrupt flag
    TIMSK |= (1<<TOIE0);                    // Enable Timer0 OVF interrupt
	TCCR0B |= (PRESCALECMD); 				/* Set prescaler to /8, start Timer0
						CS00 and CS02 need to be zero (default at startup) */
	GTCCR |= (1<<PSR0);						// start prescaller at 0
    USICR  = (0<<USISIE)|(1<<USIOIE)|       // Enable USI Counter OVF interrupt.
             (0<<USIWM1)|(1<<USIWM0)|       // Select Three Wire mode.
             // Select Timer0 overflow as USI Clock source.
             (0<<USICS1)|(1<<USICS0)|(0<<USICLK)|
             (0<<USITC);                                           
             
    USISR  = 0xF0 |              // Clear all USI interrupt flags.
             0x0F;				 /* Preload the USI counter to generate
             	 	 	 	 	 	 	 	   interrupt at first USI clock */
    USIDR  = 0xFF;               // Make sure MSB is '1' before enabling USI_DO
    USI_DDR  |= (1<<USI_DO_PIN); // Configure USI_DO as output
                  
    uuart_status.ongoing_Transmission_From_Buffer = TRUE;
                  
    sei();								// enable all interrupts
}


/**
 * Puts data in the transmission buffer, after reversing the bits in the byte.
 * Initiates the transmission routines if not already started.
 * @param data a byte to be transmitted: unsigned char
 * @return void
 */
void uuart_tx_byte( unsigned char data ) {
    unsigned char tmphead;
    // Calculate buffer index and if necessary, roll over at upper bound
    tmphead = (uuart_tx_head + 1) & UART_TX_BUFFER_MASK;
    while ( tmphead == uuart_tx_tail );          // Wait for free space in buffer
    uuart_tx_buf[tmphead] = Bit_Reverse(data);   /* Reverse the order of the bits
                                  in the data byte and store data in buffer */
    uuart_tx_head = tmphead;                     // Store new index.
    // Start transmission from buffer (if not already started).
    if (!(uuart_status.ongoing_Transmission_From_Buffer)) {
        while (uuart_status.ongoing_Reception_Of_Package); /* Wait for USI
                                             to finish reading incoming data */
        uuart_tx_init();              
    }
}

void
uuart_tx_bytes (unsigned char *buf, unsigned char len)
{
    for (int i = 0; i < len; i++)
    {
        uuart_tx_byte(buf[i]);
    }
}

/**
 * Returns a byte from the receive buffer. Waits if buffer is empty.
 * @param void
 * @return data from the buffer: unsigned char
 */
unsigned char uuart_rx_byte( void ) {
    unsigned char tmptail;
        
    while ( uuart_rx_head == uuart_rx_tail );             // Wait for incoming data
    // Calculate buffer index and if necessary, roll over at upper bound
    tmptail = ( uuart_rx_tail + 1 ) & UART_RX_BUFFER_MASK;
    uuart_rx_tail = tmptail;                                // Store new index 
    return Bit_Reverse(uuart_rx_buf[tmptail]);              /* Reverse the order
    	of the bits in the data byte before it returns data from the buffer */
}


unsigned char
uuart_rx_bytes (unsigned char *buf, unsigned char len)
{

    unsigned char bytes_read = 0;

    while (bytes_read < len)
    {
        buf[bytes_read++] = uuart_rx_byte();
    }
    return bytes_read;
}

unsigned char
uuart_rx_bytes_until (unsigned char sep, unsigned char *buf, unsigned char len)
{
    unsigned char bytes_read = 0;
    unsigned char read_byte;

    while (bytes_read < len)
    {
        read_byte = uuart_rx_byte();
        if (read_byte == sep)
        {
            return bytes_read;
        }
        buf[bytes_read++] = read_byte;
    }

    return len;
}

/**
 * Check if there is data in the receive buffer.
 * @return  0 (FALSE) if the receive buffer is empty: unsigned char
 */
unsigned char uuart_rx_data_available( void ) {
    return ( uuart_rx_head != uuart_rx_tail );
}


// ********** Interrupt Handlers ********** //

/**
 * The pin change interrupt is used to detect USI_UART reception.
// It is here that the USI is configured to sample the UART signal.
 * @param PCINT0_vect
 */
ISR(PCINT0_vect) {
	/** The next code line is needed if more than one pin change interrupt
	 * is enabled in the PCINT0 group. If the USI DI pin is low, then it
	 * is likely this pin generated the pin change interrupt.  It sets up
	 * TIMER0 and the USI for the receive process.
	 */
    if (!(USI_INPUT & _BV(USI_DI_PIN) )) {  // USI_INPUT is PINA for ATtiny84
   /* Plant TIMER0 seed to match baudrate (including interrupt start up time)*/
        TCNT0  = INTERRUPT_STARTUP_DELAY + INITIAL_TIMER0_SEED;

        TCCR0B  |= (PRESCALECMD); 		/* Set prescaler to /8, and start Timer0
                    	CS00 and CS02 are set to zero at startup by default */
        GTCCR |= (1<<PSR0);					// start prescaller at zero
        TIFR  |= (1<<TOV0);                 // Clear Timer0 OVF interrupt flag
        TIMSK |= (1<<TOIE0);                // Enable Timer0 OVF interrupt
                                                                    
        USICR = (0<<USISIE)|(1<<USIOIE)|   // Enable USI Counter OVF interrupt
                 (0<<USIWM1)|(1<<USIWM0)|            // Select Three Wire mode.
                 (0<<USICS1)|(1<<USICS0)|(0<<USICLK)| // Select Timer0 OVR
                 (0<<USITC);                  //          as USI Clock source.
       // Note that enabling the USI will also disable the pin change interrupt.
        USISR  = 0xF0 |                       // Clear all USI interrupt flags.
                   USI_COUNTER_SEED_RECEIVE;  /* Preload the USI counter to
                   the number of bits to be shifted out before an interrupt */
        GIMSK &=  ~(1<<PCIE);   			// Disable pin change interrupts
        
        uuart_status.ongoing_Reception_Of_Package = TRUE;             
    }
}

/**
 * The USI Counter Overflow interrupt is used for moving data between memory
 * and the USI data register. The interrupt is used for both transmission
 * and reception; hence, its complexity.
 * @param USI_OVF_vect
 */
ISR(USI_OVF_vect) {
    unsigned char tmphead,tmptail;
    
    // Check if we are running in Transmit mode.
    if( uuart_status.ongoing_Transmission_From_Buffer ) {
        // If ongoing transmission, then send second half of transmit data.
        if( uuart_status.ongoing_Transmission_Of_Package ) {
        	// Clear on-going package transmission flag.
            uuart_status.ongoing_Transmission_Of_Package = FALSE;
            // Load USI Counter seed and clear all USI flags.
            USISR = 0xF0 | (USI_COUNTER_SEED_TRANSMIT);
            // Reload the USIDR with the rest of the data and a stop-bit.
            USIDR = (uuart_tx_data << 3) | 0x07;
        }
        // Else start sending more data or leave transmit mode.
        else {
       // If there is data in the transmit buffer, then send first half of data
            if ( uuart_tx_head != uuart_tx_tail )  {
            	// Set on-going package transmission flag.
                uuart_status.ongoing_Transmission_Of_Package = TRUE;
                // Calculate buffer index and if necessary, roll over.
                tmptail = ( uuart_tx_tail + 1 ) & UART_TX_BUFFER_MASK;
                uuart_tx_tail = tmptail;                       // Store new index
                /* Read out the data that is to be sent. Note that the data must
                   be bit reversed before sent. The bit reversing is moved to
                   the application section to save time within the interrupt */
                uuart_tx_data = uuart_tx_buf[tmptail];
                /* Load USI Counter seed and clear all USI flags */
                USISR  = 0xF0 | (USI_COUNTER_SEED_TRANSMIT);
                /* Copy (initial high state,) start-bit and 6 LSB of original
                 *                 data (6 MSB of bit of bit reversed data). */
                USIDR  = (uuart_tx_data >> 2) | 0x80;
            }
            // Else enter receive mode.
            else {
            	uuart_status.ongoing_Transmission_From_Buffer = FALSE;
                TCCR0B  &= ~(PRESCALECMD);			// Stop Timer0
                USI_DDR &= ~(1 << USI_DO_PIN);			// config DO pin as input
                USI_OUTPUT |= (1 << USI_DO_PIN);	// Enable pull up on USI DI
            //   USI_DDR  &= ~(1<<USI_DI_PIN);		// config DI pin as input
                USICR  =  0;                        // Disable USI
                GIFR   |=  (1<<PCIF);        // Clear pin change interrupt flag
                GIMSK |=  (1<<PCIE);   	// Enable all pin change interrupts
 			    PCMSK |=  (1<<PCINT6);	 // Enable pin change interrupt for PA6
           }
        }
    }
    
    // Else running in receive mode.
    else {
        uuart_status.ongoing_Reception_Of_Package = FALSE;
        //Calculate buffer index and if necessary, roll over at upper bound.
        tmphead     = ( uuart_rx_head + 1 ) & UART_RX_BUFFER_MASK;
        // If buffer is full trash data and set buffer full flag.
        if ( tmphead == uuart_rx_tail ) {
        	// Store status to take actions elsewhere in the application code
            uuart_status.reception_Buffer_Overflow = TRUE;
        }
        else {          // If there is space in the buffer then store the data.
            uuart_rx_head = tmphead;                          // Store new index.
            /* Store received data in buffer. Note that the data must be bit
             * reversed before used.  The bit reversing is moved to the
             * application section to save time within the interrupt. */
            uuart_rx_buf[tmphead] = USIDR;
//            uuart_rx_buf(tmphead) = USIRB; // ? use the buffered USI register?
        }
        TCCR0B  &= ~(PRESCALECMD);  			// Stop Timer0.
    //    USI_DDR  &= ~(1<<USI_DI_PIN);			// Set DI pin as input
    //    USI_OUTPUT |=  (1<<USI_DI_PIN);		// Enable pull up on USI DI
    //     USI_DDR  |= (1<<USI_DO_PIN);			// Set DO pin as output
        USI_DDR &= ~(1 << USI_DO_PIN);			// config DO pin as input
        USI_OUTPUT |= (1 << USI_DO_PIN);	// Enable pull up on USI DI
        USICR  =  0;                     		// Disable USI.
        GIFR  |=  (1<<PCIF);           // Clear pin change interrupt flag.
        GIMSK |=  (1<<PCIE);		   // Enable pin change interrupt
	    PCMSK |=  (1<<PCINT6);		   // Enable pin change interrupt for PA6
     }
    
}

/**
 * Timer0 Overflow interrupt is used to trigger the sampling of signals on
 * the USI Input pin - hopefully at midpoint in the bit period.
 * @param TIM0_OVF_vect
 */
ISR(TIM0_OVF_vect) {
    TCNT0 += TIMER0_SEED;    /* Reload the timer, current count is added for
    							timing correction */
}

/**
 * Send a string of characters
 * @param str - pointer to the string location: i.e. the array's name
 */
void uuart_print(char *str) {
	uint8_t i = 0;
	while (str[i]) {		//Null char looks like a FALSE indication
		uuart_tx_byte(str[i++]);
	}
    // uuart_tx_byte('\n');
}

void uuart_showbits(int byte) {
	char buf[17];
	itoa(byte, buf, 2);
	uuart_print("0b");
	uuart_print(buf);
}

void uuart_showhex(int byte) {
	char buf[17];
	itoa(byte, buf, 16);
	uuart_print("0x");
	uuart_print(buf);
}
