
bool command_disk()
{
   //Print card type
   NewSerial.print(F("\nCard type: "));
   switch(card.type()) {
      case SD_CARD_TYPE_SD1:
        NewSerial.println(F("SD1"));
        break;
      case SD_CARD_TYPE_SD2:
        NewSerial.println(F("SD2"));
        break;
      case SD_CARD_TYPE_SDHC:
        NewSerial.println(F("SDHC"));
        break;
      default:
        NewSerial.println(F("Unknown"));
   }

   //Print card information
   cid_t cid;
   if (!card.readCID(&cid)) {
     NewSerial.print(F("readCID failed"));
     return false;
   }

   NewSerial.print(F("Manufacturer ID: "));
   NewSerial.println(cid.mid, HEX);

   NewSerial.print(F("OEM ID: "));
   NewSerial.print(cid.oid[0]);
   NewSerial.println(cid.oid[1]);

   NewSerial.print(F("Product: "));
   for (byte i = 0; i < 5; i++) {
     NewSerial.print(cid.pnm[i]);
   }

   NewSerial.print(F("\n\rVersion: "));
   NewSerial.print(cid.prv_n, DEC);
   NewSerial.print(F("."));
   NewSerial.println(cid.prv_m, DEC);

   NewSerial.print(F("Serial number: "));
   NewSerial.println(cid.psn);

   NewSerial.print(F("Manufacturing date: "));
   NewSerial.print(cid.mdt_month);
   NewSerial.print(F("/"));
   NewSerial.println(2000 + cid.mdt_year_low + (cid.mdt_year_high <<4));

   csd_t csd;
   uint32_t cardSize = card.cardSize();
   if (cardSize == 0 || !card.readCSD(&csd)) {
     NewSerial.println(F("readCSD failed"));
     return false;
   }
   NewSerial.print(F("Card Size: "));
   cardSize /= 2; //Card size is coming up as double what it should be? Don't know why. Dividing it by 2 to correct.
   NewSerial.print(cardSize);
   //After division
   //7761920 = 8GB card
   //994816 = 1GB card
   NewSerial.println(F(" KB"));
   return true;
}