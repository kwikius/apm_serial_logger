/*
 12-3-09
 Nathan Seidle
 SparkFun Electronics 2012
 
 OpenLog hardware and firmware are released under the Creative Commons Share Alike v3.0 license.
 http://creativecommons.org/licenses/by-sa/3.0/
 Feel free to use, distribute, and sell varients of OpenLog. 
All we ask is that you include attribution of 'Based on OpenLog by SparkFun'.
 
 */
#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers
#include <EEPROM.h>
#include "log.h"
#include "serial_port.h"
#include "error_code.hpp"
#include <quan/min.hpp>
#define DEBUG  0
//#define RAM_TESTING  1 //On
#define RAM_TESTING  0 //Off
// In order for compiler to optimise calls and save rom 
// these are included inline rather than linked
#include "system_error.cpp"
#include "serial_port.cpp"
#include "free_ram.cpp"

namespace {
   
   //#define Reset_AVR() wdt_enable(WDTO_1S); while(1) {} //Correct way of resetting the ATmega, but doesn't work with 
   //Arduino pre-Optiboot bootloader
   void(* Reset_AVR) (void) = 0; //Dirty way of resetting the ATmega, but it works for now

   struct setting{
      static constexpr long      baud_rate = 9600; //This is the baud rate that the system runs at, default is 9600. Can be 1,200 to 1,000,000
      static constexpr byte      escape_character = 0x1A; //This is the ASCII character we look for to break logging, default is ctrl+z
      static constexpr byte      num_escapes = 3; //Number of escape chars before break logging, default is 3, dont make 0 atm
      static constexpr uint8_t   max_commandline_args = 5;
      static constexpr uint8_t   max_folder_depth = 2;
      static constexpr uint16_t  max_file_string_len = 13;    
      static constexpr uint8_t   subdir_indent = 2;
      static constexpr uint8_t   echo = 0x01;
      static constexpr uint8_t   extended_info = 0x02;
      static constexpr uint8_t   end_marker = 0x08;
   };

   //Internal EEPROM locations
   struct eeprom_loc{
     static constexpr uint16_t new_file_idx = 0x03; // 2 bytes
   };

   struct port_pin{
     static constexpr uint8_t low = LOW;
     static constexpr uint8_t high = HIGH;
     typedef decltype (_SFR_IO8(0)) port_type;
     port_pin(port_type port_in, uint8_t pin_in): port{port_in}, pin{pin_in}{}
     port_type port;
     const uint8_t pin; // think this is the arduino pin number
   };

   port_pin statled1{PORTD,5};
   port_pin statled2{PORTB,13};

   Sd2Card card;
   SdVolume volume;
   SdFile currentDirectory;
   SdFile file;

   static char folderTree[setting::max_folder_depth][12];
   //Used for wild card delete and search
   struct command_arg_t {
     const char* arg; //Points to first character in command line argument
     byte arg_length; //Length of command line argument
   };

   command_arg_t cmd_arg[setting::max_commandline_args];
   constexpr byte feedback_mode = (setting::echo | setting::extended_info);

void setup_sd_and_fat()
{
  //Setup SD & FAT
  if (!card.init(SPI_FULL_SPEED)) {
      systemError(error_code::card_init);
  }
  if (!volume.init(&card)) {
   systemError(error_code::volume_init);
  }
  currentDirectory.close(); //We close the cD before opening root. This comes from QuickStart example. Saves 4 bytes.
  if (!currentDirectory.openRoot(&volume)) {
      systemError(error_code::root_init);
  }
}

 void command_shell();

} // namespace 

#include "setup.cpp"

void loop(void)
{
  command_shell();
 // while(1); //We should never get this far
}

