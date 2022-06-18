/** @file icsp.c
 * 
 * This part of the firmware implements the ICSP interface by bitbanging some
 * GPIO pins.
 * 
*/

#ifndef _icsp_h_
#define _icsp_h_

// ICSP Pin Configuration
#define ICSP_PIN_MCLR   (1 << 0)
#define ICSP_PIN_CLK    (1 << 1)
#define ICSP_PIN_DAT    (1 << 2)
#define ICSP_PORT       PORTA
#define ICSP_DDR        DDRA
#define ICSP_PIN        PINA

// ICSP Commands
#define ICSP_CMD_ADDR_LOAD 0x80
#define ICSP_CMD_ERASE_BULK 0x18
#define ICSP_CMD_ERASE_ROW 0xF0
#define ICSP_CMD_LOAD_DATA_INC 0x02
#define ICSP_CMD_LOAD_DATA 0x00
#define ICSP_CMD_READ_DATA_INC 0xFE
#define ICSP_CMD_READ_DATA 0xFC
#define ICSP_CMD_ADDR_INC 0xF8
#define ICSP_CMD_START_INT 0xE0
#define ICSP_CMD_START_EXT 0xC0
#define ICSP_CMD_STOP_EXT 0x82

// Startup bit sequence to enter programming mode is cleverly MCHIP in ascii.=
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



void        icsp_init (void);

/** Enter the connected chip into ICSP programming mode. */
void        icsp_enable (void);

/** Exit the connected chip from ICSP programming mode. */
void        icsp_disable (void);

/** Send ICSP command to the connected chip. */
void        icsp_command (unsigned char command);

/** Send a data payload to the connected chip. */
void        icsp_payload (unsigned int payload);

/** Read an incoming data payload from the connected chip. */
unsigned int icsp_read (void);

#endif
