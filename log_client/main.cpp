#include <quan/serial_port.hpp>
#include <quan/utility/timer.hpp>
#include <iostream>

/*

*/

bool connect(quan::serial_port & sp)
{
   sp.init();
   if (sp.good()) {
       std::cout << "Connected ok\n";
   }else {
      std::cout << "Sorry .. couldnt connect\n";
      return false;
   }
   sp.set_baud (9600);
   // toggle dtr to reset the logger
   sp.set_dtr (false);
   quan::timer<> timer;
   while (timer() < quan::time::ms {500}) { ; }
   sp.set_dtr (true);

   timer.restart();
   int chars_in = 0;
   bool good = false;
   bool ready = false;
   while ( (timer() < quan::time::s {2})  && (!ready )) {
       char ch;
       if ( sp.in_avail()){
         sp.read((unsigned char*) &ch,1);
         switch (chars_in){
            case 0:
               if ( ch  == '1'){
                  std::cout << "Loger set up uart...\n";
                  ++chars_in; good = true;
               }else{
                  std::cout << "logger set up uart fail\n";
                  good = false;
               }
               break;
            case 1: 
               if ( ch  == '2'){
                  std::cout << "...logger set up SD and FAT...\n";
                  ++chars_in; good = true;
               }else{
                  std::cout << "...logger set up SD and FAT fail\n";
                  good = false;
               }
               break;
            case 2:
                if ( ch == '<'){
                  std::cout << "...logger ready!\n";
                  ++chars_in; good = true; ready = true;
               }else{
                  std::cout << "...logger ready fail\n";
                  good = false;
               }
               break;
            default:
               break;
         }
         if(! good){
            return false;
         }
       }
   }
   if (!good){
      std::cout << "timed out waiting for logger to ack\n";
   }  
   return good;
}

int main()
{
   quan::serial_port sp ("/dev/ttyUSB0");

   if ( connect(sp)){
      std::cout << "success\n";
   }else{
     std::cout << "fail\n";
   }

   uint8_t buf [] = {0x1a,0x1a,0x1a};
   sp.write(buf,3);

   while (sp.in_avail(quan::time::s{0.1})){
      char ch;
      sp.read((unsigned char*) &ch,1);
      std::cout << ch ;
   }
   sp.write("ls\r");
   while (sp.in_avail(quan::time::s{0.1})){
      char ch;
      sp.read((unsigned char*) &ch,1);
      std::cout << ch ;
   }
}
 