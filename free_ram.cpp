#include "arduino_compat.h"
#include "serial_port.h"

//Passes back the available amount of free RAM
int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void printRam() 
{
  NewSerial.print(F(" RAM:"));
  NewSerial.println(freeRam());
}