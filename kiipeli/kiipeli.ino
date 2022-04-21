#include <LiquidCrystal.h>
#include <RGBLed.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "Joystick.h"

//#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
#endif

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

// String storages
String simRead = "";
String numberSms = "";
String textSms = "";

// String commands
String openKeySmsCmd = "Kiipeli";
String registerKeyCmd = "abc123";
String clearEEPROMCmd = "clrEEPROM";

// Case flags
bool messageArrived = false;
bool messageParsed = false;
bool openSesame = false;
bool unauthorizedSms = false;
bool authorizeNumberSms = false;

// Time var
long currentTime = 0;

void setup() {
  
  // Read phonenumbers from EEPROM
  EEPROM.get(0, authorizedNumbers);

  // Activate serial transmitting in debug mode
  #ifdef DEBUG
    Serial.begin(9600);
    for(int i = 0; i < 10; i++)
    {
      DEBUG_PRINT(authorizedNumbers[i]);  
    }
  #endif

  for(int i = 0; i < 10; i++)
  {
    DEBUG_PRINT(authorizedNumbers[i]);  
  }

  // LCD configuration no. of columns and rows
  lcd.begin(16,2);
  lcd.print("Initializing...");

  // Start sim communication
  sim.begin(57600);
  delay(3000);

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
  #ifdef DEBUG  
  if(Serial.available())
  {
    while(Serial.available())
    {
      sim.write(Serial.read());  
    }  
  }
  #endif

  // Check sim messages
  updateSimSerial();
  if(messageArrived) parseMessage();
  if(messageParsed) handleParsedMessage();
  if(openSesame) handleFlag(openSesame, 1);
  if(authorizeNumberSms) handleFlag(authorizeNumberSms, 2);
  if(unauthorizedSms) handleFlag(unauthorizedSms, 3);
  
  //if(openSesame) handleOpenSesame();
  //if(authorizeNumberSms) handleAuthorizeSms();
  //if(unauthorizedSms) handleUnauthorizedSms();
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
    DEBUG_PRINT(simRead);
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
  DEBUG_PRINT("first quote: " + (String)indexOfSeparator);

  // Remove beginning before sender phonenumber
  tempStr.remove(0, indexOfSeparator + 1);

  // Find the end of sender phonenumber 
  indexOfSeparator = tempStr.indexOf('"');
  DEBUG_PRINT("second quote: " + (String)indexOfSeparator);

  // Save sender phonenumber to variable
  numberSms = tempStr.substring(0,indexOfSeparator);
  DEBUG_PRINT("Sender: ");Serial.println(numberSms);

  // Find beginning of sms from last quote
  indexOfSeparator = tempStr.lastIndexOf('"');
  DEBUG_PRINT("last quote: " + (String)indexOfSeparator);
  
  // Remove string until the last quote + cr + lf
  tempStr.remove(0, indexOfSeparator + 3);  
  DEBUG_PRINT("remainder text: " + tempStr);

  // Copy remaining string without last two cr + lf chars
  textSms = tempStr.substring(0, tempStr.length() - 2);
  DEBUG_PRINT("sms: " + textSms);
  
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
  
  if(textSms.equals(registerKeyCmd))
  {
      char number[14];
      numberSms.toCharArray(number, 14);
      authorizeNumberSms = addPhonenumberToList(number);
  }
  else if(isPhonenumberOnTheList(numberSms) && textSms.equals(openKeySmsCmd))
  {
      openSesame = true;
  }
  else if(isPhonenumberOnTheList(numberSms) && textSms.equals(clearEEPROMCmd))
  {
      clearEEPROM();
  }
  else if(!authorizeNumberSms)
  {
      unauthorizedSms = true;
  }
}

/**
  Handles event flag.
  @param bool &flag passed value by reference flag
  @param int ledColor 1=Green, 2=Yellow, 3=Red
*/
void handleFlag(bool &flag, int ledColor)
{
  if(!currentTime)
  {
    currentTime = millis(); 
  }
  if(currentTime)
  {
    switch(ledColor)
    {
      case 1:
        led.flash(RGBLed::GREEN, 100, 100);
        break;
      case 2:
        led.flash(RGBLed::YELLOW, 100, 100);
        break;  
      case 3:
        led.flash(RGBLed::RED, 100, 100);
        break; 
      default:
        break;
    }  
  }

  if(millis() >= currentTime + 5000)
  {
    led.off();
    currentTime = 0;
    flag = false;  
  }
}

/**
  Saves phonenumber to EEPROM
  @param char phonenumber[14] Phonenumber to be saved in EEPROM
  @return true if number was saved to EEPROM or is already on the EEPROM.
*/
bool addPhonenumberToList(char phonenumber[14])
{
   if(isPhonenumberOnTheList((String)phonenumber))
   {
      DEBUG_PRINT("Phonenumber " + (String)phonenumber + " was on the list. Not saved.");
      return true;
   }
   for(int i = 0; i < 10; i++)
   {
      String tempStr = authorizedNumbers[i];
      String countryCode = tempStr.substring(0,4);
      DEBUG_PRINT("Countrycode: " + (String)countryCode);
      if(!countryCode.equals("+358"))
      {
        DEBUG_PRINT("countryCode true when index " + String(i));
        strcpy(authorizedNumbers[i],phonenumber);
        EEPROM.put(0, authorizedNumbers);
        DEBUG_PRINT("Phonenumber was saved to index " + (String)i);
        return true;
      }
   }
   DEBUG_PRINT("Phonebook full. Number was not saved");
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

/**
  Fills phonebook in the EEPROM with zeroes
*/
void clearEEPROM()
{
    char emptyArray[10][14];
    for(int i = 0; i < 10; i++)
    {
      for(int j = 0; j < 14;j++)
      {
        emptyArray[i][j] = 0;  
      }
    }
    EEPROM.put(0, emptyArray);
}
