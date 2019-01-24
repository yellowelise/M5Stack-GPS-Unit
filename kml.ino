/*
  Fast GPS data logger.
  (c) 13th July 2016 A.G.Doswell

  License: The MIT License (See full license at the bottom of this file)

  The sketch uses a U-Blox 6M GPS satellite module connected to the hardware serial interface,
  a 128x64 SDD1306 OLED display module connected as an I2C device to pins A4 (SDA)& A5 (SCL) and an SD card interface
  connected to pins 4 (CS), 11 (MOSI), 12 (MISO) & 13 (SCK) The arduino used is a Nano with 3.3v ATMEGA328P.
  Warning - Many SD card interfaces are not 5V.
  There's a toggle switch which connects pin 9 to GND for Record.
  Data is written to the card an impressive 4 times per second, in real-time.

  The OzOled library is not quite "right" for this application, but works, and is lightweight and fast enough. Thanks Oscar!
  It is available here : http://blog.oscarliang.net/arduino-oled-display-library/
  TinyGPSPlus is available here :https://github.com/mikalhart/TinyGPSPlus

*/

#include <SPI.h>
#include <Wire.h>
#include <OzOLED.h>
#include <TinyGPS++.h>
#include <SD.h>

static const uint32_t GPSBaud = 57600;
int SatVal;                        // Number of satellites locked
int Speed;                         // Speed in MPH
char SpeedA [4];                   // Speed as char
TinyGPSPlus gps;                   // Feed gps data to TinySGPSPlus
File myFile;                       // Start SD
int speedDigit;                    // Number of digits in Speedo display
boolean card;                      // Is there a card present?
boolean rec = false;               // Is the record switch low? (false = high)
int recPin = 9;                    // Record pin (Pull low to record data)
boolean header = false;            // Has the header been written?
String filename ;                  // Concaternated filename +.kml
String filename1 ;                 // Raw GPS Time filename.
char fileNameChar [14];            // file name as char array

void setup()   {

  Serial.begin(GPSBaud);           // Start GPS serial comms
  pinMode (recPin, INPUT_PULLUP);  // recPin as input and pulled high
  OzOled.init();                   // initialze SDD1306 OLED display
  OzOled.sendCommand(0x8d);        // Set displays inbuilt inverter on
  OzOled.sendCommand(0x14);
  OzOled.setBrightness(0xFF);      // ... and brightness to max
  OzOled.clearDisplay();           // Clear the screen
  OzOled.setNormalDisplay();       // Set display to Normal mode
  pinMode(10, OUTPUT);             // CS for SD card, wether it likes it, or not.
  OzOled.clearDisplay();


  if (!SD.begin(4)) {              //Check SD card is present
    OzOled.printString("SD card fail    ", 0, 7);
    card = false;
  }
  else {
    OzOled.printString("SD card OK      ", 0, 7);
    card = true;
  }

  OzOled.printString("Sats:", 0, 0); // Set up display
  OzOled.printString("Speed:", 0, 1);
  OzOled.printString("MPH", 7, 6);
}

void loop() {
  while (Serial.available() > 0) //While GPS message received
    if (gps.encode(Serial.read()))
      displayInfo();
}

void displayInfo() {               // Display the data
  SatVal = int(gps.satellites.value());
  Speed = int(gps.speed.mph());
  itoa (Speed, SpeedA, 10);
  OzOled.printNumber ((long)SatVal, 6, 0);
  OzOled.printBigNumber (SpeedA, 7, 1);
  speedDigit = strlen(SpeedA);    // get length of Speed , and delete left behind zero if req'd
  if (speedDigit == 1) OzOled.printBigNumber(" ", 10, 1);
  
  if (digitalRead(recPin) == HIGH && card == true) { //If the record switch is high and the card is OK, write card data, and update display
    if (header == false) {
      if (gps.date.year() > 2000 && gps.location.age() < 1500) { // makes sure we have (at some stage since power up) a valid time, and the current fix is valid.{ // if the header hasn't been written, create a new filename with current time HHMMSSCC.kml
        String filename1(gps.time.value());
        String filename = filename1 + ".kml";
        filename.toCharArray(fileNameChar, 13);

      }
    }
    OzOled.printString("SD card OK REC", 0, 7);
  }
  if (digitalRead(recPin) == LOW && card == true ) { // if record switch is off, set flag and update display
    OzOled.printString("SD card OK    ", 0, 7);
  }

  if (digitalRead(recPin) == LOW && header == true) { // if record switch is off, but the header flag is set, write the footer.
    writeFooter();
  }

  if (card == true && digitalRead(recPin) == HIGH && (gps.speed.mph() >= 5)) {
    writeInfo(); //write the data to the SD card
  }
  smartDelay(200);

}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}

void writeInfo() { //Write the data to the SD card fomratted for google earth kml

  myFile = SD.open(fileNameChar, FILE_WRITE);
  if (header == false) { // If the header hasn't been written, write it.
    if (myFile) {
      myFile.print(F("<?xml version=\"1.0\" encoding=\"UTF-8\"?> <kml xmlns=\"http://earth.google.com/kml/2.0\"> <document>"));
      header = true; // header flag set.
    }
  }
  if (myFile) { // write current GPS position data as KML
    myFile.println(F("<placemark>"));
    myFile.print(F("<name>"));
    myFile.print(gps.speed.mph(), 1);
    myFile.println(F("</name>"));
    myFile.print(F("<point><coordinates>"));
    myFile.print(gps.location.lng(), 6);
    myFile.print(F(","));
    myFile.print(gps.location.lat(), 6);
    myFile.println(F("</coordinates></Point></Placemark>"));
    myFile.close(); // close the file:
  }
}

void writeFooter() { // if the record switch is now set to off, write the kml footer, and reset the header flag
  myFile = SD.open(fileNameChar, FILE_WRITE);
  header = false;
  if (myFile) {
    myFile.println(F("</Document>"));
    myFile.println (F("</kml>"));
    myFile.close ();
  }
}

/*
   Copyright (c) 2016 Andrew Doswell

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/


