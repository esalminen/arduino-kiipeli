#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <RGBLed.h>
#include <SoftwareSerial.h>
#include "Joystick.h"

// Phonebook for authorized numbers
struct phoneBook {
    String no1 = "+358406646131";
    String no2 = "";
    String no3 = "";
    String no4 = "";
    String no5 = "";
    String no6 = "";
    String no7 = "";
    String no8 = "";
    String no9 = "";
    String no10 = "";
  };

// LCD display 
const uint8_t rs = 2, en = 4, d4 = 13, d5 = 12, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// RGB led
const uint8_t ledRedPin = 3, ledGreenPin = 5, ledBluePin = 6;
RGBLed led(ledRedPin, ledGreenPin, ledBluePin, RGBLed::COMMON_CATHODE);

// Sim800L
//const uint8_t rx = 10, tx = 11;
//SoftwareSerial sim(10, 11);

// Joystick
JoystickInputs joystickInputs;
Joystick joystick(A0, A1, 7, 100, 400);

String rawTextSms = "";
String textSms = "";
String numberSms = "";
bool messageArrived = false;
bool messageReadyForLCD = false;
bool initActive = true;

void setup() {
  // Read EEPROM phonenumberStorage
  //EEPROM.get(phoneBook, 0);

  // Start sim communication
  //sim.begin(9600);

  // LCD configuration no. of columns and rows
  lcd.begin(16,2);
  lcd.print("hello, world!");

  // Joystick setup
  joystick.begin();
}

void loop() {
  // Read joystick inputs
  joystickInputs = joystick.readInputs();

  // Test colors with joystick
  if(joystickInputs.up)
  {
    led.setColor(RGBLed::RED);  
  }
  else if(joystickInputs.down)
  {
    led.setColor(RGBLed::GREEN);  
  } else if(joystickInputs.left)
  {
    led.setColor(RGBLed::BLUE);  
  } else if(joystickInputs.right)
  {
    led.setColor(RGBLed::YELLOW);  
  } else if(joystickInputs.buttonPressed)
  {
    led.setColor(RGBLed::WHITE);  
  } else
  {
    led.off();  
  }
}
