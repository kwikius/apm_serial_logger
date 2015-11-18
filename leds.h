#ifndef APM_SERIAL_LOG_LEDS_H_INCLUDED
#define APM_SERIAL_LOG_LEDS_H_INCLUDED

#include "arduino_compat.h"

//typedef decltype _SFR_IO8(0x0) sfr8_type;

//STAT1 is a general LED and indicates serial traffic
#define STAT1  5 //On PORTD
#define STAT1_PORT  PORTD
#define STAT2  5 //On PORTB
#define STAT2_PORT  PORTB

//Blinking LED error codes

#define ERROR_SD_INIT	  3
#define ERROR_NEW_BAUD	  5
#define ERROR_CARD_INIT   6
#define ERROR_VOLUME_INIT 7
#define ERROR_ROOT_INIT   8
#define ERROR_FILE_OPEN   9

constexpr byte statled1 = 5;  //This is the normal status LED
constexpr byte statled2 = 13; //This is the SPI LED, indicating SD traffic


#endif // APM_SERIAL_LOG_LEDS_H_INCLUDED
