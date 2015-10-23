#include <LiquidCrystal.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

// RTC
#define DS1307_ADDRESS 0x68   // Address IC 0x68
byte zero = 0x00;
int _weekDay;
int _second, _minute, _hour;
int _monthDay, _month, _year;
int control, d1, d2;

// LCD
LiquidCrystal lcd(A0, 9, 5, 7, 6, 8);

// Switch datalog
#define LOG A1
#define Backlight A2
#define DEBOUNCE 300
#define CLAMP A3

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

// initial variable
int mode = 0;
int set_time = 1;
unsigned int set_second =      0;    // Set Second 0-59
unsigned int set_minute =      0;   // Set Minute 0-59
unsigned int set_hour =        0;    // Set Hour   0-23
unsigned int set_monthDay =    0;   // Set Day of Month 1-31
unsigned int set_month =       0;   // Set Month  1-12
unsigned int set_year  =       0;   // Set Year   0-99

unsigned long previousMillis = 0;
unsigned long previous = 0;
const long interval = 30000;

int addr = 0;
int periods = 0;
int pre = 0;

int line1 = 0;
int line2 = 0;
int line3 = 0;
float line_1 = 0.0;
float line_2 = 0.0;
float line_3 = 0.0;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pinMode(10, OUTPUT);

  lcd.begin(16, 2);

  pinMode(LOG, INPUT);
  pinMode(encoderPin1, INPUT_PULLUP);
  pinMode(encoderPin2, INPUT_PULLUP);
  pinMode(encoderSwitchPin, INPUT_PULLUP);

  pinMode(Backlight, OUTPUT);
  pinMode(CLAMP, OUTPUT);
  digitalWrite(Backlight, HIGH);

  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  while (!SD.begin(chipSelect)) {
    lcd.home();
    lcd.print("initialization  ");
    lcd.setCursor(0, 1);
    lcd.print(" SD Card failed!");
    delay(TIME);
  }

  lcd.home();
  lcd.print("  POWER METER   ");
  lcd.setCursor(0, 1);
  lcd.print(" SD Card done!  ");

  main_name();
  delay(TIME);

  debug();
  now_datetime();
  pre = EEPROM.read(9) + _minute;
  lcd.clear();

}

