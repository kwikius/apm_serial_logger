#ifndef APM_LOG_LOG_H_INCLUDED
#define APM_LOG_LOG_H_INCLUDED

#include <SdFat.h> //We do not use the built-in SD.h file because it calls Serial.print
#include <SerialPort.h> //This is a new/beta library written by Bill Greiman. You rock Bill!
#include <EEPROM.h>


void blink_error(unsigned char ERROR_TYPE);
void read_system_settings(void);
void read_config_file(void);
void check_emergency_reset(void);
char* newlog(void);
unsigned char append_file(char* file_name);
void seqlog(void);
void command_shell(void);
void toggleLED(unsigned char pinNumber);
void check_emergency_reset(void);
void set_default_settings(void);
void writeBaud(long uartRate);
long readBaud(void);
void record_config_file(void);
uint32_t strtolong(const char* str);
unsigned char read_line(char* buffer, unsigned char buffer_length);
char* get_cmd_arg(unsigned char index);
void print_menu(void);
void baud_menu(void);
void system_menu(void);
unsigned char count_cmd_args(void);
unsigned char wildcmp(const char* wild, const char* string);
unsigned char gotoDir(char *dir);
char* is_number(char* buffer, unsigned char buffer_length);
unsigned char read_line(char* buffer, unsigned char buffer_length);
unsigned char split_cmd_line_args(char* buffer, unsigned char buffer_length);
void lsPrint(SdFile * theDir, char * cmdStr, unsigned char flags, unsigned char indent);
unsigned char lsPrintNext(SdFile * theDir, char * cmdStr, unsigned char flags, unsigned char indent);
signed char getNextFolderTreeIndex();
void printRam() ;
void systemError(byte error_type);

typedef SerialPort<0, 250, 0> serial_port_t;
extern  serial_port_t NewSerial;

extern Sd2Card card;
extern SdVolume volume;
extern  SdFile currentDirectory;

extern  long setting_uart_speed; //This is the baud rate that the system runs at, default is 9600. Can be 1,200 to 1,000,000
extern  byte setting_system_mode; //This is the mode the system runs in, default is MODE_NEWLOG
extern  byte setting_escape_character; //This is the ASCII character we look for to break logging, default is ctrl+z
extern  byte setting_max_escape_character; //Number of escape chars before break logging, default is 3
extern  byte setting_verbose; //This controls the whether we get extended or simple responses.
extern  byte setting_echo; //This turns on/off echoing at the command prompt
extern  byte setting_ignore_RX; //This flag, when set to 1 will make OpenLog ignore the state of the RX pin when powering up

#define FOLDER_TRACK_DEPTH 2 //Decreased for more RAM access
extern  char folderTree[FOLDER_TRACK_DEPTH][12];
#endif // APM_LOG_LOG_H_INCLUDED
