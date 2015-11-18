#include "log.h"

void loop(void)
{
  //If we are in new log mode, find a new file name to write to
  //if(setting_system_mode == MODE_NEWLOG)
    append_file(newlog()); //Append the file name that newlog() returns

  //If we are in sequential log mode, determine if seqlog.txt has been created or not, and then open it for logging
  //if(setting_system_mode == MODE_SEQLOG)
    seqlog();

  //Once either one of these modes exits, go to normal command mode, which is called by returning to main()
  command_shell();

  while(1); //We should never get this far
}