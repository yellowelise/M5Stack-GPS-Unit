#include <M5Stack.h>
#include <TinyGPS++.h>
#define M5STACKFIRE_SPEAKER_PIN 25 // speaker DAC, only 8 Bit

static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;

// The serial connection to the GPS device
HardwareSerial ss(2);


// For stats that happen every 5 seconds
unsigned long last = 0UL;



  char filename[] = "/LOGGER00.CSV";
  String dataToWrite = "";

  
 File logfile;


void saveData()
{
if (dataToWrite != "")
{  
if(SD.exists(filename)){ // check the card is still there
// now append new data file
logfile = SD.open(filename, FILE_APPEND );
if (logfile){
logfile.println(dataToWrite);
logfile.close(); // close the file
dataToWrite = "";

}
}
else{
  logfile = SD.open(filename, FILE_WRITE );
if (logfile){
logfile.println("DATE,LAT,LNG,HDOP,COURSE,SPEED,ALTITUDE");
logfile.println(dataToWrite);
logfile.close(); // close the file
dataToWrite = "";

}
Serial.println("Error writing to file !");
}
}
}

void setup()
{
 
  M5.begin();
  dacWrite(M5STACKFIRE_SPEAKER_PIN, 0); // make sure that the speaker is quite
  Wire.begin();    
  
  M5.Lcd.setTextColor(GREEN, BLACK);
  ss.begin(GPSBaud);
  M5.Lcd.println(F("Set Baud to 57600..."));
  ss.println("$PCAS01,4*18");
  ss.flush();
  ss.begin(57600);
  
  M5.Lcd.println(F("Set Frequency to 10Hz..."));
  ss.println("$PCAS02,100*1E");

  M5.Lcd.println(F("Set Costellation to GPS,BDS,GLONASS..."));
  ss.println("$PCAS04,7*1E");

  
  M5.Lcd.println(F("Set Receiver to Automotive..."));
  ss.println("$PCAS11,3*1E");

  M5.Lcd.println(F("Set Reboot to HOT..."));
  ss.println("$PCAS10,0*1C");


  M5.Lcd.println(F("Inizialization terminated."));

   /* ss.write("$PCAS01,5*19");
  delay(200);
  ss.flush();
  ss.end();
  ss.begin(115200);
  */
  // 2hz ss.println("$PCAS02,500*1A");

/*  M5.Lcd.println(F("GPSUnitExample.ino"));
  M5.Lcd.println(F("Based on TinyGPS++ KitchenSink.ino (by Mikal Hart)"));
  M5.Lcd.print(F("Testing TinyGPS++ library v. ")); M5.Lcd.println(TinyGPSPlus::libraryVersion());
  M5.Lcd.println(F(""));
  M5.Lcd.println();
*/
  Serial.begin(115200);

        
}

void loop()
{
  //M5.Lcd.clear();
 
  // Dispatch incoming characters
  while (ss.available() > 0)
  {
    int ch = ss.read();
    Serial.write(ch);
    gps.encode(ch);
  }
  if (gps.location.isUpdated())
  {
    M5.Lcd.setCursor(0, 70);
    M5.Lcd.print(F("LOCATION  Fix Age: "));
    M5.Lcd.println(gps.location.age());

    M5.Lcd.setCursor(0, 150);
    M5.Lcd.print(F("LAT: "));
    M5.Lcd.print(gps.location.lat(), 6);
    M5.Lcd.print(F(" - LNG: "));
    M5.Lcd.println(gps.location.lng(), 6);
       
    static char str[30];
    sprintf(str, "%04d-%02d-%02d %02d:%02d.%03d", String(gps.date.year()),String(gps.date.month()),String(gps.date.day()),String(gps.time.hour()),String(gps.time.minute()),String(gps.time.second()),String(gps.time.centisecond()));
    dataToWrite = String(str) + "," + String(gps.location.lat(),6) + "," + String(gps.location.lng(),6) + "," + String(gps.hdop.hdop()) + "," + String(gps.course.deg()) + "," + String(gps.speed.kmph()) +","+String(gps.altitude.meters());
    M5.Lcd.setCursor(0, 190);
    M5.Lcd.println(dataToWrite);
    saveData();
    
  }

 if (gps.date.isUpdated())
  {
    M5.Lcd.setCursor(0, 80);
    //M5.Lcd.print(F("DATE       Fix Age="));
    //M5.Lcd.print(gps.date.age());
//    M5.Lcd.print(F("ms Raw="));
//    M5.Lcd.print(gps.date.value());
    M5.Lcd.print(F("DATE: "));
    M5.Lcd.print(gps.date.year());
    M5.Lcd.print(F("/"));
    M5.Lcd.print(gps.date.month());
    M5.Lcd.print(F("/"));
    M5.Lcd.println(gps.date.day());
  }

 if (gps.time.isUpdated())
  {
    M5.Lcd.setCursor(0, 90);
    M5.Lcd.print(F("TIME: "));
//    M5.Lcd.print(gps.time.age());
//    M5.Lcd.print(F("ms Raw="));
//    M5.Lcd.print(gps.time.value());
//    M5.Lcd.print(F(" Hour="));
    M5.Lcd.print(gps.time.hour());
    M5.Lcd.print(F(":"));
    M5.Lcd.print(gps.time.minute());
    M5.Lcd.print(F(":"));
    M5.Lcd.print(gps.time.second());
    M5.Lcd.print(F("."));
    M5.Lcd.println(gps.time.centisecond());
  }

 
 if (millis() - last > 5000)
  {
     M5.Lcd.setCursor(0, 160);
//    M5.Lcd.print(F("DIAGS Chars: "));
//    M5.Lcd.print(gps.charsProcessed());
//    Serial.println(gps.charsProcessed());

    M5.Lcd.print(F("Sentences-with-Fix: "));
    M5.Lcd.print(gps.sentencesWithFix());
    M5.Lcd.print(F(" Failed-checksum: "));
    M5.Lcd.print(gps.failedChecksum());
    M5.Lcd.print(F(" Passed-checksum: "));
    M5.Lcd.println(gps.passedChecksum());

    if (gps.charsProcessed() < 10)
      M5.Lcd.println(F("WARNING: No GPS data.  Check wiring."));

    last = millis();
    M5.Lcd.println();
  }
}