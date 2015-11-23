    //NOTE on using "rm <option>/<file> <subfolder>"
    // "rm -rf <subfolder>" removes the <subfolder> and all contents recursively
    // "rm <subfolder>" removes the <subfolder> only if its empty
    // "rm <filename>" removes the <filename>
//-------------------------------------------------------------
bool command_rm()
{
   //Argument 2: Remove option or file name/subdirectory to remove
   char* command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return false;
   }
  // SdFile file;
   //Argument 2: Remove subfolder recursively?
   if ((count_cmd_args() == 3) && (strcmp_P(command_arg, PSTR("-rf")) == 0)) {
     //Remove the subfolder
     if (file.open(&currentDirectory, get_cmd_arg(2), O_READ))
     {
       byte tmp_var = file.rmRfStar();
       file.close();
       return tmp_var;
     }else{
       return false;
     }
   }
   
   //Argument 2: Remove subfolder if empty or remove file
   if (file.open(&currentDirectory, command_arg, O_READ)){
     byte tmp_var = 0;
     if (file.isDir() || file.isSubDir()){
       tmp_var = file.rmDir();
     }
     else{
       file.close();
       if (file.open(&currentDirectory, command_arg, O_WRITE)){
         tmp_var = file.remove();
       }
     }
     file.close();
     return tmp_var;
   }

   //Argument 2: File wildcard removal
   //Fixed by dlkeng - Thank you!
   uint32_t filesDeleted = 0;
   char fname[13];
   strupr(command_arg);
   currentDirectory.rewind();
   while (file.openNext(&currentDirectory, O_READ)){ //Step through each object in the current directory
     if (!file.isDir() && !file.isSubDir()) { // Remove only files 
       if (file.getFilename(fname)) { // Get the filename of the object we're looking at
         if (wildcmp(command_arg, fname)) { // See if it matches the wildcard 
           file.close();
           file.open(&currentDirectory, fname, O_WRITE);  // Re-open for WRITE to be delete
           if (file.remove()) {// Remove this file
             ++filesDeleted;
           }
         }
       }
     }
     file.close();
   }

   if ((feedback_mode & setting::extended_info) > 0) {
     NewSerial.print(filesDeleted);
     NewSerial.println(F(" file(s) deleted"));
   }
   return (filesDeleted > 0);
}

//---------------------------------------------------------