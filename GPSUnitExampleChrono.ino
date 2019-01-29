#include <M5Stack.h>
#include <TinyGPS++.h>
#include <Math.h>

#define M5STACKFIRE_SPEAKER_PIN 25 // speaker DAC, only 8 Bit
#include "Array.h"
// fine viale tirreno 41.93713,12.529524 -- 41.9367835,12.5313038
// viale ionio 41.945960, 12.531180 -- 41.945481, 12.531126
// via delle vigne nuove 41.952806, 12.535804 -- 41.952917, 12.536549
// casa 41.969688, 12.532461 -- 41.969800, 12.532651


static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;

const byte size = 10;
byte Index =0;
float rawVel[size] = {0,0,0,0,0,0,0,0,0,0};
float rawLat[size] = {0,0,0,0,0,0,0,0,0,0};
float rawLng[size] = {0,0,0,0,0,0,0,0,0,0};
float prev_latitude;
float prev_longitude;
String intersect_str = "      ";


struct point { float x, y; };
struct line  { struct point p1, p2; };



// The serial connection to the GPS device
HardwareSerial ss(2);


// For stats that happen every 5 seconds
unsigned long last = 0UL;



  String filename = "/LOGGER00.CSV";
  String dataToWrite = "";

 File logfile;


int ccw (struct point p0,struct point p1,struct point p2)
{
   float dx1,dx2,dy1,dy2;

   dx1 = p1.x-p0.x; 
   dy1 = p1.y-p0.y;
   dx2 = p2.x-p0.x; 
   dy2 = p2.y-p0.y;

   if (dx1*dy2 > dy1*dx2) return +1;
   if (dx1*dy2 < dy1*dx2) return -1;
   if ((dx1*dx2 < 0) || (dy1*dy2 < 0)) return -1; 
   if ((dx1*dx1+dy1*dy1) < (dx2*dx2+dy2*dy2)) return +1;
   return 0;   
}

int intersect(struct line l1,struct line l2)
{
   return ((ccw(l1.p1,l1.p2,l2.p1)*ccw(l1.p1,l1.p2,l2.p2)) <=0) &&
          ((ccw(l2.p1,l2.p2,l1.p1)*ccw(l2.p1,l2.p2,l1.p2)) <=0);
}



void saveData()
{
if (dataToWrite != "")
{  
  if(SD.exists(filename)){ // check the card is still there
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
      logfile.println("DATE,TIME,AGE,LAT,LATM,LNG,LNGM,HDOP,COURSE,SPEED,SPEEDM,ALTITUDE,SAT,INTERSECT");
      logfile.println(dataToWrite);
      logfile.close(); // close the file
      dataToWrite = "";
    }
}
}
}




void connectToGPS()
{
  Serial.begin(115200);  

  M5.Lcd.clear();
  
  ss.begin(GPSBaud);
  delay(1000);
  M5.Lcd.println(F("Set Baud to 57600..."));
  ss.println("$PCAS01,4*18");//57600
  delay(1000);
  ss.flush(); 
  delay(1000);
  ss.end();
  delay(1000);
  ss.begin(57600);
  delay(1000);
 // ss.println("$PCAS01,3*1F");//38400
  //ss.updateBaudRate(57600);
  //delay(5);
  //ss.end();
  //delay(2000);
  //ss.begin(57600);
  
  delay(250);
  M5.Lcd.println(F("Set Frequency to 10Hz..."));
  ss.println("$PCAS02,100*1E");

  delay(250);
  M5.Lcd.println(F("Set Costellation to GPS,BDS,GLONASS..."));
  ss.println("$PCAS04,7*1E");

  
  delay(250);
  M5.Lcd.println(F("Set Receiver to Automotive..."));
  ss.println("$PCAS11,3*1E");

  delay(250);
  M5.Lcd.println(F("Set Reboot to HOT..."));
  ss.println("$PCAS10,0*1C");


  M5.Lcd.println(F("Inizialization terminated."));
}


void setup()
{
 
  M5.begin();
  connectToGPS();  
  M5.Lcd.setTextColor(GREEN, BLACK);
}

