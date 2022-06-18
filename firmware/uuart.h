

/** USI UART Config **/

#define SYSTEM_CLOCK		F_CPU
#define TIMER_PRESCALER		8
#define PRESCALECMD			(1<<CS01)

#define BAUDRATE		9600

// Buffer size must be a power of 2
#define UART_RX_BUFFER_SIZE     4
#define UART_TX_BUFFER_SIZE     4

/** USI UART Functions **/

void		uuart_init(void);
void		uuart_flush_buffers(void);
void		uuart_tx_init(void);

void			uuart_tx_byte(unsigned char);
void			uuart_tx_bytes(unsigned char *buf, unsigned char len);
unsigned char	uuart_rx_byte(void);
unsigned char   uuart_rx_bytes(unsigned char *buf, unsigned char len);
unsigned char   uuart_rx_bytes_until(unsigned char sep, unsigned char *buf, unsigned char len);

unsigned char	uuart_rx_data_available(void);

void		  uuart_print(char *str);		// transmit a string
void 		  uuart_showbits(int byte);	// show binary value
void		  uuart_showhex(int byte);

/** Chip Specific Defines **/
// #ifdef __AVR_ATtiny44__
	#define TIFR		TIFR0
	#define TIMSK		TIMSK0
	#define PCIF		PCIF0
	#define PCIE		PCIE0
	#define PCMSK		PCMSK0
	#define	PSR0		PSR10
	#define USI_DDR		DDRA
	#define USI_OUTPUT	PORTA
	#define USI_INPUT	PINA
	#define USI_DI_PIN	PA6
	#define USI_DO_PIN	PA5
// #endif

/** (Mostly) Static Defines **/
#define TRUE 1
#define FALSE 0

#define DATA_BITS                 8
#define START_BIT                 1
#define STOP_BIT                  1
#define HALF_FRAME                5

#define USI_COUNTER_MAX_COUNT     16
#define USI_COUNTER_SEED_TRANSMIT (USI_COUNTER_MAX_COUNT - HALF_FRAME)
#define INTERRUPT_STARTUP_DELAY   (0x11 / TIMER_PRESCALER)
#define TIMER0_SEED               (256 - ( (SYSTEM_CLOCK / BAUDRATE) / TIMER_PRESCALER )) // has round off error
// #define TIMER0_SEED               (256 - ( (SYSTEM_CLOCK / BAUDRATE) / TIMER_PRESCALER ))-2

#if ( (( (SYSTEM_CLOCK / BAUDRATE) / TIMER_PRESCALER ) * 3/2) > (256 - INTERRUPT_STARTUP_DELAY) )
    #define INITIAL_TIMER0_SEED       ( 256 - (( (SYSTEM_CLOCK / BAUDRATE) / TIMER_PRESCALER ) * 1/2) )
    #define USI_COUNTER_SEED_RECEIVE  ( USI_COUNTER_MAX_COUNT - (START_BIT + DATA_BITS) )
#else
    #define INITIAL_TIMER0_SEED       ( 256 - (( (SYSTEM_CLOCK / BAUDRATE) / TIMER_PRESCALER ) * 3/2) )
    #define USI_COUNTER_SEED_RECEIVE  (USI_COUNTER_MAX_COUNT - DATA_BITS)
#endif

#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1 )
#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK )
    #error RX buffer size is not a power of 2
#endif

#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1 )
#if ( UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK )
    #error TX buffer size is not a power of 2
#endif

