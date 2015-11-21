

bool command_size()
{
   //Argument 2: File name - no wildcard search
   char* command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return 0;
   }
   SdFile tempFile;
   bool result = false;
   //search file in current directory and open it
   if (tempFile.open(&currentDirectory, command_arg, O_READ)) {
     NewSerial.print(tempFile.fileSize());
     tempFile.close();
     result = true;
   }else {
      if ((feedback_mode & setting::extended_info) > 0){
       NewSerial.print(F("-1")); //Indicate no file is found
      }
      result = false;
   }
   if ((feedback_mode & setting::end_marker) == 0){
     NewSerial.println();
   }
   return result;
}