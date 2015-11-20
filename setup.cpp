
void setup(void)
{
  pinMode(statled1.pin, OUTPUT);
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

  NewSerial.begin(setting::baud_rate);

  NewSerial.print(F("1"));

  setup_sd_and_fat();

  NewSerial.print(F("2"));

#if RAM_TESTING == 1
  printRam(); //Print the available RAM
#endif
  memset(folderTree, 0, sizeof(folderTree)); //Clear folder tree
}