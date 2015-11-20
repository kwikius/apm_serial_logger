#ifndef APM_SERIAL_LOG_ARDUINO_COMPAT_H_INCLUDED
#define APM_SERIAL_LOG_ARDUINO_COMPAT_H_INCLUDED

#include <avr/pgmspace.h>
#if ARDUINO < 100
#include <WProgram.h>
#else  // ARDUINO
#include <Arduino.h>
#endif  // ARDUINO

#endif // APM_SERIAL_LOG_ARDUINO_COMPAT_H_INCLUDED
