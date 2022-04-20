#include <LiquidCrystal.h>
#include <RGBLed.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "Joystick.h"

// Data structure for authorized numbers
char authorizedNumbers[10][14];

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
bool openSesame = false;
long currentTime = 0;

void setup() {  

  // Read phonenumbers from EEPROM
  EEPROM.get(0, authorizedNumbers);

  Serial.begin(9600);
  for(int i = 0; i < 10; i++)
  {
    Serial.println(authorizedNumbers[i]);  
  }

  // LCD configuration no. of columns and rows
  lcd.begin(16,2);
  lcd.print("Initializing...");

  // Start sim communication
  sim.begin(57600);
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

  // Debugging commands to sim from serial
  if(Serial.available())
  {
    while(Serial.available())
    {
      sim.write(Serial.read());  
    }  
  }

  // Check sim messages
  updateSimSerial();
  if(messageArrived) parseMessage();
  if(messageParsed) handleParsedMessage();
  if(openSesame) handleOpenSesame();
  delay(20);
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
    Serial.println(simRead);
    messageArrived = true;
  }
}

/**
  Parses sms-sender number and sms-message to variables.
*/
void parseMessage()
{ 
  // ASCII Incoming sms: 131043677784583234435153565248545452544951493444343444345050474852474957445051584856584955434950341310751051051121011081051310
  // CHAR  example       CRLF + C M T :   " + 3 5 8 4 0 6 6 4 6 1 3 1 " , " " , " 2 2 / 0 4 / 1 9 , 2 3 : 0 8 : 1 7 + 1 2 "CRLF K  i  i  p  e  l  iCRLF
  
  String tempStr = simRead;

  // Find the beginning of sender phonenumber
  int indexOfSeparator = tempStr.indexOf('"');
  //Serial.println("ensimmainen lainausmerkki: " + (String)indexOfSeparator);

  // Remove beginning before sender phonenumber
  tempStr.remove(0, indexOfSeparator + 1);

  // Find the end of sender phonenumber 
  indexOfSeparator = tempStr.indexOf('"');
  //Serial.println("toinen lainausmerkki: " + (String)indexOfSeparator);

  // Save sender phonenumber to variable
  numberSms = tempStr.substring(0,indexOfSeparator);
  //Serial.print("Lahettaja: ");Serial.println(numberSms);

  // Find beginning of sms from last quote
  indexOfSeparator = tempStr.lastIndexOf('"');
  //Serial.println("viimeinen lainausmerkki: " + (String)indexOfSeparator);
  
  // Remove string until the last quote + cr + lf
  tempStr.remove(0, indexOfSeparator + 3);  
  //Serial.println("jaljelle jai: " + tempStr);

  // Copy remaining string without last two cr + lf chars
  textSms = tempStr.substring(0, tempStr.length() - 2);
  //Serial.println("sms: " + textSms);
  
  messageArrived = false;
  messageParsed = true;
}

/**
  Handles parsed sms-message.
*/
void handleParsedMessage()
{
  messageParsed = false;
  if(textSms.equals("123"))
  {
      char number[14];
      numberSms.toCharArray(number, 14);
      bool saved = addPhonenumberToList(number);
  }
  lcd.clear();
  lcd.print(numberSms);
  lcd.setCursor(0,1);
  lcd.print(textSms);
  openSesame = true;
}

/**
  Handles open sesame flag.
*/
void handleOpenSesame()
{
  if(!currentTime)
  {
    currentTime = millis();  
  }

  if(currentTime)
  {
    led.flash(RGBLed::YELLOW, 100, 100);   
  }

  if(millis() >= currentTime + 5000)
  {
    led.off();
    currentTime = 0;
    openSesame = false;  
  }
}

/**
  Saves phonenumber to EEPROM
  @param char phonenumber[14] Phonenumber to be saved in EEPROM
  @return true if number was saved to EEPROM. Otherwise false.
*/
bool addPhonenumberToList(char phonenumber[14])
{
   for(int i = 0; i < 10; i++)
   {
      String tempStr = authorizedNumbers[i];
      String countryCode = tempStr.substring(0,4);
      Serial.print("Countrycode: ");Serial.println(countryCode);
      if(!countryCode.equals("+358"))
      {
        Serial.print("countryCode true when index ");
        Serial.println(i);
        strcpy(authorizedNumbers[i],phonenumber);
        EEPROM.put(0, authorizedNumbers);
        return true;
      }
   }
   return false;
}

/**
  Check if phoneNumber is int the authorized number list
  @param char phonenumber[14] Phonenumber to be checked
  @return true if number is authorized. Otherwise false.
*/
bool isPhonenumberOnTheList(String phonenumber)
{
    for(int i = 0; i < 10; i++)
   {
      String tempStr = authorizedNumbers[i];
      if(tempStr.equals(phonenumber))
      {
        return true;
      }
   }
   return false;
}
