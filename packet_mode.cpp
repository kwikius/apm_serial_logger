
#include <quan/uav/cobs/protocol.hpp>

namespace {

   // decodes to 
   constexpr uint8_t nack_packet []= {0,1,1};
   // decodes to 1
   constexpr uint8_t ack_packet []= {0,2,1};
 
   enum packet_id{
       data_packet,
       new_log_packet,
       file_close_packet,
       cd_packet
   };


   static constexpr uint8_t max_packet_length = 131;
   static uint8_t packet_idx = 0; // index of next pos in array and count of num in packet
   enum { not_synced, in_zeros, in_packet};
   static uint8_t current_packet_mode = not_synced;
   uint8_t raw_packet_buffer[max_packet_length];
   uint8_t decoded_packet_buffer[max_packet_length -1];
   
   void packet_ack()
   {
      NewSerial.write(ack_packet, sizeof(ack_packet));
   }

   void packet_nack()
   {
      NewSerial.write(nack_packet, sizeof(nack_packet));
   }
}

/*
   stateful function
   if the funtion returns anything but 0
   then a new packet is in the raw_packet_buffer
*/
uint8_t get_new_raw_data()
{
   if ( NewSerial.available() == 0){
      return 0;
   }
   uint8_t ch = (uint8_t)NewSerial.read();
   switch (current_packet_mode){
      case in_zeros:{
         if ( ch != 0){
            raw_packet_buffer[0] = ch;
            packet_idx = 1;
            current_packet_mode = in_packet;
         }
         return 0;
      }
      case in_packet:{
         if ( ch != 0){
            if ( packet_idx < max_packet_length){
               raw_packet_buffer[packet_idx] = ch;
               ++packet_idx;
            }else{
               // error .. send nack?
               packet_nack();
               current_packet_mode = not_synced;  
            }
            return 0;  
         }else{ // got a new packet
            // return number of chars
            // send ack?
            packet_ack();
            return packet_idx;
         }
      }
      case not_synced:
      default:{
         if ( ch == 0){
            current_packet_mode = in_zeros;
         }
         return 0;
      }
   }
}

bool packet_mode()
{
   for (;;){
      uint8_t packet_length = get_new_raw_data();
      if (packet_length > 0){
         if (quan::uav::cobs::decode(raw_packet_buffer,packet_length,decoded_packet_buffer)){
            switch(decoded_packet_buffer[0]){
                  case cd_packet:
                     // not while file_open
                    // if (file.is_open
                     decoded_packet_buffer[quan::min(packet_length,max_packet_length - 2)] = 0;
                     
                  case new_log_packet:
                  // end the filename with a zero
                     decoded_packet_buffer[quan::min(packet_length,max_packet_length - 2)] = 0;
                  
                  case file_close_packet:
                  break;
                  case data_packet:
                  // if have file open
                  //verify data length
                   // count of chars in
                  // if chars in == 512
                  // sync
                  break;
               
            }
         }
      }
   }
   return true;
}