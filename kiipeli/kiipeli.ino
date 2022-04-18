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

String simRead = "";
String numberSms = "";
String textSms = "";
bool messageArrived = false;
bool messageParsed = false;

void setup() {  
  // LCD configuration no. of columns and rows
  lcd.begin(16,2);
  lcd.print("Initializing...");

  // Start sim communication
  sim.begin(9600);
  delay(1000);

  // Handshake test with Sim800L
  sim.println("AT");
  delay(500);
  updateSimSerial();
  lcd.clear();
  lcd.print(String("Handshake: " + simRead.substring(simRead.length()-4, simRead.length()-2)));
  delay(3000);

  // SMS-mode configuration
  sim.println("AT+CMGF=1");
  delay(500);
  updateSimSerial();
  lcd.clear();
  lcd.print(String("Mode Config: " + simRead.substring(simRead.length()-4, simRead.length()-2)));
  delay(3000);

  // SMS-receive configuration
  sim.println("AT+CNMI=1,2,0,0,0");
  delay(500);
  updateSimSerial();
  lcd.clear();
  lcd.print(String("Rcv Config: " + simRead.substring(simRead.length()-4, simRead.length()-2)));
  delay(3000);

  // Init completed
  lcd.clear();
  lcd.print("Init completed");
  delay(3000);
  lcd.clear();

  // Joystick setup
  joystick.begin();

  // Reset messagearrived-flag from init messages
  messageArrived = false;
}

void loop() {
  // Read joystick inputs
  joystickInputs = joystick.readInputs();

  // Check sim messages
  updateSimSerial();
  if(messageArrived) parseMessage();
  if(messageParsed) handleParsedMessage();
  delay(500);

}

/**
  Checks if there is any data in the sim-module buffer.
*/
void updateSimSerial()
{
  if (sim.available())
  {
    simRead = "";
    while (sim.available())
    {
      simRead += (char)sim.read();
    }
    messageArrived = true;
  }
}

/**
  Parses sms-sender number and sms-message to variables.
*/
void parseMessage()
{
  numberSms = simRead.substring(9,22); // Get sender phonenumber from simRead string
  textSms = simRead.substring(51, simRead.length() - 2); // Get sms message from simRead string
  messageArrived = false;
  messageParsed = true;
}

/**
  Handles parsed sms-message.
*/
void handleParsedMessage()
{
  messageParsed = false;
  lcd.clear();
  lcd.print(numberSms);
  lcd.setCursor(0,1);
  lcd.print(textSms);
}
