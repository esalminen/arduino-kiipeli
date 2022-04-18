#include <LiquidCrystal.h>
#include <RGBLed.h>
#include <SoftwareSerial.h>
#include "Joystick.h"

// LCD display 
const uint8_t rs = 2, en = 4, d4 = 13, d5 = 12, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// RGB led
const uint8_t ledRedPin = 3, ledGreenPin = 5, ledBluePin = 6;
RGBLed led(ledRedPin, ledGreenPin, ledBluePin, RGBLed::COMMON_CATHODE);

// Sim800L
const uint8_t rx = 10, tx = 11;
SoftwareSerial sim(10, 11);

// Joystick
JoystickInputs joystickInputs;
const uint8_t xAxis = A0, yAxis = A1, buttonPin = 7;
Joystick joystick(xAxis, yAxis, buttonPin, 100, 400);

String rawTextSms = "";
String textSms = "";
String numberSms = "";
bool messageArrived = false;
bool messageReadyForLCD = false;
bool initActive = true;

void setup() {
  // Start sim communication
  sim.begin(9600);

  // LCD configuration no. of columns and rows
  lcd.begin(16,2);
  lcd.print("hello, world!");

  // Joystick setup
  joystick.begin();
}

void loop() {
  // Read joystick inputs
  joystickInputs = joystick.readInputs();

}