namespace {

void power_saving_mode()
{
   digitalWrite(statled1.pin, port_pin::low);

   power_timer0_disable(); //Shut down peripherals we don't need
   power_spi_disable();
   sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received

   power_spi_enable(); //After wake up, power up peripherals
   power_timer0_enable();

}

#include "write_text_file.cpp"

bool command_init()
{
   currentDirectory.close();
   //Open the root directory
   if (!currentDirectory.openRoot(&volume)) {
      systemError(error_code::root_init);
   }
   memset(folderTree, 0, sizeof(folderTree)); //Clear folder tree
   return true;
}

bool command_ls()
{
   char * command_arg = nullptr;
   if (count_cmd_args() == 1)  {
     // Don't use the 'ls()' method in the SdFat library as it does not
     // limit recursion into subdirectories.
     command_arg = get_cmd_arg(0); // reuse the command name
     command_arg[0] = '*';       // use global wildcard
     command_arg[1] = '\0';
   }else{
     command_arg = get_cmd_arg(1);
     strupr(command_arg);
   }
   lsPrint(&currentDirectory, command_arg, LS_SIZE | LS_R, 0);
   return true;
}

bool command_md()
{
   //Argument 2: Directory name
   char * command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return false;
   }
   SdFile newDirectory;
   if (!newDirectory.makeDir(&currentDirectory, command_arg)) {
     if ((feedback_mode & setting::extended_info) > 0){
       NewSerial.print(F("error creating directory: "));
       NewSerial.println(command_arg);
     }
     return false;
   }else{
     return true;
   }
}

} // ~namespace

#include "packet_mode.cpp"

namespace {

#include "command_rm.cpp"
#include "command_read.cpp"
#include "command_write.cpp"
#include "command_size.cpp"
#include "command_cd.cpp"
#include "command_disk.cpp"
#include "command_pwd.cpp"

void command_shell(void)
{
  char buffer[30];
  bool command_succeeded = true;
#if RAM_TESTING == 1
  printRam(); //Print the available RAM
#endif
  while(true){

    if (command_succeeded == 0){
      NewSerial.print(F("!"));
    }
    NewSerial.print(F(">"));
    //Read command
    if(read_line(buffer, sizeof(buffer)) < 1){
      command_succeeded = 1;
      continue;
    }
    command_succeeded = false;
    //Argument 1: The actual command
    char* command_arg = get_cmd_arg(0);

    if (strcmp_P(command_arg, PSTR("packet-mode")) == 0){
       command_succeeded = packet_mode();
    }
    else if(strcmp_P(command_arg, PSTR("init")) == 0){
      command_succeeded = command_init();
    }
//---------------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("help")) == 0) {
      command_succeeded = print_menu();
    }
//---------------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("ls")) == 0){
      command_succeeded = command_ls();
    }
//--------------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("md")) == 0) {
      command_succeeded = command_md();
    }
//----------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("rm")) == 0) {
      command_succeeded = command_rm();
    }
//----------------------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("cd")) == 0){
      command_succeeded = command_cd();
    }
//---------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("read")) == 0) {
      command_succeeded = command_read();
    }
//--------------------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("write")) == 0){
      command_succeeded = command_write();
    }
//-------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("size")) == 0) {
       command_succeeded = command_size();
    }
//----------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("disk")) == 0) {
      command_succeeded = command_disk();
    }
//--------------------------RESET--------------------------
    else if(strcmp_P(command_arg, PSTR("reset")) == 0) {
      Reset_AVR();
    }
//--------------------------NEW -----------------------------
    //Create new file
    else if(strcmp_P(command_arg, PSTR("new")) == 0) {
      command_succeeded = command_new();
    }
//-------------------------APPEND ------------------------------------
    //Append to a current file
    else if(strcmp_P(command_arg, PSTR("append")) == 0){
      command_succeeded = command_append();
    }
//---------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("pwd")) == 0) {
      command_succeeded = command_pwd();
    }
//---------------------------------------------------------
    else {
      if ((feedback_mode & setting::extended_info) > 0) {
        NewSerial.print(F("unknown command: "));
        NewSerial.println(command_arg);
      }
      command_succeeded = false;
    }
//--------------------------------------------------------
  }
  //Do we ever get this far?
  NewSerial.print(F("Exiting: closing down\n"));
}

} // namespace

