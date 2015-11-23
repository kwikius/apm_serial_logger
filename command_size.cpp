

bool command_size()
{
   //Argument 2: File name - no wildcard search
   char* command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return 0;
   }
   if (file.open(&currentDirectory, command_arg, O_READ)) {
     NewSerial.print(file.fileSize());
     file.close();
     return true;
   }else {
     return  false;
   }
}