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
      static constexpr byte      num_escapes = 3; //Number of escape chars before break logging, default is 3
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

   static char folderTree[setting::max_folder_depth][12];

   //Used for wild card delete and search
   struct command_arg_t
   {
     const char* arg; //Points to first character in command line argument
     byte arg_length; //Length of command line argument
   };

   command_arg_t cmd_arg[setting::max_commandline_args];
   constexpr byte feedback_mode = (setting::echo | setting::extended_info);
}

void setup_sd_and_fat()
{
  //Setup SD & FAT
  if (!card.init(SPI_FULL_SPEED)) systemError(error_code::card_init);
  if (!volume.init(&card)) systemError(error_code::volume_init);
  currentDirectory.close(); //We close the cD before opening root. This comes from QuickStart example. Saves 4 bytes.
  if (!currentDirectory.openRoot(&volume)) systemError(error_code::root_init);
}

#include "setup.cpp"

void loop(void)
{
  command_shell();
 // while(1); //We should never get this far
}

void power_saving_mode()
{
   digitalWrite(statled1.pin, port_pin::low);

   power_timer0_disable(); //Shut down peripherals we don't need
   power_spi_disable();
   sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received

   power_spi_enable(); //After wake up, power up peripherals
   power_timer0_enable();

}

#include "append_file.cpp"

byte command_init()
{
   currentDirectory.close();
   //Open the root directory
   if (!currentDirectory.openRoot(&volume)) {
      systemError(error_code::root_init);
   }
   memset(folderTree, 0, sizeof(folderTree)); //Clear folder tree

   return 1;
}

uint8_t command_ls()
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
   return 1;
}

byte command_md()
{
   //Argument 2: Directory name
   char * command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return 0;
   }
   SdFile newDirectory;
   if (!newDirectory.makeDir(&currentDirectory, command_arg)) {
     if ((feedback_mode & setting::extended_info) > 0)
     {
       NewSerial.print(F("error creating directory: "));
       NewSerial.println(command_arg);
     }
     return 0;
   }
   else
   {
     return 1;
   }
}
#include "command_rm.cpp"
#include "command_read.cpp"
#include "command_write.cpp"

void command_shell(void)
{
  //Provide a simple shell
  char buffer[30];
  byte tmp_var;
  SdFile tempFile;
  byte command_succeeded = 1;
#if RAM_TESTING == 1
  printRam(); //Print the available RAM
#endif
  while(true){
    if ((feedback_mode & setting::end_marker) > 0){
      NewSerial.print(F("\0x1A")); // Ctrl+Z ends the data and marks the start of result
    }
    if (command_succeeded == 0){
      NewSerial.print(F("!"));
    }
    NewSerial.print(F(">"));
    //Read command
    if(read_line(buffer, sizeof(buffer)) < 1){
      command_succeeded = 1;
      continue;
    }
    command_succeeded = 0;
    //Argument 1: The actual command
    char* command_arg = get_cmd_arg(0);
//-------------------------------------------------
    //Execute command
    if(strcmp_P(command_arg, PSTR("init")) == 0){
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
    //NOTE on using "rm <option>/<file> <subfolder>"
    // "rm -rf <subfolder>" removes the <subfolder> and all contents recursively
    // "rm <subfolder>" removes the <subfolder> only if its empty
    // "rm <filename>" removes the <filename>
    else if(strcmp_P(command_arg, PSTR("rm")) == 0) {
      command_succeeded = command_rm();
    }
//----------------------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("cd")) == 0){
      //Argument 2: Directory name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0){
        continue;
      }
      //open directory
      tmp_var = gotoDir(command_arg);
      command_succeeded = tmp_var;
    }
//---------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("read")) == 0) {
      command_succeeded = command_read();
    }
//--------------------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("write")) == 0){
#if 0
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (!tempFile.open(&currentDirectory, command_arg, O_WRITE)) {
        if ((feedback_mode & setting::extended_info) > 0) {
          NewSerial.print(F("Failed to open file "));
          NewSerial.println(command_arg);
        }
        continue;
      }

      //Argument 3: File seek position
      if ((command_arg = get_cmd_arg(2)) != 0){
        if (is_number(command_arg, strlen(command_arg))) {
          int32_t offset = strtolong(command_arg);
          if(!tempFile.seekSet(offset)) {
            if ((feedback_mode & setting::extended_info) > 0)
            {
              NewSerial.print(F("Error seeking to "));
              NewSerial.println(command_arg);
            }
            tempFile.close();
            continue;
          }
        }
      }
      //read text from the shell and write it to the file
      byte dataLen;
      while(1) {
        if ((feedback_mode & setting::end_marker) > 0){
          NewSerial.print((char)0x1A); // Ctrl+Z ends the data and marks the start of result
        }
        NewSerial.print(F("<")); //give a different prompt

        //read one line of text
        dataLen = read_line(buffer, sizeof(buffer));
        if(!dataLen) {
//#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succeeded = 1;
//#endif
          break;
        }
        
        //If we see the escape character at the end of the buffer then record up to
        //that point in the buffer excluding the escape char
        //See issue 168: https://github.com/sparkfun/OpenLog/issues/168
        /*if(buffer[dataLen] == setting::escape_character)
        {
          //dataLen -= 1; //Adjust dataLen to remove the escape char
          tempFile.write((byte*) buffer, dataLen); //write text to file
          break; //Quit recording to file
        }*/

        //write text to file
        if(tempFile.write((byte*) buffer, dataLen) != dataLen) {
          if ((feedback_mode & setting::extended_info) > 0)
            NewSerial.print(F("error writing to file\n\r"));
          break;
        }
        //If we didn't fill up the buffer then user must have sent NL. Append new line and return
        if(dataLen < (sizeof(buffer) - 1)) tempFile.write("\n\r", 2); 
        
      }

      tempFile.close();
