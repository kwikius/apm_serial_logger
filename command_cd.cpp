
bool command_cd()
{
   //Argument 2: Directory name
   char* command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return false;
   }else{
      return change_to_dir(command_arg) != 0 ;
   }
}