void loop()   {
  /******************* Main Display ****************************/
  while (mode == 0)  {

    if (!encoderValue) {
      Serial.print("None ");
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        digitalWrite(Backlight, LOW);
        previousMillis = currentMillis;
      }
    }
    else  {
      Serial.print("Done ");
      digitalWrite(Backlight, HIGH);
      encoderValue = !encoderValue;
    }

    if (digitalRead(encoderSwitchPin) == 0) {
      delay(DEBOUNCE);
      set_monthDay  = 0;
      set_month = 0;
      set_year  = 0;
      set_hour  = 0;
      set_minute  = 0;
      set_second  =  0;
      lcd.clear();
      mode = 1;
    }

//    unsigned long current = millis();
//    if (current - previous >= 60000) {
//      debug();
//      previous = current;
//    }

    now_datetime();
    Read_ARMS();
    lcd.setCursor(0, 0);
    lcd.print("  U    V    W  ");
    lcd.setCursor(1, 1);
    lcd.print(line1/10.0, 1);
    lcd.print("  ");
    lcd.setCursor(6, 1);
    lcd.print(line2/10.0, 1);
    lcd.print("  ");
    lcd.setCursor(11, 1);
    lcd.print(line3/10.0, 1);
    lcd.print("   ");

    Serial.print(_monthDay);
    Serial.print("/");
    Serial.print(_month);
    Serial.print("/20");
    Serial.print(_year);
    Serial.print(", ");
    Serial.print(_hour);
    Serial.print(":");
    Serial.print(_minute);
    Serial.print(":");
    Serial.print(_second);
    Serial.print("  Pre ");
    Serial.print(pre);
    Serial.print(" EEP ");
    Serial.println(EEPROM.read(9));

    if (pre >= 59)  {
      pre = pre - 59;
    }
    if (digitalRead(LOG) == 0 && pre == _minute)  {
      Read_ARMS();
      if (digitalRead(LOG) == 0)  {   // read clamp  and writing data to sd card
        line_1 = line1 / 10.0;
        line_2 = line2 / 10.0;
        line_3 = line3 / 10.0;
        writing(line_1, line_2, line_3);
        Serial.print(" Writing ");
        pre = EEPROM.read(9) + _minute; // update time
        debug();
      }
    }
  }

  /******************* Set date ****************************/
  while (mode == 1)  {
    digitalWrite(Backlight, HIGH);
    if (digitalRead(encoderSwitchPin) == 0) {
      delay(DEBOUNCE);
      lcd.clear();
      set_time ++;
    }
    if (set_time == 1) {
      Serial.print("monthDay");
      Serial.println(encoderValue);
      if (encoderValue <= 0 ) encoderValue = 0;
      if (encoderValue >= 31 ) encoderValue = 31;
      set_monthDay =  encoderValue;
    }
    if (set_time == 2) {
      Serial.print("month");
      Serial.println(encoderValue);
      if (encoderValue <= 0 ) encoderValue = 0;
      if (encoderValue >= 12 ) encoderValue = 12;
      set_month =  encoderValue;
    }
    if (set_time == 3) {
      Serial.print("year");
      Serial.println(encoderValue);
      if (encoderValue <= 0 ) encoderValue = 0;
      if (encoderValue >= 99 ) encoderValue = 99;
      set_year =  encoderValue;
    }
    if (set_time == 4)  {
      lcd.clear();
      set_time = 1;
      mode = 2;
    }

    lcd.setCursor(0, 0);
    lcd.print("Set            ");
    lcd.setCursor(0, 1);
    lcd.print(" Date : ");
    lcd.print(set_monthDay);
    lcd.print("/");
    lcd.print(set_month);
    lcd.print("/");
    lcd.print(set_year);
    lcd.print("   ");
  }

  /******************* Set time ****************************/
  while (mode == 2)  {
    if (digitalRead(encoderSwitchPin) == 0) {
      delay(DEBOUNCE);
      lcd.clear();
      set_time ++;
    }

    if (set_time == 1) {
      Serial.print("hour");
      Serial.println(encoderValue);
      if (encoderValue <= 0 ) encoderValue = 0;
      if (encoderValue >= 23) encoderValue = 23;
      set_hour =  encoderValue;
    }
    if (set_time == 2) {
      Serial.print("month");
      Serial.println(encoderValue);
      if (encoderValue <= 0 ) encoderValue = 0;
      if (encoderValue >= 59) encoderValue = 59;
      set_minute =  encoderValue;
    }
    if (set_time == 3) {
      Serial.print("year");
      Serial.println(encoderValue);
      if (encoderValue <= 0 ) encoderValue = 0;
      if (encoderValue >= 59 ) encoderValue = 59;
      set_second =  encoderValue;
    }
    if (set_time == 4)  {
      lcd.clear();
      set_time = 1;
      mode = 3;
    }

    lcd.setCursor(0, 0);
    lcd.print("Set            ");
    lcd.setCursor(0, 1);
    lcd.print(" Time : ");
    lcd.print(set_hour);
    lcd.print("/");
    lcd.print(set_minute);
    lcd.print("/");
    lcd.print(set_second);
    lcd.print("   ");
  }
  while (mode == 3) {
    Serial.print(set_hour);
    Serial.print(set_minute);
    Serial.print(set_second);
    Serial.print("    ");
    Serial.print(set_monthDay);
    Serial.print(set_month);
    Serial.println(set_year);

    setDateTime(set_second, set_minute, set_hour, set_monthDay, set_month, set_year);
    mode = 4;
  }
  while (mode == 4) {
    if (digitalRead(encoderSwitchPin) == 0) {
      delay(DEBOUNCE);
      lcd.clear();
      set_time ++;
    }

    if (set_time == 1) {
      Serial.print("periods");
      Serial.print(periods);
      Serial.print(" ");
      Serial.println(encoderValue);
      if (encoderValue <= 0) encoderValue = 0;
      if (encoderValue == 1) periods = 1;
      if (encoderValue == 2) periods = 2;
      if (encoderValue == 3) periods = 5;
      if (encoderValue == 4) periods = 10;
      if (encoderValue == 5) periods = 15;
      if (encoderValue == 6) periods = 20;
      if (encoderValue == 7) periods = 25;
      if (encoderValue == 8) periods = 30;
      if (encoderValue >= 8) encoderValue = 8;
    }
    if (set_time == 2)  {
      lcd.clear();
      set_time = 1;
      EEPROM.write(9, periods);
      now_datetime();
      pre = EEPROM.read(9) + _minute;
      digitalWrite(Backlight, HIGH);
      mode = 0;
    }

    lcd.setCursor(0, 0);
    lcd.print("Set time record");
    lcd.setCursor(0, 1);
    lcd.print(" Periods = ");
    lcd.print(periods);
    lcd.print(" S  ");
  }

} // END LOOP



