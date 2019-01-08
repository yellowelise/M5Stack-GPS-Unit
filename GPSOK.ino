#include <M5Stack.h>
#include <TinyGPS++.h>

/*
   This sample code demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
HardwareSerial ss(2);

void setup()
{
  M5.begin();
  ss.begin(GPSBaud);

  // M5.Lcd.println(F("FullExample.ino"));
  // M5.Lcd.println(F("An extensive example of many interesting TinyGPS++ features"));
  // M5.Lcd.print(F("Testing TinyGPS++ library v. ")); M5.Lcd.println(TinyGPSPlus::libraryVersion());
  // M5.Lcd.println(F("by Mikal Hart"));
  // M5.Lcd.println();
  //M5.Lcd.println(F("Sats HDOP Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
  //M5.Lcd.println(F("          (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail"));
  //M5.Lcd.println(F("---------------------------------------------------------------------------------------------------------------------------------------"));
}

void loop()
{
  M5.Lcd.setCursor(0, 70);
  M5.Lcd.setTextColor(GREEN, BLACK);
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;

  if (gps.satellites.isValid())
    M5.Lcd.println("SAT: " + String(gps.satellites.value()));//, , 5);
  if (gps.hdop.isValid())
    M5.Lcd.println("HDOP: " + String(gps.hdop.value()));
  if (gps.location.isValid())
  {
    M5.Lcd.println("LAT: " + String(gps.location.lat()));
    M5.Lcd.println("LNG: " + String(gps.location.lng()));
  }
  if (gps.location.isValid())
    M5.Lcd.println("AGE: " + gps.location.age());

  printDateTime(gps.date,gps.time);
  M5.Lcd.println("");
  if (gps.altitude.isValid())
    M5.Lcd.println("ALT: " + String(gps.altitude.meters()));
  
  //printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  if (gps.speed.isValid())
    M5.Lcd.println("Km/h: " + String(gps.speed.kmph()));
  
  //printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);

  /*unsigned long distanceKmToLondon =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON) / 1000;
  printInt(distanceKmToLondon, gps.location.isValid(), 9);

  double courseToLondon =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON);

  printFloat(courseToLondon, gps.location.isValid(), 7, 2);

  const char *cardinalToLondon = TinyGPSPlus::cardinal(courseToLondon);

  printStr(gps.location.isValid() ? cardinalToLondon : "*** ", 6);

  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  M5.Lcd.println();
  */
  smartDelay(100);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    M5.Lcd.println(F("No GPS data received: check wiring"));
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  char ccc;
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      ccc = ss.read();
      gps.encode(ccc);
      Serial.print(ccc);
    }
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      M5.Lcd.print('*');
    M5.Lcd.print(' ');
  }
  else
  {
    M5.Lcd.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      M5.Lcd.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "                 ";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  M5.Lcd.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    M5.Lcd.print(F("             "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    M5.Lcd.print(sz);
  }
  
  if (!t.isValid())
  {
    M5.Lcd.print(F("         "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    M5.Lcd.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    M5.Lcd.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}
