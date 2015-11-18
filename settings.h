#ifndef APM_SERIAL_LOG_SETTINGS_H_INCLUDED
#define APM_SERIAL_LOG_SETTINGS_H_INCLUDED

#include "arduino_compat.h"

#define MODE_NEWLOG	0
#define MODE_SEQLOG     1
#define MODE_COMMAND    2

#define ECHO			0x01
#define EXTENDED_INFO		0x02
#define OFF  0x00
#define ON   0x01

#define BAUD_MIN  300
#define BAUD_MAX  1000000

#define LOCATION_SYSTEM_SETTING		0x02
#define LOCATION_FILE_NUMBER_LSB	0x03
#define LOCATION_FILE_NUMBER_MSB	0x04
#define LOCATION_ESCAPE_CHAR		0x05
#define LOCATION_MAX_ESCAPE_CHAR	0x06
#define LOCATION_VERBOSE                0x07
#define LOCATION_ECHO                   0x08
#define LOCATION_BAUD_SETTING_HIGH	0x09
#define LOCATION_BAUD_SETTING_MID	0x0A
#define LOCATION_BAUD_SETTING_LOW	0x0B
#define LOCATION_IGNORE_RX		0x0C

extern byte feedback_mode; 
extern long setting_uart_speed; //This is the baud rate that the system runs at, default is 9600. Can be 1,200 to 1,000,000
extern byte setting_system_mode; //This is the mode the system runs in, default is MODE_NEWLOG
extern byte setting_escape_character; //This is the ASCII character we look for to break logging, default is ctrl+z
extern byte setting_max_escape_character; //Number of escape chars before break logging, default is 3
extern byte setting_verbose; //This controls the whether we get extended or simple responses.
extern byte setting_echo; //This turns on/off echoing at the command prompt
extern byte setting_ignore_RX; //This flag, when set to 1 will make OpenLog ignore the state of the RX pin when powering up

#endif // APM_SERIAL_LOG_SETTINGS_H_INCLUDED
