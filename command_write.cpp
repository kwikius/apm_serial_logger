
bool command_write(char * buffer, uint8_t buffer_len)
{
   //Argument 2: File name
   char * command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return false;
   }

   SdFile tempFile;
   //search file in current directory and open it
   if (!tempFile.open(&currentDirectory, command_arg, O_WRITE)) {
     return false;
   }
   //Argument 3: File seek position
   command_arg = get_cmd_arg(2);
   if (command_arg != 0){
     if (is_number(command_arg, strlen(command_arg))) {
       int32_t offset = strtolong(command_arg);
       if(!tempFile.seekSet(offset)) {
         tempFile.close();
         return false;
       }
     }
   }
   //read text from the shell and write it to the file
   while(1) {
     if ((feedback_mode & setting::end_marker) > 0){
       NewSerial.print((char)0x1A); // Ctrl+Z ends the data and marks the start of result
     }
     NewSerial.print(F("<")); //give a different prompt
     //read one line of text
     byte dataLen = read_line(buffer, buffer_len);
     if(dataLen == 0) {
       return true;
     }
     //If we see the escape character at the end of the buffer then record up to
     //that point in the buffer excluding the escape char
     //See issue 168: https://github.com/sparkfun/OpenLog/issues/168
     if(buffer[dataLen] == setting::escape_character) {
       //dataLen -= 1; //Adjust dataLen to remove the escape char
       tempFile.write((byte*) buffer, dataLen); //write text to file
       return true; //Quit recording to file
     }
     //write text to file
     if(tempFile.write((byte*) buffer, dataLen) != dataLen) {
       if ((feedback_mode & setting::extended_info) > 0)
         NewSerial.print(F("error writing to file\n\r"));
       break;
     }
     //If we didn't fill up the buffer then user must have sent NL. Append new line and return
     if(dataLen < (sizeof(buffer) - 1)) {
         tempFile.write("\n\r", 2); 
     }
   }
   tempFile.close();
   return true;
}