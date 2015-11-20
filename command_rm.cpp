    //NOTE on using "rm <option>/<file> <subfolder>"
    // "rm -rf <subfolder>" removes the <subfolder> and all contents recursively
    // "rm <subfolder>" removes the <subfolder> only if its empty
    // "rm <filename>" removes the <filename>
//-------------------------------------------------------------
byte command_rm()
{
   //Argument 2: Remove option or file name/subdirectory to remove
   char* command_arg = get_cmd_arg(1);
   if(command_arg == 0){
     return 0;;
   }

   SdFile tempFile;
   //Argument 2: Remove subfolder recursively?
   if ((count_cmd_args() == 3) && (strcmp_P(command_arg, PSTR("-rf")) == 0)) {
     //Remove the subfolder
     if (tempFile.open(&currentDirectory, get_cmd_arg(2), O_READ))
     {
       byte tmp_var = tempFile.rmRfStar();
       tempFile.close();
       return tmp_var;
     }else{
      return 0;
     }
   }
   
   //Argument 2: Remove subfolder if empty or remove file
   if (tempFile.open(&currentDirectory, command_arg, O_READ)){
     byte tmp_var = 0;
     if (tempFile.isDir() || tempFile.isSubDir()){
       tmp_var = tempFile.rmDir();
     }
     else{
       tempFile.close();
       if (tempFile.open(&currentDirectory, command_arg, O_WRITE)){
         tmp_var = tempFile.remove();
       }
     }
     tempFile.close();
     return tmp_var;
   }

   //Argument 2: File wildcard removal
   //Fixed by dlkeng - Thank you!
   uint32_t filesDeleted = 0;
   char fname[13];
   strupr(command_arg);
   currentDirectory.rewind();
   while (tempFile.openNext(&currentDirectory, O_READ)){ //Step through each object in the current directory
     if (!tempFile.isDir() && !tempFile.isSubDir()) { // Remove only files 
       if (tempFile.getFilename(fname)) { // Get the filename of the object we're looking at
         if (wildcmp(command_arg, fname)) { // See if it matches the wildcard 
           tempFile.close();
           tempFile.open(&currentDirectory, fname, O_WRITE);  // Re-open for WRITE to be delete
           if (tempFile.remove()) {// Remove this file
             ++filesDeleted;
           }
         }
       }
     }
     tempFile.close();
   }

   if ((feedback_mode & setting::extended_info) > 0) {
     NewSerial.print(filesDeleted);
     NewSerial.println(F(" file(s) deleted"));
   }
   if (filesDeleted > 0){
     return 1;
   }else{
     return 0;
   }
}

//---------------------------------------------------------