
### APM serial log makefile ###

# Copyright (c) 2013 Andy Little 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>

###########################################################
## You will need to modify these three variables for your system
ARDUINO_PATH = /home/andy/arduino-1.0.4/
#QUAN_INCLUDE_PATH = /home/andy/website/quan-trunk/
ARDUINO_PORT = /dev/ttyUSB0
###########################################################

ARDUINO_MCU = atmega328p
AVRDUDE_MPU = m328p
#ARDUINO_BURNRATE = 57600
ARDUINO_BURNRATE = 115200
ARDUINO_FCPU = 16000000UL

ARDUINO_SRC_PATH = $(ARDUINO_PATH)hardware/arduino/cores/arduino/
ARDUINO_EXTRA_PATH = $(ARDUINO_PATH)hardware/arduino/variants/standard/

LIBRARY_PATH = /home/andy/cpp/projects/apm_serial_log/libraries/
SDFAT_PATH = $(LIBRARY_PATH)SdFat/
SERIALPORT_PATH = $(LIBRARY_PATH)SerialPort/
EEPROM_PATH = $(ARDUINO_PATH)libraries/EEPROM/

#arduino_cpp_objects = CDC.o HardwareSerial.o Print.o Stream.o  WString.o main.o new.o 
arduino_cpp_objects = CDC.o  Print.o main.o new.o
arduino_c_objects = WInterrupts.o wiring.o wiring_digital.o
local_objects = apm_log.o
sdfat_objects = istream.o  Sd2Card.o     SdFat.o      SdFile.o    SdVolume.o \
ostream.o  SdBaseFile.o  SdFatUtil.o  SdStream.o

OBJECTS = $(arduino_cpp_objects) $(arduino_c_objects) $(local_objects) $(sdfat_objects) eeprom.o serial_port.o

INCLUDES =  -I$(ARDUINO_SRC_PATH) -I$(ARDUINO_EXTRA_PATH) \
-I$(SDFAT_PATH) -I$(SERIALPORT_PATH) -I$(EEPROM_PATH)

OUTPUT_FILENAME = apm_log
ELFFILE =     $(OUTPUT_FILENAME).elf
HEXFILE =     $(OUTPUT_FILENAME).hex
EEPHEXFILE =  $(OUTPUT_FILENAME).eep.hex
LISTFILE   =  $(OUTPUT_FILENAME).lss

CC = avr-g++
CC1 = avr-gcc
LD = avr-g++
CP = avr-objcopy
OD  = avr-objdump
SIZ = avr-size
PROG = avrdude

CFLAGS_C = -Wall -mmcu=$(ARDUINO_MCU) $(INCLUDES) -ffunction-sections -fdata-sections -Wno-deprecated-declarations \
-D__PROG_TYPES_COMPAT__ -DARDUINO=104 -DF_CPU=$(ARDUINO_FCPU) -DAP_QUANTRACKER_CUSTOM_BUILD -Os 

CFLAGS_CPP = $(CFLAGS_C) --std=c++11 -fno-rtti -fno-exceptions 

MAPFILE =  $(ELFFILE).map

LFLAGS = -mmcu=$(ARDUINO_MCU) -Wl,-Map=$(MAPFILE),--cref -Wl,--gc-sections -Wl,-u,vfprintf -lprintf_flt -lm -s

all : test

upload : test
	$(PROG) -p $(AVRDUDE_MPU) -c arduino -P $(ARDUINO_PORT) -b $(ARDUINO_BURNRATE) -D -Uflash:w:./$(HEXFILE):i

#upload : test
#	$(PROG) -p $(AVRDUDE_MPU) -c stk500v1 -P $(ARDUINO_PORT) -b $(ARDUINO_BURNRATE) -U flash:w:/$(HEXFILE)

test : $(ELFFILE)
	$(CP) -O ihex -R .eeprom -R .eesafe $(ELFFILE) $(HEXFILE)
#	$(CP) --no-change-warnings -j .eeprom --change-section-lma .eeprom=0 -O ihex $(ELFFILE) $(EEPHEXFILE)
	$(OD) -h -S $(ELFFILE) > $(LISTFILE)
	
$(arduino_cpp_objects) : %.o : $(ARDUINO_SRC_PATH)%.cpp
	$(CC) $(CFLAGS_CPP) -c $< -o $@

$(arduino_c_objects) : %.o : $(ARDUINO_SRC_PATH)%.c
	$(CC1) $(CFLAGS_C) -c $< -o $@

$(sdfat_objects) : %.o : $(SDFAT_PATH)%.cpp
	$(CC) $(CFLAGS_CPP) -c $< -o $@

$(local_objects) : %.o : %.cpp
	$(CC) $(CFLAGS_CPP) -c $< -o $@

$(ELFFILE) : $(OBJECTS)
	$(CC) -o $@ $(LFLAGS) $(OBJECTS)
	$(SIZ) -A $(ELFFILE)

eeprom.o : $(EEPROM_PATH)EEPROM.cpp
	$(CC) $(CFLAGS_CPP) -c $< -o $@

serial_port.o : $(SERIALPORT_PATH)SerialPort.cpp
	$(CC) $(CFLAGS_CPP) -c $< -o $@

clean:
	-rm -rf *.o *.elf *.hex *.lss *.map


