#else
      command_succeeded = command_write(buffer, sizeof(buffer));
#endif
    }
//-------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("size")) == 0) {
      //Argument 2: File name - no wildcard search
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (tempFile.open(&currentDirectory, command_arg, O_READ)) {
        NewSerial.print(tempFile.fileSize());
        tempFile.close();
//#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succeeded = 1;
//#endif
      }
      else
      {
        if ((feedback_mode & setting::extended_info) > 0)
          NewSerial.print(F("-1")); //Indicate no file is found
      }
//#ifdef INCLUDE_SIMPLE_EMBEDDED
      if ((feedback_mode & setting::end_marker) == 0)
//#endif
        NewSerial.println();
    }
//----------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("disk")) == 0) {
      //Print card type
      NewSerial.print(F("\nCard type: "));
      switch(card.type()) {
      case SD_CARD_TYPE_SD1:
        NewSerial.println(F("SD1"));
        break;
      case SD_CARD_TYPE_SD2:
        NewSerial.println(F("SD2"));
        break;
      case SD_CARD_TYPE_SDHC:
        NewSerial.println(F("SDHC"));
        break;
      default:
        NewSerial.println(F("Unknown"));
      }

      //Print card information
      cid_t cid;
      if (!card.readCID(&cid)) {
        NewSerial.print(F("readCID failed"));
        continue;
      }

      NewSerial.print(F("Manufacturer ID: "));
      NewSerial.println(cid.mid, HEX);

      NewSerial.print(F("OEM ID: "));
      NewSerial.print(cid.oid[0]);
      NewSerial.println(cid.oid[1]);

      NewSerial.print(F("Product: "));
      for (byte i = 0; i < 5; i++) {
        NewSerial.print(cid.pnm[i]);
      }

      NewSerial.print(F("\n\rVersion: "));
      NewSerial.print(cid.prv_n, DEC);
      NewSerial.print(F("."));
      NewSerial.println(cid.prv_m, DEC);

      NewSerial.print(F("Serial number: "));
      NewSerial.println(cid.psn);

      NewSerial.print(F("Manufacturing date: "));
      NewSerial.print(cid.mdt_month);
      NewSerial.print(F("/"));
      NewSerial.println(2000 + cid.mdt_year_low + (cid.mdt_year_high <<4));

      csd_t csd;
      uint32_t cardSize = card.cardSize();
      if (cardSize == 0 || !card.readCSD(&csd)) {
        NewSerial.println(F("readCSD failed"));
        continue;
      }
      NewSerial.print(F("Card Size: "));
      cardSize /= 2; //Card size is coming up as double what it should be? Don't know why. Dividing it by 2 to correct.
      NewSerial.print(cardSize);
      //After division
      //7761920 = 8GB card
      //994816 = 1GB card
      NewSerial.println(F(" KB"));
//#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succeeded = 1;
//#endif
    }
//--------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("sync")) == 0){
      //Flush all current data and record it to card
      //This isn't really tested.
      tempFile.sync();
      currentDirectory.sync();
//#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succeeded = 1;
//#endif
    }
//--------------------------RESET--------------------------
    //Reset the AVR
    else if(strcmp_P(command_arg, PSTR("reset")) == 0)
    {
      Reset_AVR();
    }
//--------------------------NEW -----------------------------
    //Create new file
    else if(strcmp_P(command_arg, PSTR("new")) == 0) {
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //Try to open file, if fail (file doesn't exist), then break
      if (tempFile.open(&currentDirectory, command_arg, O_CREAT | O_EXCL | O_WRITE)) {//Will fail if file already exsists
        tempFile.close(); //Everything is good, Close this new file we just opened
//#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succeeded = 1;
//#endif
      }
      else
      {
        if ((feedback_mode & setting::extended_info) > 0) {
          NewSerial.print(F("Error creating file: "));
          NewSerial.println(command_arg);
        }
      }
    }
