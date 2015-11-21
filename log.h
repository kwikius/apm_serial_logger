#ifndef APM_LOG_LOG_H_INCLUDED
#define APM_LOG_LOG_H_INCLUDED

#include <SdFat.h> //We do not use the built-in SD.h file because it calls Serial.print

int freeRam();
void blink_error(unsigned char ERROR_TYPE);
void read_system_settings(void);
void read_config_file(void);
void check_emergency_reset(void);
char* newlog(void);
//unsigned char append_file(const char* file_name);
//void seqlog(void);
//void command_shell(void);
void toggleLED(unsigned char pinNumber);
//void check_emergency_reset(void);
//void set_default_settings(void);
//void writeBaud(long uartRate);
//long readBaud(void);
void systemError(byte error_type);
void record_config_file(void);
uint32_t strtolong(const char* str);
unsigned char read_line(char* buffer, unsigned char buffer_length);
char* get_cmd_arg(unsigned char index);
bool print_menu(void);
//void baud_menu(void);
void system_menu(void);
unsigned char count_cmd_args(void);
bool wildcmp(const char* wild, const char* string);
bool gotoDir(const char *dir);
bool is_number(const char* buffer, unsigned char buffer_length);
unsigned char read_line(char* buffer, unsigned char buffer_length);
unsigned char split_cmd_line_args(const char* buffer, unsigned char buffer_length);
void lsPrint(SdFile * theDir, const char * cmdStr, unsigned char flags, unsigned char indent);
unsigned char lsPrintNext(SdFile * theDir, const char * cmdStr, unsigned char flags, unsigned char indent);
signed char getNextFolderTreeIndex();
#endif // APM_LOG_LOG_H_INCLUDED
