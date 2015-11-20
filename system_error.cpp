
#include "serial_port.h"
#include "error_code.hpp"
#include "log.h"

void systemError(byte error_type)
{
  NewSerial.print(F("Error "));
  switch(error_type)
  {
  case error_code::sd_init:
    NewSerial.print(F("sd")); 
    break;
  case error_code::card_init:
   NewSerial.print(F("card")); 
    break;
  case error_code::volume_init:
    NewSerial.print(F("volume")); 
    break;
  case error_code::root_init:
    NewSerial.print(F("root")); 
    break;
  case error_code::file_open:
    NewSerial.print(F("file")); 
    break;
  }
  blink_error(error_type);
}

//Given a pin, it will toggle it from high to low or vice versa
void toggleLED(byte pinNumber)
{
  if(digitalRead(pinNumber)) digitalWrite(pinNumber, LOW);
  else digitalWrite(pinNumber, HIGH);
}