void loop()
{


line l1;
line l2;
line l3;
line l4;
line l5;

l1.p1.x = 41.93713;
l1.p1.y = 12.529524;
l1.p2.x = 41.9367835;
l1.p2.y = 12.5313038;

// viale ionio 41.945960, 12.531180 -- 41.945481, 12.531126
l3.p1.x = 41.945960;
l3.p1.y = 12.531180;
l3.p2.x = 41.945481;
l3.p2.y = 12.531126;

// via delle vigne nuove 41.952806, 12.535804 -- 41.952917, 12.536549
l4.p1.x = 41.952806;
l4.p1.y = 12.535804;
l4.p2.x = 41.952917;
l4.p2.y = 12.536549;

// casa 41.969688, 12.532461 -- 41.969800, 12.532651
l5.p1.x = 41.969688;
l5.p1.y = 12.532461;
l5.p2.x = 41.969800;
l5.p2.y = 12.532651;





  M5.update();  
  if (M5.BtnA.pressedFor(700))
    M5.powerOFF();
  while (ss.available() > 0)
  {
    int ch = ss.read();
    Serial.write(ch);
    gps.encode(ch);
  }
  
  if (gps.location.isValid())
  {
    M5.Lcd.setCursor(0, 70);
    M5.Lcd.print(F("LOCATION  Fix Age: "));
    M5.Lcd.println(gps.location.age());

    M5.Lcd.setCursor(0, 150);
    M5.Lcd.print(F("LAT: "));
    M5.Lcd.print(gps.location.lat(), 6);
    M5.Lcd.print(F(" - LNG: "));
    M5.Lcd.println(gps.location.lng(), 6);
       
    if (gps.location.age()<50)
    {
      Index++;
      if (Index>=size)
          Index = 0;
  
      rawVel[Index] =  gps.speed.kmph();
      rawLat[Index] =  gps.location.lat();
      rawLng[Index] =  gps.location.lng();
      
      Array<float> velocity = Array<float>(rawVel,size);
      Array<float> latitude = Array<float>(rawLat,size);
      Array<float> longitude = Array<float>(rawLng,size);

      if (Index==0)
        {
          prev_latitude = latitude.getAverage();
          prev_longitude = longitude.getAverage();
        }
  


      l2.p1.x = prev_latitude;
      l2.p1.y = prev_longitude;
      l2.p2.x = latitude.getAverage();
      l2.p2.y = longitude.getAverage();

      
      if (intersect(l2,l1) == 1)
        intersect_str = "int_L1";
      else if (intersect(l2,l3) == 1)
        intersect_str = "int_L3";
      else if (intersect(l2,l4) == 1)
        intersect_str = "int_L4";
      else if (intersect(l2,l5) == 1)
        intersect_str = "int_L5";
       
      dataToWrite = String(gps.date.value())+","+String(gps.time.value()) + ","+String(gps.location.age())+"," + String(gps.location.lat(),6) +"," + String(latitude.getAverage(),6) + "," + String(gps.location.lng(),6) + "," + String(longitude.getAverage(),6) + "," + String(gps.hdop.hdop()) + "," + String(gps.course.deg()) + "," +String(gps.speed.kmph())+"," + String(velocity.getAverage()) +","+String(gps.altitude.meters())+","+String(gps.satellites.value())+"," + intersect_str ;
      M5.Lcd.setCursor(0, 190);
      M5.Lcd.println(dataToWrite);
      saveData();
      
    }
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
    filename = "/"+String(gps.date.value())+"_"+String(random(0,999)) + ".CSV";
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
    M5.Lcd.print(F("Sentences-with-Fix: "));
    M5.Lcd.print(gps.sentencesWithFix());
    M5.Lcd.print(F(" Failed-checksum: "));
    M5.Lcd.print(gps.failedChecksum());
    M5.Lcd.print(F(" Passed-checksum: "));
    M5.Lcd.println(gps.passedChecksum());

    if (gps.charsProcessed() < 10)
    {
      connectToGPS();
      M5.Lcd.println(F("WARNING: No GPS data.  Check wiring."));
    }
    last = millis();
   
  }
}