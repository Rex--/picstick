/** @file commands.h
 *
 * Handle serial commands from the host and convert them into ICSP commands.
 * 
*/

#ifndef _commands_h_
#define _commands_h_

// #define CMD_SEP     ':'
// #define CMD_END     ';'
#define CMD_PING    'I' // Ping command
#define CMD_START   'S' // Start programming mode
#define CMD_QUIT    'Q' // Quit programming mode
#define CMD_COMMAND 'C' // Send command
#define CMD_PAYLOAD 'P' // Send command with payload
#define CMD_READ    'R' // Send command and read response

#define RESP_PONG   'O'
#define RESP_OK     'K' // Command executed successfully
// #define RESP_ERR    'E' // Command encountered an error
#define RESP_UNK    '?' // Command unknown

void handle_command(void);

#endif
