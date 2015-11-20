
// not used atm
#if 0
namespace {
   char new_file_name[13];
}
char* newlog(void)
{
  SdFile newFile; //This will contain the file for SD writing
  uint16_t new_file_number = 
      static_cast<uint16_t>(EEPROM.read(eeprom_loc::new_file_idx)) | 
      (static_cast<uint16_t>(EEPROM.read(eeprom_loc::new_file_idx+1)) << 8);
      
  //If both EEPROM spots are 255 (0xFF), that means they are un-initialized (first time OpenLog has been turned on)
  //Let's init them both to 0
  if(new_file_number == 0xFFFF) {
    new_file_number = 0; //By default, unit will start at file number zero
    EEPROM.write(eeprom_loc::new_file_idx, 0x00);
    EEPROM.write(eeprom_loc::new_file_idx+1, 0x00);
  }

  //The above code looks like it will forever loop if we ever create 65535 logs
  //Let's quit if we ever get to 65534
  //65534 logs is quite possible if you have a system with lots of power on/off cycles
  if(new_file_number == 65534) {
    //Gracefully drop out to command prompt with some error
    NewSerial.print(F("!Too many logs:1!"));
    return(0); //Bail!
  }
  //If we made it this far, everything looks good - let's start testing to see if our file number is the next available
  //Search for next available log spot
  while(1) {
    sprintf_P(new_file_name, PSTR("LOG%05u.TXT"), new_file_number); //Splice the new file number into this file name
    //Try to open file, if fail (file doesn't exist), then break
    if (newFile.open(&currentDirectory, new_file_name, O_CREAT | O_EXCL | O_WRITE)) {
      break;
    }
    //Try to open file and see if it is empty. If so, use it.
    if (newFile.open(&currentDirectory, new_file_name, O_READ))  {
      if (newFile.fileSize() == 0) {
        newFile.close();        // Close this existing file we just opened.
        return(new_file_name);  // Use existing empty file.
      }
      newFile.close(); // Close this existing file we just opened.
    }
    //Try the next number
    new_file_number++;
    if(new_file_number > 65533) //There is a max of 65534 logs
    {
      NewSerial.print(F("!Too many logs:2!"));
      return(0); //Bail!
    }
  }
  newFile.close(); //Close this new file we just opened
  new_file_number++; //Increment so the next power up uses the next file #

  //Record new_file number to EEPROM
  byte lsb = (byte)(new_file_number & 0x00FF);
  byte msb = (byte)((new_file_number & 0xFF00) >> 8);
  EEPROM.write(eeprom_loc::new_file_idx, lsb); 
  if (EEPROM.read(eeprom_loc::new_file_idx+1) != msb){
    EEPROM.write(eeprom_loc::new_file_idx+1, msb); // MSB
  }

#if DEBUG
  NewSerial.print(F("\nCreated new file: "));
  NewSerial.println(new_file_name);
#endif
  return(new_file_name);
}
#endif