//Reads a line until the \r enter character is found
// uses commnd_shell fun buffer
byte read_line(char* buffer, byte buffer_length)
{
  memset(buffer, 0, buffer_length); 

  byte read_length = 0;
  while(read_length < buffer_length - 1) {
    while (!NewSerial.available());
    byte c = NewSerial.read();

    toggleLED(statled1.pin);

    if(c == 0x08 || c == 0x7f) { //Backspace characters
      if(read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0'; //Put a terminator on the string in case we are finished

      NewSerial.print((char)0x08); //Move back one space
      NewSerial.print(F(" ")); //Put a blank there to erase the letter from the terminal
      NewSerial.print((char)0x08); //Move back again

      continue;
    }

    // Only echo back if this is enabled
    if ((feedback_mode & setting::echo)){
      NewSerial.print((char)c);
    }

    if(c == '\r') {
      NewSerial.println();
      buffer[read_length] = '\0';
      break;
    }
    else if (c == '\n') {
      //NewSerial.print("Newline\n");
      //Do nothing - ignore newlines
      //This was added to v2.51 to make command line control easier from a micro
      //You never know what fprintf or sprintf is going to throw at the buffer
      //See issue 66: https://github.com/nseidle/OpenLog/issues/66
    }
    /*else if (c == setting::escape_character) {
      NewSerial.println();
      buffer[read_length] = c;
      buffer[read_length + 1] = '\0';
      break;
      //If we see an escape character bail recording whatever is in the current buffer
      //up to the escape char
      //This is used mostly when doing the write command where we need
      //To capture the escape command and immediately record
      //the buffer then stop asking for input from user
      //See issue 168: https://github.com/sparkfun/OpenLog/issues/168
    }*/
    else {
      buffer[read_length] = c;
      ++read_length;
    }
  }
  //Split the command line into arguments
  split_cmd_line_args(buffer, buffer_length);
  return read_length;
}

int8_t getNextFolderTreeIndex()
{
  int8_t i;
  for (i = 0; i < setting::max_folder_depth; i++)
    if (strlen(folderTree[i]) == 0)
      return i;

  if (i >= setting::max_folder_depth){
    i = -1;
  }
  return i;
}

bool change_to_dir(const char *dir)
{
  SdFile newdir;
  bool result = false;
  //Goto parent directory
  //@NOTE: This is a fix to make this work. Should be replaced with
  //proper handling. Limitation: setting::max_folder_depth subfolders
  //ERROR  if (strcmp_P(dir, F("..")) == 0) {

  if (strcmp_P(dir, PSTR("..")) == 0) {
    result = true;
    currentDirectory.close();
    if (!currentDirectory.openRoot(&volume)) {
       systemError(error_code::root_init);
    }
    int8_t index = getNextFolderTreeIndex() - 1;
    if (index >= 0) {
      for (int8_t iTemp = 0; iTemp < index; iTemp++){
        result = newdir.open(&currentDirectory, folderTree[iTemp], O_READ);
        if (!result){
            break;
        }
        currentDirectory = newdir; //Point to new directory
        newdir.close();
      }
      memset(folderTree[index], 0, 11);
    }
    if (((feedback_mode & setting::extended_info) > 0) && (result == false)){
      NewSerial.print(F("cannot cd to parent directory: "));
      NewSerial.println(dir);
    }
  }else {
    result = newdir.open(&currentDirectory, dir, O_READ);
    if (!result) {
      if ((feedback_mode & setting::extended_info) > 0){
        NewSerial.print(F("directory not found: "));
        NewSerial.println(dir);
      }
    }else {
      currentDirectory = newdir; //Point to new directory
      int8_t index = getNextFolderTreeIndex();
      if (index >= 0){
        strncpy(folderTree[index], dir, 11);
      }
    }
  }
  return result;
}

bool print_menu(void)
{
  NewSerial.println(F("OpenLog v3.3"));
  NewSerial.println(F("Basic commands:"));
  NewSerial.println(F("new <file>\t\t: Create <file>"));
  NewSerial.println(F("append <file>\t\t: Append <file>"));
  NewSerial.println(F("write <file> <offset>\t: Write text to <file>, Finish with empty line"));
  NewSerial.println(F("rm <file>\t\t: Delete <file>. Can use \'*\' wildcard"));
  NewSerial.println(F("md <dir>\t\t: Create <dir>"));
  NewSerial.println(F("cd <dir>\t\t: Change directory to <dir>"));
  NewSerial.println(F("cd ..\t\t: Change to parent directory"));
  NewSerial.println(F("ls\t\t\t: List current directory"));
  NewSerial.println(F("read <file> [<start>] [<length>][ <type>]: Output file"));
  NewSerial.println(F("size <file>\t\t: show size of <file>"));
  NewSerial.println(F("disk\t\t\t: Shows card info"));
  NewSerial.println(F("reset\t\t\t: reset"));
  return true;
}

//A rudimentary way to convert a string to a long 32 bit integer
//Used by the read command, in command shell and baud from the system menu
uint32_t strtolong(const char* str)
{
  uint32_t l = 0;
  while(*str >= '0' && *str <= '9'){
    l = l * 10 + (*str++ - '0');
  }
  return l;
}

//Returns the number of command line arguments
byte count_cmd_args(void)
{
  byte count = 0;
  for( byte i = 0; i < setting::max_commandline_args; i++){
    if((cmd_arg[i].arg != 0) && (cmd_arg[i].arg_length > 0)){
      count++;
    }
  }
  return count;
}

//Safe index handling of command line arguments
char general_buffer[30]; //Needed for command shell
char* get_cmd_arg(byte index)
{
  memset(general_buffer, 0, sizeof(general_buffer));
  if (index < setting::max_commandline_args){
    if ((cmd_arg[index].arg != 0) && (cmd_arg[index].arg_length > 0)){
      return strncpy(general_buffer, cmd_arg[index].arg, quan::min(sizeof(general_buffer), cmd_arg[index].arg_length));
    }
  }
  return nullptr;
}

//Safe adding of command line arguments
void add_cmd_arg(const char* buffer, byte buffer_length)
{
  byte count = count_cmd_args();
  if (count < setting::max_commandline_args){
    cmd_arg[count].arg = buffer;
    cmd_arg[count].arg_length = buffer_length;
  }
}

//Split the command line arguments
//Example:
//	read <filename> <start> <length>
//	arg[0] -> read
//	arg[1] -> <filename>
//	arg[2] -> <start>
//	arg[3] -> <end>
byte split_cmd_line_args(const char* buffer, byte buffer_length)
{
  byte arg_index_start = 0;
  byte arg_index_end = 1;
  //Reset command line arguments
  memset(cmd_arg, 0, sizeof(cmd_arg));
  //Split the command line arguments
  while (arg_index_end < buffer_length){
    //Search for ASCII 32 (Space)
    if ((buffer[arg_index_end] == ' ') || (arg_index_end + 1 == buffer_length)) {
      //Fix for last character
      if (arg_index_end + 1 == buffer_length){
        arg_index_end = buffer_length;
      }
      //Add this command line argument to the list
      add_cmd_arg(&(buffer[arg_index_start]), (arg_index_end - arg_index_start));
      arg_index_start = ++arg_index_end;
    }
    arg_index_end++;
  }
  //Return the number of available command line arguments
  return count_cmd_args();
}

//The following functions are required for wildcard use
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Returns char* pointer to buffer if buffer is a valid number or
//0(null) if not.
bool is_number(const char* buffer, byte buffer_length)
{
  for (int i = 0; i < buffer_length; i++){
    if (!isdigit(buffer[i])){
      return false;
    }
  }
  return true;
}
void removeErrorCallback(const char* fileName)
{
  NewSerial.print((char *)F("Remove failed: "));
  NewSerial.println(fileName);
}

//Wildcard string compare.
//Written by Jack Handy - jakkhandy@hotmail.com
//http://www.codeproject.com/KB/string/wildcmp.aspx
bool wildcmp(const char* wild, const char* string)
{
  const char *cp = 0;
  const char *mp = 0;

  while (*string && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')){
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*'){
      if (!(*(++wild))){
        return 1;
      }
      mp = wild;
      cp = string+1;
    }else if ((*wild == *string) || (*wild== '?')) {
      wild++;
      string++;
    }else{
      wild = mp;
      string = cp++;
    }
  }
  while (*wild == '*'){
    wild++;
  }
  return !(*wild);
}

#include "lsprint.cpp"

void blink_error(byte ERROR_TYPE) 
{
  while(1) {
    for(int x = 0 ; x < ERROR_TYPE ; x++) {
      digitalWrite(statled1.pin, port_pin::high);
      delay(200);
      digitalWrite(statled1.pin, port_pin::low);
      delay(200);
    }
    delay(2000);
  }
}
