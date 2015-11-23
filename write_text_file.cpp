// include inline in apm_log.cpp

//-----------------------------------------------------------------------------------
// use for text only
byte write_text_file(const char* file_name, uint8_t oflags)
{
  if ( file_name == nullptr){
      NewSerial.print(F("No file specified\n"));
      return false;
  }
  if (!file.open(&currentDirectory, file_name, oflags)) {
      NewSerial.print(F("Couldnt open"));
      NewSerial.println(file_name);
      return false;
  }
  if (file.fileSize() == 0) {
    //This is a trick to make sure first cluster is allocated - found in Bill's example/beta code
    file.rewind();
    file.sync();
  }  

   NewSerial.print(F("<")); //give a different prompt to indicate no echoing
   digitalWrite(statled1.pin, HIGH); //Turn on indicator LED

   byte escape_chars_received = 0;
//   constexpr uint16_t max_idle_time_ms = 500; //The number of milliseconds before unit goes to sleep
   constexpr uint16_t max_time_before_sync_ms = 5000;
   uint32_t lastSyncTime = millis(); //Keeps track of the last time the file was synced
   #if RAM_TESTING == 1
   printRam(); //Print the available RAM
   #endif
   //Start recording incoming characters

   for(;;){
     if((millis() - lastSyncTime) > max_time_before_sync_ms){ 
     //This is here to make sure a log is recorded in the instance
     //where the user is throwing non-stop data at the unit from power on to forever
        file.sync(); //Sync the card
        lastSyncTime = millis();
     }
     if ( NewSerial.available()){
        uint8_t const ch = NewSerial.read();
        if ( ch == setting::escape_character){
            if ( ++ escape_chars_received == setting::num_escapes){
                 file.sync();
                 file.close(); // Done recording, close out the file
                 digitalWrite(statled1.pin, port_pin::low); // Turn off indicator LED
                 NewSerial.print(F("~")); // Indicate a successful record
                 return(1); //Success!
            }
        }else{
            // write any escapes to file
            for ( uint8_t i = 0; i != escape_chars_received; ++i){
               file.write(setting::escape_character);
            }
            escape_chars_received = 0;
            file.write(ch);
        }
      }
    }

}

//----------------------------------------------------------------------------
