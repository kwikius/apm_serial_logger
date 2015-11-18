#ifndef APM_SERIAL_LOG_SETTINGS_H_INCLUDED
#define APM_SERIAL_LOG_SETTINGS_H_INCLUDED

#include "arduino_compat.h"

extern long setting_uart_speed; //This is the baud rate that the system runs at, default is 9600. Can be 1,200 to 1,000,000
extern byte setting_system_mode; //This is the mode the system runs in, default is MODE_NEWLOG
extern byte setting_escape_character; //This is the ASCII character we look for to break logging, default is ctrl+z
extern byte setting_max_escape_character; //Number of escape chars before break logging, default is 3
extern byte setting_verbose; //This controls the whether we get extended or simple responses.
extern byte setting_echo; //This turns on/off echoing at the command prompt
extern byte setting_ignore_RX; //This flag, when set to 1 will make OpenLog ignore the state of the RX pin when powering up

#endif // APM_SERIAL_LOG_SETTINGS_H_INCLUDED