//-------------------------APPEND ------------------------------------
    //Append to a current file
    else if(strcmp_P(command_arg, PSTR("append")) == 0){
      //Argument 2: File name
      //Find the end of a current file and begins writing to it
      //Ends only when the user inputs Ctrl+z (ASCII 26)
      command_arg = get_cmd_arg(1);
      if(command_arg == 0){
        continue;
      }
      //append_file: Uses circular buffer to capture full stream of text and append to file
      command_succeeded = append_file(command_arg);
    }
//---------------------------------------------------------
    else if(strcmp_P(command_arg, PSTR("pwd")) == 0) {
      NewSerial.print(F(".\\"));
      tmp_var = getNextFolderTreeIndex();
      for (byte i = 0; i < tmp_var; i++)
      {
        NewSerial.print(folderTree[i]);
        if (i < tmp_var-1) NewSerial.print(F("\\"));
      }
      NewSerial.println();
      command_succeeded = 1;
    }
//---------------------------------------------------------
    else
    {
      if ((feedback_mode & setting::extended_info) > 0) {
        NewSerial.print(F("unknown command: "));
        NewSerial.println(command_arg);
      }
    }
//--------------------------------------------------------
  }
  //Do we ever get this far?
  NewSerial.print(F("Exiting: closing down\n"));
}

//Reads a line until the \n enter character is found
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
      NewSerial.print("Newline\n");
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

  if (i >= setting::max_folder_depth)
    i = -1;

  return i;
}

byte gotoDir(const char *dir)
{
  SdFile subDirectory;
  byte tmp_var = 0;

  //Goto parent directory
  //@NOTE: This is a fix to make this work. Should be replaced with
  //proper handling. Limitation: setting::max_folder_depth subfolders
  //ERROR  if (strcmp_P(dir, F("..")) == 0) {
  if (strcmp_P(dir, PSTR("..")) == 0) {
    tmp_var = 1;
    //close file system
    currentDirectory.close();
    // open the root directory
    if (!currentDirectory.openRoot(&volume)) systemError(error_code::root_init);
    int8_t index = getNextFolderTreeIndex() - 1;
    if (index >= 0)
    {
      for (int8_t iTemp = 0; iTemp < index; iTemp++)
      {
        if (!(tmp_var = subDirectory.open(&currentDirectory, folderTree[iTemp], O_READ)))
          break;

        currentDirectory = subDirectory; //Point to new directory
        subDirectory.close();
      }
      memset(folderTree[index], 0, 11);
    }
    if (((feedback_mode & setting::extended_info) > 0) && (tmp_var == 0))
    {
      NewSerial.print(F("cannot cd to parent directory: "));
      NewSerial.println(dir);
    }
  }
  else
  {
    if (!(tmp_var = subDirectory.open(&currentDirectory, dir, O_READ))) {
      if ((feedback_mode & setting::extended_info) > 0)
      {
        NewSerial.print(F("directory not found: "));
        NewSerial.println(dir);
      }
    }
    else
    {
      currentDirectory = subDirectory; //Point to new directory
      int8_t index = getNextFolderTreeIndex();
      if (index >= 0)
        strncpy(folderTree[index], dir, 11);
    }
  }
  return tmp_var;
}

byte print_menu(void)
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
  return 1;
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
 
  for( byte i = 0; i < setting::max_commandline_args; i++)
    if((cmd_arg[i].arg != 0) && (cmd_arg[i].arg_length > 0))
      count++;

  return count;
}

//Safe index handling of command line arguments
char general_buffer[30]; //Needed for command shell
#define MIN(a,b) ((a)<(b))?(a):(b)
char* get_cmd_arg(byte index)
{
  memset(general_buffer, 0, sizeof(general_buffer));
  if (index < setting::max_commandline_args)
    if ((cmd_arg[index].arg != 0) && (cmd_arg[index].arg_length > 0))
      return strncpy(general_buffer, cmd_arg[index].arg, MIN(sizeof(general_buffer), cmd_arg[index].arg_length));

  return 0;
}

//Safe adding of command line arguments
void add_cmd_arg(const char* buffer, byte buffer_length)
{
  byte count = count_cmd_args();
  if (count < setting::max_commandline_args)
  {
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
byte wildcmp(const char* wild, const char* string)
{

  const char *cp = 0;
  const char *mp = 0;

  while (*string && (*wild != '*'))
  {
    if ((*wild != *string) && (*wild != '?'))
      return 0;

    wild++;
    string++;
  }

  while (*string)
  {
    if (*wild == '*')
    {
      if (!(*(++wild)))
        return 1;

      mp = wild;
      cp = string+1;
    }
    else if ((*wild == *string) || (*wild== '?'))
    {
      wild++;
      string++;
    }
    else
    {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*')
    wild++;
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
