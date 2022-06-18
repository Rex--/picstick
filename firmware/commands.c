#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#include "uuart.h"
#include "icsp.h"

#include "commands.h"


unsigned char input_buffer[INPUT_BUFFER_SIZE];
int recv_size;


// Status Flags
#define STATUS_DISCONNECTED 0
#define STATUS_CONNECTED 1
#define STATUS_PROGRAM 2

unsigned char cmd_is(const char *cmd)
{
    if (memcmp(input_buffer, cmd, strlen(cmd)) == 0) {
        return 1;
    }
    return 0;
}

void cmd_resp(const char *resp)
{
    uuart_print((char *)resp);
    uuart_tx_byte(SERIAL_CMD_SEP);
}

void cmd_resp_error(unsigned char *msg, unsigned char msg_len)
{
    uuart_print(SERIAL_CMD_ERROR);
    uuart_tx_byte(SERIAL_CMD_SEP);

    for (int i=0; i < msg_len; i++)
    {
        uuart_tx_byte(msg[i]);
    }
}

unsigned char cmd_hello(void)
{
    cmd_resp(SERIAL_CMD_HELLO);
    return STATUS_CONNECTED;
}

unsigned char cmd_bye(void)
{
    cmd_resp(SERIAL_CMD_BYE);
    return STATUS_DISCONNECTED;
}

unsigned char cmd_start(void)
{
    icsp_enable();
    cmd_resp(SERIAL_CMD_OK);
    return STATUS_PROGRAM;
}

unsigned char cmd_stop(void)
{
    icsp_disable();
    cmd_resp(SERIAL_CMD_OK);
    return STATUS_CONNECTED;
}

unsigned char cmd_addr(void)
{
    uuart_rx_bytes(input_buffer, 2);
    unsigned int address = (input_buffer[0] << 8) | input_buffer[1];
    icsp_command(ICSP_CMD_ADDR_LOAD);
    _delay_us(ICSP_DELAY_DLY);
    icsp_payload(address);
    cmd_resp(SERIAL_CMD_OK);
    return STATUS_PROGRAM;
}

unsigned char cmd_word(void)
{
    uuart_rx_bytes(input_buffer, 5);
    if (input_buffer[2] != ':') {
        cmd_resp_error(input_buffer, 5);
        return STATUS_PROGRAM;
    }
    unsigned int address = (input_buffer[0] << 8) | input_buffer[1];
    unsigned int word = (input_buffer[3] << 8) | input_buffer[4];

    // Load address
    icsp_command(ICSP_CMD_ADDR_LOAD);
    _delay_us(ICSP_DELAY_DLY);
    icsp_payload(address);
    _delay_us(ICSP_DELAY_DLY);

    // Write word
    icsp_command(ICSP_CMD_LOAD_DATA);
    _delay_us(ICSP_DELAY_DLY);
    icsp_payload(word);
    _delay_us(ICSP_DELAY_DLY);

    // Begin write command
    icsp_command(ICSP_CMD_START_INT);
    _delay_us(ICSP_DELAY_PINT_CW);

    // verify data

    // Return response
    cmd_resp(SERIAL_CMD_OK);

    return STATUS_PROGRAM;
}

unsigned char cmd_row(void)
{
    // Get address
    recv_size = uuart_rx_bytes_until(SERIAL_CMD_SEP, input_buffer, INPUT_BUFFER_SIZE);
    if (recv_size != 2) {
        cmd_resp_error(input_buffer, recv_size);
        return STATUS_PROGRAM;
    }
    unsigned int address = (input_buffer[0] << 8) | input_buffer[1];
    
    // Get row
    recv_size = uuart_rx_bytes(input_buffer, 128);
    if (recv_size != 128) {
        cmd_resp_error(input_buffer, recv_size);
        return STATUS_PROGRAM;
    }

    // Load address
    icsp_command(ICSP_CMD_ADDR_LOAD);
    _delay_us(ICSP_DELAY_DLY);
    icsp_payload(address);
    _delay_us(ICSP_DELAY_DLY);

    // Write 63 words of data
    unsigned int word;
    unsigned char offset;
    for (offset=0; offset < 126; offset += 2) {
        // Write word
        word = (input_buffer[offset] << 8) | input_buffer[offset+1];
        icsp_command(ICSP_CMD_LOAD_DATA_INC);
        _delay_us(ICSP_DELAY_DLY);
        icsp_payload(word);
        _delay_us(ICSP_DELAY_DLY);
    }

    //Write 64th word without incrementing address
    word = (input_buffer[offset] << 8) | input_buffer[offset+1];
    icsp_command(ICSP_CMD_LOAD_DATA);
    _delay_us(ICSP_DELAY_DLY);
    icsp_payload(word);
    _delay_us(ICSP_DELAY_DLY);


    // Begin write command
    icsp_command(ICSP_CMD_START_INT);
    _delay_us(ICSP_DELAY_PINT_PM);

    // verify data

    // Return response
    cmd_resp(SERIAL_CMD_OK);

    return STATUS_PROGRAM;
}

