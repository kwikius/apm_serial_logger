

bool command_read()
{
      //Argument 2: File name
      char* command_arg = get_cmd_arg(1);
      if(command_arg == 0){
        return false;
      }

      //search file in current directory and open it
      if (!file.open(&currentDirectory, command_arg, O_READ)) {
        return false;
      }

      command_arg = get_cmd_arg(2);
      //Argument 3: File seek position
      if (command_arg != 0) {
        if ( is_number(command_arg, strlen(command_arg))) {
          int32_t offset = strtolong(command_arg);
          if(!file.seekSet(offset)) {
            // error seeking
            file.close();
            return false;
          }
        }
      }

      //Argument 4: How much data (number of characters) to read from file
      uint32_t readAmount = (uint32_t)-1;
      command_arg = get_cmd_arg(3);
      if (command_arg != 0){
        if (is_number(command_arg, strlen(command_arg))){
          readAmount = strtolong(command_arg);
         }
      }

      //Argument 5: Should we print ASCII or HEX? 1 = ASCII, 2 = HEX, 3 = RAW
      uint32_t printType = 1; //Default to ASCII
      command_arg = get_cmd_arg(4);
      if (command_arg != 0){
        if (is_number(command_arg, strlen(command_arg))){
          printType = strtolong(command_arg);
        }
      }

      //Print file contents from current seek position to the end (readAmount)
      uint32_t readSpot = 0;
    //  while ( (int16_t v = file.read()) >= 0) {
        for(;;){
         int16_t v = file.read();
         if ( v >= 0){
           //file.read() returns a 16 bit character. We want to be able to print extended ASCII
           //So we need 8 bit unsigned.
           byte c = v; //Force the 16bit signed variable into an 8bit unsigned

           if(++readSpot > readAmount) break;
           if(printType == 1) { //Printing ASCII
             //Test character to see if it is visible, if not print '.'
             if(c >= ' ' && c < 127)
               NewSerial.write(c); //Regular ASCII
             else if (c == '\n' || c == '\r')
               NewSerial.write(c); //Go ahead and print the carriage returns and new lines
             else
               NewSerial.write(F(".")); //For non visible ASCII characters, print a .
           }
           else if (printType == 2) {
             NewSerial.print(c, HEX); //Print in HEX
             NewSerial.print(F(" "));
           }
           else if (printType == 3) {
             NewSerial.write(c); //Print raw
           }
         }else{
            break;
         }
      }
      file.close();

      if ((feedback_mode & setting::end_marker) == 0){
        NewSerial.println();
      }
      return true;
}