void main_name() {
  myFile = SD.open(OUTPUT_FILE, FILE_WRITE);
  if (myFile)
  {
    myFile.print("Date");
    myFile.print(",");
    myFile.print("Time");
    myFile.print(",");
    myFile.print("Line1(A-RMS)");
    myFile.print(",");
    myFile.print("Line2(A-RMS)");
    myFile.print(",");
    myFile.println("Line3(A-RMS)");
    delay(TIME);
    myFile.close();
  }
}

void writing(float line1, float line2, float line3)  {
  now_datetime();
  myFile = SD.open(OUTPUT_FILE, FILE_WRITE);
  if (myFile)
  {
    myFile.print(_monthDay);
    myFile.print("/");
    myFile.print(_month);
    myFile.print("/");
    myFile.print(_year);
    myFile.print(",");
    myFile.print(_hour);
    myFile.print(":");
    myFile.print(_minute);
    myFile.print(":");
    myFile.print(_second);
    myFile.print(",");
    myFile.print(line1);
    myFile.print(",");
    myFile.print(line2);
    myFile.print(",");
    myFile.println(line3);
    delay(TIME);
    myFile.close();
  }
}

void updateEncoder() {
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded;
}

byte decToBcd(byte val)
{
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)
{
  // Convert binary coded decimal to normal decimal numbers
  return ( (val / 16 * 10) + (val % 16) );
}

/****** Set Date Time ***********************************/
void setDateTime(byte _second, byte _minute, byte _hour, byte _monthDay, byte _month, byte _year)
{
  /*******  Config to time *****************************/
  Serial.print("SETTIME ");
  Serial.print(_second);
  Serial.print(_minute);
  Serial.print(_hour);
  Serial.print(_monthDay);
  Serial.print(_month);
  Serial.print(_year);

  byte set_second =   _second;    // Set Second 0-59
  byte set_minute =   _minute;   // Set Minute 0-59
  byte set_hour   =   _hour;    // Set Hour   0-23
  byte set_monthDay =   _monthDay;   // Set Day of Month 1-31
  byte set_month    =   _month;   // Set Month  1-12
  byte set_year     =   _year;   // Set Year   0-99
  byte set_weekDay =     1;    // Set Day of Week  1-7

  //  byte set_weekDay =     2;    // Set Day of Week  1-7
  byte control  =    0b10010011;
  byte  d1;
  byte  d2;

  // Write to address ds1307
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);  //stop Oscillator

  Wire.write(decToBcd(set_second));
  Wire.write(decToBcd(set_minute));
  Wire.write(decToBcd(set_hour));
  Wire.write(decToBcd(set_weekDay));
  Wire.write(decToBcd(set_monthDay));
  Wire.write(decToBcd(set_month));
  Wire.write(decToBcd(set_year));

  Wire.write(control);
  Wire.write(zero);  //start
  Wire.endTransmission();

}

void now_datetime()
{
  //  Read from address ds1307
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 10);
  _second = bcdToDec(Wire.read());
  _minute = bcdToDec(Wire.read());
  _hour = bcdToDec(Wire.read() & 0b111111); // 24 hour time
  _weekDay = bcdToDec(Wire.read());         // 1-7 ->Sunday - Saturday
  _monthDay = bcdToDec(Wire.read());
  _month = bcdToDec(Wire.read());
  _year = bcdToDec(Wire.read());
  control = Wire.read();
}