unsigned char cmd_erase(void)
{
    recv_size = uuart_rx_bytes(input_buffer, 2);
    if (recv_size != 2) {
        cmd_resp_error(input_buffer, recv_size);
    }

    unsigned int address = (input_buffer[0] << 8) | input_buffer[1];
    unsigned char cmd = ICSP_CMD_ERASE_ROW;
    unsigned int delay_len = ICSP_DELAY_ERAR;

    // Bulk erase the whole device
    if (address == SERIAL_CMD_ERASE_ALL) {
        address = 0x8000;
        cmd = ICSP_CMD_ERASE_BULK;
        delay_len = ICSP_DELAY_ERAB;
        
    }
    // Bulk erase user flash
    else if (address == SERIAL_CMD_ERASE_FLASH) {
        address = 0x0000;
        cmd = ICSP_CMD_ERASE_BULK;
        delay_len = ICSP_DELAY_ERAB;
    }

    icsp_command(ICSP_CMD_ADDR_LOAD);
    _delay_us(ICSP_DELAY_DLY);
    icsp_payload(address);
    _delay_us(ICSP_DELAY_DLY);
    icsp_command(cmd);
    if (delay_len == ICSP_DELAY_ERAB)
    {
        _delay_us(ICSP_DELAY_ERAB);
    }
    else
    {
        _delay_us(ICSP_DELAY_ERAR);
    }

    cmd_resp(SERIAL_CMD_OK);
    return STATUS_PROGRAM;
}

unsigned char cmd_read(void)
{
    recv_size = uuart_rx_bytes(input_buffer, 2);
    if (recv_size != 2) {
        cmd_resp_error(input_buffer, recv_size);
    }

    unsigned int address = (input_buffer[0] << 8) | input_buffer[1];

    icsp_command(ICSP_CMD_ADDR_LOAD);
    _delay_us(ICSP_DELAY_DLY);
    icsp_payload(address);
    _delay_us(ICSP_DELAY_DLY);
    icsp_command(ICSP_CMD_READ_DATA);
    _delay_us(ICSP_DELAY_DLY);
    unsigned int word = icsp_read();

    // Return response
    cmd_resp(SERIAL_CMD_OK);
    uuart_tx_byte((word >> 8));
    uuart_tx_byte(word & 0xFF);
    return STATUS_PROGRAM;
}


unsigned char handle_command(void)
{
    recv_size = uuart_rx_bytes_until(':', input_buffer, INPUT_BUFFER_SIZE);

    // Greeting
    if (cmd_is(SERIAL_CMD_HELLO))
        return cmd_hello();

    else if (cmd_is(SERIAL_CMD_BYE))
        return cmd_bye();

    else if (cmd_is(SERIAL_CMD_START))
        return cmd_start();

    else if (cmd_is(SERIAL_CMD_STOP))
        return cmd_stop();

    // else if (cmd_is(SERIAL_CMD_ADDR)) This doesnt really need to be a command
    //     return cmd_addr();        since we specify address in all the others    
    
    else if (cmd_is(SERIAL_CMD_WORD))
        return cmd_word();
    
    else if (cmd_is(SERIAL_CMD_ROW))
        return cmd_row();
    
    else if (cmd_is(SERIAL_CMD_ERASE))
        return cmd_erase();
    
    else if (cmd_is(SERIAL_CMD_READ))
        return cmd_read();

    uuart_print("UNKOWN:");
    uuart_tx_bytes(input_buffer, recv_size);
    return 0;
}