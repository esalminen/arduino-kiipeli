#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <RGBLed.h>
#include <SoftwareSerial.h>

// Phonebook for allowed numbers
struct phoneBook {
    String no1 = "+358406646131";
    String no2 = "";
    String no3 = "";
    String no4 = "";
    String no5 = "";
  }

// LCD display 
const uint8_t rs = 12, en = 13, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
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
  // Read EEPROM phonenumberStorage
  EEPROM.get(phoneBook, 0);

  // Start sim communication
  sim.begin(9600);

  // LCD configuration no. of columns and rows
  lcd.begin(16,2);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i = 0; i < 10; i++)
  {
    Serial.println(phonenumberStorage[i]);
  }
}

}
