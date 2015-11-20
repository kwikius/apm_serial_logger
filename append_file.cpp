// include inline in apm_log.cpp

//-----------------------------------------------------------------------------------

//This is the most important function of the device. These loops have been tweaked as much as possible.
//Modifying this loop may negatively affect how well the device can record at high baud rates.
//Appends a stream of serial data to a given file
//Assumes the currentDirectory variable has been set before entering the routine
//Does not exit until Ctrl+z (ASCII 26) is received
//Returns 0 on error
//Returns 1 on success
byte append_file(const char* file_name)
{
  SdFile workingFile;
  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!workingFile.open(&currentDirectory, file_name, O_CREAT | O_APPEND | O_WRITE)) systemError(error_code::file_open);
  if (workingFile.fileSize() == 0) {
    //This is a trick to make sure first cluster is allocated - found in Bill's example/beta code
    //workingFile.write((byte)0); //Leaves a NUL at the beginning of a file
    workingFile.rewind();
    workingFile.sync();
  }  

  NewSerial.print(F("<")); //give a different prompt to indicate no echoing
  digitalWrite(statled1.pin, HIGH); //Turn on indicator LED

  const byte LOCAL_BUFF_SIZE = 128; //This is the 2nd buffer. It pulls from the larger NewSerial buffer as quickly as possible.
  byte localBuffer[LOCAL_BUFF_SIZE];
  byte checkedSpot;
  byte escape_chars_received = 0;

  const uint16_t MAX_IDLE_TIME_MSEC = 500; //The number of milliseconds before unit goes to sleep
  const uint16_t MAX_TIME_BEFORE_SYNC_MSEC = 5000;
  uint32_t lastSyncTime = millis(); //Keeps track of the last time the file was synced
#if RAM_TESTING == 1
  printRam(); //Print the available RAM
#endif
  //Check if we should ignore escape characters
  //If we are ignoring escape characters the recording loop is infinite and can be made shorter (less checking)
  //This should allow for recording at higher incoming rates
  if(setting::num_escapes == 0){
    while(1){
      byte charsToRecord = NewSerial.read(localBuffer, sizeof(localBuffer)); //Read characters from global buffer into the local buffer
      if (charsToRecord > 0) {
         workingFile.write(localBuffer, charsToRecord); //Record the buffer to the card
         toggleLED(statled1.pin); //STAT1_PORT ^= (1<<STAT1); //Toggle the STAT1 LED each time we record the buffer
         if((millis() - lastSyncTime) > MAX_TIME_BEFORE_SYNC_MSEC){ 
          //This is here to make sure a log is recorded in the instance
          //where the user is throwing non-stop data at the unit from power on to forever
           workingFile.sync(); //Sync the card
           lastSyncTime = millis();
         }
      }else if( (millis() - lastSyncTime) > MAX_IDLE_TIME_MSEC) { //If we haven't received any characters in 2s, goto sleep
         workingFile.sync(); //Sync the card before we go to sleep
         power_saving_mode();
         escape_chars_received = 0; // Clear the esc flag as it has timed out
         lastSyncTime = millis(); //Reset the last sync time to now
      }
    }
  }
  //We only get this far if escape characters are more than zero
  //Start recording incoming characters
  while(escape_chars_received < setting::num_escapes) {
    byte charsToRecord = NewSerial.read(localBuffer, sizeof(localBuffer)); //Read characters from global buffer into the local buffer
    if (charsToRecord > 0) {
      if (localBuffer[0] == setting::escape_character)  {
        escape_chars_received++;
        //Scan the local buffer for escape characters
        for(checkedSpot = 1 ; checkedSpot < charsToRecord ; checkedSpot++){
          if(localBuffer[checkedSpot] == setting::escape_character) {
            escape_chars_received++;
            //If charsToRecord is greater than 3 there's a chance here where we receive three esc chars
            // and then reset the variable: 26 26 26 A T + would not escape correctly
            if(escape_chars_received == setting::num_escapes){
              break;
            }
          }
          else{
            escape_chars_received = 0;
          }
        }
      }else{
        escape_chars_received = 0;
      }
      workingFile.write(localBuffer, charsToRecord); //Record the buffer to the card
      toggleLED(statled1.pin); //STAT1_PORT ^= (1<<STAT1); //Toggle the STAT1 LED each time we record the buffer
      if((millis() - lastSyncTime) > MAX_TIME_BEFORE_SYNC_MSEC){ 
        //This is here to make sure a log is recorded in the instance
        //where the user is throwing non-stop data at the unit from power on to forever
        workingFile.sync(); //Sync the card
        lastSyncTime = millis();
      }
    }
    //No characters received?
    else if( (millis() - lastSyncTime) > MAX_IDLE_TIME_MSEC) { //If we haven't received any characters in 2s, goto sleep
      workingFile.sync(); //Sync the card before we go to sleep
      power_saving_mode();
      escape_chars_received = 0; // Clear the esc flag as it has timed out
      lastSyncTime = millis(); //Reset the last sync time to now
    }
  }
  workingFile.sync();
  workingFile.close(); // Done recording, close out the file
  digitalWrite(statled1.pin, port_pin::low); // Turn off indicator LED
  NewSerial.print(F("~")); // Indicate a successful record
  return(1); //Success!
}

//----------------------------------------------------------------------------
