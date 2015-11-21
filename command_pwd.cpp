bool command_pwd()
{
   NewSerial.print(F(".\\"));
   byte const tmp_var = getNextFolderTreeIndex();
   for (byte i = 0; i < tmp_var; ++i){
      NewSerial.print(folderTree[i]);
      if (i < tmp_var-1) {
         NewSerial.print(F("\\"));
      }
   }
   NewSerial.println();
   return true;
}