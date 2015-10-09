/*******************************************************************************
 * Project  : Clampmeter                                                       *
 * Compiler : Arduino 1.6.5                                                    *
 * Board    : Arduino UNO                                                      *
 * Device   : RTC and SD Card Sheild                                           *
 * Author   : Bavensky :3                                                      *
 * E-Mail   : Aphirak_Sang-ngenchai@hotmail.com                                *
 * Date     : 08/10/2015                                                       *
 *******************************************************************************/

#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

#define TIME 3000
#define OUTPUT_FILE "Datalog.csv"

File myFile;
const int chipSelect = 10;

RTC_DS1307 rtc;

char line[50];
int now_minute = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  pinMode(10, OUTPUT);

  while (!SD.begin(chipSelect))
  {
    Serial.println("initialization failed!");
    delay(TIME);
  }

  main_name();
  DateTime now = rtc.now();
  now_minute = now.minute();  // set minute now
}

void loop()   {
  DateTime now = rtc.now();
  if(now_minute > 59) now_minute = 0;
  if (now.minute() == now_minute)  {
    writing(now_minute);
    now_minute += 1;
  }
  
}

void main_name() {
  myFile = SD.open(OUTPUT_FILE, FILE_WRITE);
  if (myFile)
  {
    myFile.print("Date");
    myFile.print(",");
    myFile.print("Time");
    myFile.print(",");
    myFile.println("Watt");
    delay(TIME);
    myFile.close();
  }
}

void writing(int volume)
{
  myFile = SD.open(OUTPUT_FILE, FILE_WRITE);
  if (myFile)
  {
    DateTime now = rtc.now();
    sprintf(line, "%d/%d/%d,%d:%d:%d,%d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(), volume);
    myFile.println(line);
    delay(TIME);
    myFile.close();
  }
}


