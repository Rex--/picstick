/** @file commands.c
 * 
 * Handle serial commands from the host and convert them into ICSP commands.
 * 
*/

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#include "uuart.h"
#include "icsp.h"

#include "commands.h"


void
cmd_ping (void)
{
    uuart_tx_byte(RESP_PONG);
}


void
cmd_unknown (void)
{
    uuart_tx_byte(RESP_UNK);
}


void
cmd_start_msb (void)
{
    icsp_enable_msb();
    uuart_tx_byte(RESP_OK);
}

void
cmd_start_lsb (void)
{
    icsp_enable_lsb();
    uuart_tx_byte(RESP_OK);
}


void
cmd_quit (void)
{
    icsp_disable();
    uuart_tx_byte(RESP_OK);
}


void
cmd_command (void)
{
    icsp_command(uuart_rx_byte());

    // Wait 100ms for command to finish executing
    _delay_ms(10);

    uuart_tx_byte(RESP_OK);
}


void
cmd_payload (void)
{
    // Get command
    unsigned char cmd = uuart_rx_byte();

    // Get payload
    unsigned long payload = (unsigned long)uuart_rx_byte() << 16;
    payload |= (unsigned long)uuart_rx_byte() << 8;
    payload |= (unsigned long)uuart_rx_byte();

    // Send command
    icsp_command(cmd);

    // Send payload
    icsp_payload(payload);

    // Wait 100ms for command to finish executing
    _delay_ms(10);

    uuart_tx_byte(RESP_OK);
}


void
cmd_read (void)
{
    // Get command
    unsigned char cmd = uuart_rx_byte();

    // Send command
    icsp_command(cmd);


    // Wait 10ms for command to finish executing
    _delay_ms(10);

    // Read response payload
    unsigned long resp = icsp_read();

    uuart_tx_byte((resp >> 16) & 0xFF);
    uuart_tx_byte((resp >> 8) & 0xFF);
    uuart_tx_byte(resp & 0xFF);
}


void
cmd_short_command (void)
{
    icsp_short_command(uuart_rx_byte());


    // Wait 10ms for command to finish executing
    _delay_ms(10);

    uuart_tx_byte(RESP_OK);
}


void
cmd_short_payload (void)
{
    // Get command
    unsigned char cmd = uuart_rx_byte();

    // Get payload
    uuart_rx_byte();
    unsigned int payload = (unsigned int)uuart_rx_byte() << 8;
    payload |= (unsigned int)uuart_rx_byte();

    // Send command
    icsp_short_command(cmd);

    // Wait 10ms for command to finish executing
    _delay_ms(10);

    // Send payload
    icsp_short_payload(payload);

    // Wait 10ms for command to finish executing
    _delay_ms(10);

    uuart_tx_byte(RESP_OK);
}


void
cmd_short_read (void)
{
    // Get command
    unsigned char cmd = uuart_rx_byte();

    // Send command
    icsp_short_command(cmd);

    // Wait 10ms for command to finish executing
    _delay_ms(10);

    // Read response payload
    unsigned int resp = icsp_short_read();

    uuart_tx_byte(0);
    uuart_tx_byte((resp >> 8) & 0xFF);
    uuart_tx_byte(resp & 0xFF);
}



void
handle_command (void)
{
    unsigned char cmd = uuart_rx_byte();

    switch (cmd)
    {
        case CMD_PING:
            cmd_ping();
        break;

        case CMD_START_MSB:
            cmd_start_msb();
        break;

        case CMD_START_LSB:
            cmd_start_lsb();
        break;

        case CMD_QUIT:
            cmd_quit();
        break;


        case CMD_COMMAND:
            cmd_command();
        break;

        case CMD_PAYLOAD:
            cmd_payload();
        break;

        case CMD_READ:
            cmd_read();
        break;


        case CMD_SHORT_COMMAND:
            cmd_short_command();
        break;

        case CMD_SHORT_PAYLOAD:
            cmd_short_payload();
        break;

        case CMD_SHORT_READ:
            cmd_short_read();
        break;


        default:
            cmd_unknown();
        break;
    }
}