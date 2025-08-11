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
cmd_start (void)
{
    icsp_enable();
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

    uuart_tx_byte(RESP_OK);
}

void
cmd_unknown (void)
{
    uuart_tx_byte(RESP_UNK);
}


void
cmd_read (void)
{
    // Get command
    unsigned char cmd = uuart_rx_byte();

    // Send command
    icsp_command(cmd);

    // Read response payload
    unsigned long resp = icsp_read();

    uuart_tx_byte((resp >> 16) & 0xFF);
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

        case CMD_START:
            cmd_start();
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

        default:
            cmd_unknown();
        break;
    }
}