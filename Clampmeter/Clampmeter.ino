#include <LiquidCrystal.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// RTC

// LCD
LiquidCrystal lcd(A0, 9, 5, 7, 6, 8);

// Switch datalog
#define LOG A1

// u SD Card
#define TIME 2000
File myFile;
#define OUTPUT_FILE "Datalog.csv"
const int chipSelect = 10;

// 1 rotary encoder
#define encoderPin1 2  
#define encoderPin2 3
#define encoderSwitchPin 4 //push button switch
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
long lastencoderValue = 0;
int lastMSB = 0;
int lastLSB = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(10, OUTPUT);

  lcd.begin(16, 2);
  
//  while (!SD.begin(chipSelect))
//  {
//    Serial.println("initialization failed!");
//    delay(TIME);
//  }

  pinMode(LOG, INPUT);
  pinMode(encoderPin1, INPUT_PULLUP); 
  pinMode(encoderPin2, INPUT_PULLUP);
  pinMode(encoderSwitchPin, INPUT_PULLUP);
  
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);
  
  lcd.home();
  lcd.print("  POWER METER  ");
  
//  main_name();
//  writing(1.23, 4.56, 7.89);
//  delay(TIME);
  lcd.clear();
}

void loop()   {
  lcd.home();
  lcd.print("  R    S    T  ");
  lcd.setCursor(0, 1);
  lcd.print(" 100  100  100 ");
  if(digitalRead(encoderSwitchPin) == 0){
    Serial.print(" Push ");
  }else{
    Serial.print(" Pull ");
  }
  Serial.print(digitalRead(LOG));
  Serial.println(encoderValue);
}

void main_name() {
  myFile = SD.open(OUTPUT_FILE, FILE_WRITE);
  if (myFile)
  {
    myFile.print("Date");
    myFile.print(",");
    myFile.print("Time");
    myFile.print(",");
    myFile.print("Line1");
    myFile.print(",");
    myFile.print("Line2");
    myFile.print(",");
    myFile.println("Line3");
    delay(TIME);
    myFile.close();
  }
}

void writing(float line1, float line2, float line3)  {

  myFile = SD.open(OUTPUT_FILE, FILE_WRITE);
  if (myFile)
  {
//    myFile.print(now.day());
//    myFile.print("/");
//    myFile.print(now.month());
//    myFile.print("/");
//    myFile.print(now.year());
//    myFile.print(",");
//    myFile.print(now.hour());
//    myFile.print(":");
//    myFile.print(now.minute());
//    myFile.print(":");
//    myFile.print(now.second());
//    myFile.print(",");
    myFile.print(line1);
    myFile.print(",");
    myFile.print(line2);
    myFile.print(",");
    myFile.println(line3);
    delay(TIME);
    myFile.close();
  }
}

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; 
}

