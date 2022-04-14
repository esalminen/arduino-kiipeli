#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <RGBLed.h>
#include <SoftwareSerial.h>

// LCD display 
const int rs = 12, en = 13, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Sim800L
const uint8_t rx = 10, tx = 11;
SoftwareSerial sim(10, 11);


String rawTextSms = "";
String textSms = "";
String numberSms = "";
bool messageArrived = false;
bool messageReadyForLCD = false;
bool initActive = true;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
