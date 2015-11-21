

bool command_new()
{
  char* command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return false;
   }
   SdFile tempFile;
   //Try to open file, if fail (file doesn't exist), then break
   if (tempFile.open(&currentDirectory, command_arg, O_CREAT | O_EXCL | O_WRITE)) {//Will fail if file already exsists
     tempFile.close(); //Everything is good, Close this new file we just opened
     return true;
   }else{
     if ((feedback_mode & setting::extended_info) > 0) {
       NewSerial.print(F("Error creating file: "));
       NewSerial.println(command_arg);
     }
     return false;
   }
}