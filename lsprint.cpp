

struct print_tree_status{
   static constexpr byte eof = 0;
   static constexpr byte file = 1;
   static constexpr byte subdir = 2;
};

void lsPrint(SdFile * theDir, const char * cmdStr, byte flags, byte indent)
{
   static byte depth = 0;      // current recursion depth
   theDir->rewind();
   for(;;){
      switch(lsPrintNext(theDir, cmdStr, flags, indent)){
         case print_tree_status::eof:
            return;
         case print_tree_status::subdir:
            if ((flags & LS_R) && (depth < setting::max_folder_depth)){
               uint16_t const index = (theDir->curPosition() / sizeof(dir_t) ) - 1;  // determine current directory entry index
               SdFile subdir;
               if (subdir.open(theDir, index, O_READ)){
                  ++depth;        // limit recursion
                  lsPrint(&subdir, cmdStr, flags, indent + setting::subdir_indent); // recursively list subdirectory
                  --depth;
                  subdir.close();
               }
            }
            break;
         default:
            break;
      }
   }
}

byte lsPrintNext(SdFile * theDir, const char * cmdStr, byte flags, byte indent)
{
  byte pos = 0;           // output position
  byte open_stat;         // file open status
  byte status;            // return status
  SdFile tempFile;
  char fname[setting::max_file_string_len];

  // Find next available object to display in the specified directory
  while ((open_stat = tempFile.openNext(theDir, O_READ))){
    if (tempFile.getFilename(fname)) {
      if (tempFile.isDir() || tempFile.isFile() || tempFile.isSubDir()){
        if (tempFile.isFile()) {
          if (wildcmp(cmdStr, fname)){
            status = print_tree_status::file ;// WAS_FILE;
            break;      // is a matching file name, display it
          }
        }else {
          status = print_tree_status::subdir;
          break;        // display subdirectory name
        }
      }
    }
    tempFile.close();
  }
  if (!open_stat) {
    return print_tree_status::eof;     // nothing more in this (sub)directory
  }

  // output the file or directory name indented for dir level
  for (byte i = 0; i < indent; i++){
    NewSerial.write(' ');
  }
  // print name
  pos += NewSerial.print(fname);
  if (tempFile.isSubDir()){
    pos += NewSerial.write('/');    // subdirectory indicator
  }
  // print size if requested (files only)
  if (tempFile.isFile() && (flags & LS_SIZE)){
    while (pos++ < setting::max_file_string_len +1){
      NewSerial.write(' ');
    }
    NewSerial.write(' ');           // ensure at least 1 separating space
    NewSerial.print(tempFile.fileSize());
  }
  NewSerial.writeln();
  tempFile.close();
  return status;
}
