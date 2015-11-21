

bool command_append()
{
   //Argument 2: File name
   //Find the end of a current file and begins writing to it
   //Ends only when the user inputs Ctrl+z (ASCII 26)
   char* command_arg = get_cmd_arg(1);
   if(command_arg){
     return append_file(command_arg);
   }else{
      return false;
   }
   //append_file: Uses circular buffer to capture full stream of text and append to file   
}