#include <LiquidCrystal.h>
#include <RGBLed.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "Joystick.h"

#define DEBUG // Comment this line to stop debug prints

// Handle debug printing with precompiler
#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#endif

// Data structure for authorized numbers
char authorizedNumbers[10][14];

// LCD display
const uint8_t rs = 12, en = 13, d4 = 5, d5 = 6, d6 = 7, d7 = 8, contrast = 3;
const uint8_t contrastSetting = 125;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// RGB led
const uint8_t ledRedPin = 9, ledGreenPin = 10, ledBluePin = 11;
RGBLed led(ledRedPin, ledGreenPin, ledBluePin, RGBLed::COMMON_CATHODE);

// Sim800L
const uint8_t rx = 2, tx = 4;
SoftwareSerial sim(rx, tx);

// Joystick
JoystickInputs joystickInputs;
const uint8_t xAxis = A0, yAxis = A1, buttonPin = A2;
Joystick joystick(xAxis, yAxis, buttonPin, 100, 400);

// String storages
String simRead = "";
String numberSms = "";
String textSms = "";

// String commands
String openKeySmsCmd = "Kiipeli";
String registerKeyCmd = "abc123";
String clearEEPROMCmd = "clrEEPROM";
String resetArduinoCmd = "rstKiipeli";

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
  Serial.println("Authorized numbers list:");
  for (int i = 0; i < 10; i++)
  {
    Serial.println(authorizedNumbers[i]);
  }
#endif

  // LCD configuration no. of columns and rows
  lcd.begin(16, 2);
  analogWrite(contrast, contrastSetting); // set contrast of the lcd

  // Start sim communication
  lcd.print("Wake sim800...");
  sim.begin(57600);
  delay(1000);

  // Handshake test with Sim800L
  String testResult = "";
  lcd.clear();
  lcd.print("Handshake: ");
  while (!testResult.equals("OK"))
  {
    sim.println("AT");
    delay(500);
    updateSimSerial();
    testResult = simRead.substring(simRead.length() - 4, simRead.length() - 2);
  }

  lcd.print(" " + testResult);
  testResult = "";
  delay(1000);

  // SMS-mode configuration
  lcd.clear();
  lcd.print("Mode Config: ");
  while (!testResult.equals("OK"))
  {
    sim.println("AT+CMGF=1");
    delay(500);
    updateSimSerial();
    testResult = simRead.substring(simRead.length() - 4, simRead.length() - 2);
  }

  lcd.print(" " + testResult);
  testResult = "";
  delay(1000);

  // SMS-receive configuration
  lcd.clear();
  lcd.print("Rcv Config: ");
  while (!testResult.equals("OK"))
  {
    sim.println("AT+CNMI=1,2,0,0,0");
    delay(500);
    updateSimSerial();
    testResult = simRead.substring(simRead.length() - 4, simRead.length() - 2);
  }
  lcd.print(" " + testResult);
  testResult = "";
  delay(1000);

  // Init completed
  lcd.clear();
  lcd.print("Init completed");
  delay(1000);
  lcd.clear();

  // Joystick setup
  joystick.begin();

  // Reset messagearrived-flag from init messages
  messageArrived = false;
}

/*
   Arduino software reset function
*/
void(* resetFunc) (void) = 0;

