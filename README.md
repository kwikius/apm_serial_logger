# apm_serial_logger
Using OpenLog hardware to log from ArduPilot, using a spare serial port. 

Based on the OpenLog hardware and using the Openlog software as a basis too
One major issue with OpenLog is that it is using all the ROM on the chip
so I am trying first to pare it away to only the required functions for ardupilot (currently around 23kb from OpenLogs 28kb)

I also plan to use a packet approach to sending and receiving commands and data, rather than escapes.

The project uses the Arduino Libraries, but is built using raw avr-gcc and uploads using avrdude.
since I just find that easier than using the Arduino IDE
