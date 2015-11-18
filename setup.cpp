
#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers
#include "leds.h"
#include "log.h"

void setup(void)
{
  pinMode(statled1, OUTPUT);

  //Power down various bits of hardware to lower power usage  
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  //Shut off TWI, Timer2, Timer1, ADC
  ADCSRA &= ~(1<<ADEN); //Disable ADC
  ACSR = (1<<ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1<<AIN1D)|(1<<AIN0D); //Disable digital input buffer on AIN1/0

  power_twi_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();

  read_system_settings(); //Load all system settings from EEPROM

  //Setup UART
  NewSerial.begin(setting_uart_speed);
  if (setting_uart_speed < 500)      // check for slow baud rates
  {
    //There is an error in the Serial library for lower than 500bps. 
    //This fixes it. See issue 163: https://github.com/sparkfun/OpenLog/issues/163
    // redo USART baud rate configuration
    UBRR0 = (F_CPU / (16UL * setting_uart_speed)) - 1;
    UCSR0A &= ~_BV(U2X0);
  }
  NewSerial.print(F("1"));

  //Setup SD & FAT
  if (!card.init(SPI_FULL_SPEED)) systemError(ERROR_CARD_INIT);
  if (!volume.init(&card)) systemError(ERROR_VOLUME_INIT);
  currentDirectory.close(); //We close the cD before opening root. This comes from QuickStart example. Saves 4 bytes.
  if (!currentDirectory.openRoot(&volume)) systemError(ERROR_ROOT_INIT);

  NewSerial.print(F("2"));

  printRam(); //Print the available RAM

  //Search for a config file and load any settings found. This will over-ride previous EEPROM settings if found.
  read_config_file();

  if(setting_ignore_RX == 0) //If we are NOT ignoring RX, then
    check_emergency_reset(); //Look to see if the RX pin is being pulled low

  memset(folderTree, 0, sizeof(folderTree)); //Clear folder tree
}