void loop() {
  // Read joystick inputs
  joystickInputs = joystick.readInputs();

  // Debugging commands to sim from serial
  // TODO: simulate sms from serial
#ifdef DEBUG
  if (Serial.available())
  {
    while (Serial.available())
    {
      sim.write(Serial.read());
    }
  }
#endif

  // Check sim messages
  updateSimSerial();
  if (messageArrived) parseMessage();
  if (messageParsed) handleParsedMessage();
  if (openSesame) handleFlag(openSesame, 1);
  if (authorizeNumberSms) handleFlag(authorizeNumberSms, 2);
  if (unauthorizedSms) handleFlag(unauthorizedSms, 3);
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
  // Incoming message looks like following in ASCII numbers and letters:
  // ASCII Incoming sms: 131043677784583234435153565248545452544951493444343444345050474852474957445051584856584955434950341310751051051121011081051310
  // CHAR  example       CRLF + C M T :   " + 3 5 8 4 0 6 6 4 6 1 3 1 " , " " , " 2 2 / 0 4 / 1 9 , 2 3 : 0 8 : 1 7 + 1 2 "CRLF K  i  i  p  e  l  iCRLF

  // Save to temp variable
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
  numberSms = tempStr.substring(0, indexOfSeparator);
  DEBUG_PRINT("Sender: "); Serial.println(numberSms);

  // Find beginning of sms from last quote
  indexOfSeparator = tempStr.lastIndexOf('"');
  DEBUG_PRINT("last quote: " + (String)indexOfSeparator);

  // Remove string until the last quote + cr + lf
  tempStr.remove(0, indexOfSeparator + 3);
  DEBUG_PRINT("remainder text: " + tempStr);

  // Copy remaining string without last two cr + lf chars
  textSms = tempStr.substring(0, tempStr.length() - 2);
  DEBUG_PRINT("sms: " + textSms);

  // Turn flags accordingly
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
  lcd.setCursor(0, 1);
  lcd.print(textSms);


  // Check if sender number is in authorized list
  int checkResult = checkPhonenumberOnTheList(numberSms);
  
  // Drop key
  if ((checkResult == -1) && textSms.equals(openKeySmsCmd))
  {
    DEBUG_PRINT("Dropping key");
    openSesame = true;
  }
  // Clear EEPROM
  else if ((checkResult == -1) && textSms.equals(clearEEPROMCmd))
  {
    DEBUG_PRINT("Clearing EEPROM and authorized number list");
    clearEEPROM();
    EEPROM.get(0, authorizedNumbers); // update zeroes to list too
  }
  // Reset Arduino
  else if ((checkResult == -1) && textSms.equals(resetArduinoCmd))
  {
    DEBUG_PRINT("Resetting Arduino in 5...");
    delay(1000);
    DEBUG_PRINT("Resetting Arduino in 4...");
    delay(1000);
    DEBUG_PRINT("Resetting Arduino in 3...");
    delay(1000);
    DEBUG_PRINT("Resetting Arduino in 2...");
    delay(1000);
    DEBUG_PRINT("Resetting Arduino in 1...");
    delay(1000);
    resetFunc();
  }
  // Add number to authorized number list
  else if (textSms.equals(registerKeyCmd))
  {
    char number[14];
    numberSms.toCharArray(number, 14);
    authorizeNumberSms = addPhonenumberToList(number, checkResult);
  }   
  // Otherwise unauthorized sms
  else
  {
    DEBUG_PRINT("Unauthorized sms detected");
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
  if (!currentTime)
  {
    currentTime = millis();
  }
  if (currentTime)
  {
    switch (ledColor)
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

  if (millis() >= currentTime + 5000)
  {
    led.off();
    lcd.clear();
    currentTime = 0;
    flag = false;
  }
}

/**
  Saves phonenumber to EEPROM
  @param char phonenumber[14] Phonenumber to be saved in EEPROM
  @param int checkResult from checkPhonenumberOnTheList function
  @return true if number was saved to EEPROM or is already on the EEPROM.
*/
bool addPhonenumberToList(char phonenumber[14], int checkResult)
{
  if (checkResult == -1)
  {
    DEBUG_PRINT("Phonenumber " + (String)phonenumber + " was on the list. Not saved.");
    return true;
  }
  else if (checkResult == -2)
  {
    DEBUG_PRINT("Phonebook full. Number was not saved");
    return false;
  }
  else
  {
    strcpy(authorizedNumbers[result], phonenumber);
    EEPROM.put(0, authorizedNumbers);
    DEBUG_PRINT("Phonenumber was saved to index " + (String)result);
    return true;
  }
}

/**
  Check if phoneNumber is in the authorized number list
  @param String phonenumber to be checked
  @return -1 if number is on the list. -2 if list is full. Otherwise return next free index where to store number
*/
int checkPhonenumberOnTheList(String phonenumber)
{
  for (int i = 0; i < 10; i++)
  {
    String tempStr = authorizedNumbers[i];
    if (tempStr.equals(phonenumber))
    {
      return -1;
    }
    else if (!tempStr.substring(0, 4).equals("+358")) return i;
  }
  return -2;
}

/**
  Fills phonebook in the EEPROM with zeroes
*/
void clearEEPROM()
{
  char emptyArray[10][14];
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 14; j++)
    {
      emptyArray[i][j] = 0;
    }
  }
  EEPROM.put(0, emptyArray);
}
