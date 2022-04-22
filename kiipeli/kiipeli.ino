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

// Data structure for key drops. Number and timestamp is saved
char keyDropEvents[10][32];

// LCD display
const uint8_t rs = 12, en = 13, d4 = 5, d5 = 6, d6 = 7, d7 = 8, contrast = 3;
const uint8_t contrastSetting = 125;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// LCD Menu
enum Menu 
{
  Wait_0,
  SavedNumbers_1,
  BrowseNumber_1_1,
  KeyDrops_2,
  BrowseKeyDrops_2_1,
  Testing_3,
  JoystickTest_3_1  
};

Menu menu = Wait_0;
uint8_t menuIndex = 0;

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
  DEBUG_PRINT("Authorized numbers list:");
  for (int i = 0; i < 10; i++)
  {
    DEBUG_PRINT(authorizedNumbers[i]);
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

void loop() {
  // Read joystick inputs
  joystickInputs = joystick.readInputs();

  // Debugging commands to sim from serial
#ifdef DEBUG
  if (Serial.available())
  {
    while (Serial.available())
    {
      sim.write(Serial.read());
    }
  }
#endif

  // LCD Menu
  switch(menu)
  {
    case Wait_0:
      lcd.clear();
      if(joystickInputs.buttonPressed) menu = SavedNumbers_1;
      break;
      
    case SavedNumbers_1:     
      lcd.clear();
      lcd.print("1. Saved numbers");
      if(joystickInputs.up) menu = Testing_3;
      if(joystickInputs.down) menu = KeyDrops_2;
      if(joystickInputs.right) menu = BrowseNumber_1_1;
      if(joystickInputs.left) menu = Wait_0;
      break;
      
    case BrowseNumber_1_1:
      lcd.clear();
      lcd.print("U/D browse L <-");
      lcd.setCursor(0,1);
      lcd.print(menuIndex + 1);
      lcd.print(":");
      lcd.print(authorizedNumbers[menuIndex]);
      if(joystickInputs.up)
      {
        if(menuIndex == 0) menuIndex = 9;
        else menuIndex--;
      };
      if(joystickInputs.down)
      {
        if(menuIndex == 9) menuIndex = 0;
        else menuIndex++;
      };
      if(joystickInputs.left) menu = SavedNumbers_1;
      break;
      
    case KeyDrops_2:
      lcd.clear();
      lcd.print("2. Keydrops");
      if(joystickInputs.up) menu = SavedNumbers_1;
      if(joystickInputs.down) menu = Testing_3;
      if(joystickInputs.left) menu = Wait_0;
      break;
      
    case BrowseKeyDrops_2_1:
      lcd.clear();
      lcd.print("");
      break;
      
    case Testing_3:
      lcd.clear();
      lcd.print("3. Testing");
      if(joystickInputs.up) menu = KeyDrops_2;
      if(joystickInputs.down) menu = SavedNumbers_1;
      if(joystickInputs.left) menu = Wait_0;
      break;
      
    case JoystickTest_3_1:
      lcd.clear();
      lcd.print("");
      break;
    default:
      break; 
  }

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

  if (textSms.equals(registerKeyCmd))
  {
    char number[14];
    numberSms.toCharArray(number, 14);
    authorizeNumberSms = addPhonenumberToList(number);
  }
  else if (isPhonenumberOnTheList(numberSms) && textSms.equals(openKeySmsCmd))
  {
    openSesame = true;
  }
  else if (isPhonenumberOnTheList(numberSms) && textSms.equals(clearEEPROMCmd))
  {
    clearEEPROM();
  }
  else if (!authorizeNumberSms)
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
        DEBUG_PRINT("Led green");
        break;
      case 2:
        led.flash(RGBLed::YELLOW, 100, 100);
        DEBUG_PRINT("Led yellow");
        break;
      case 3:
        led.flash(RGBLed::RED, 100, 100);
        DEBUG_PRINT("Led red");
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
  @return true if number was saved to EEPROM or is already on the EEPROM.
*/
bool addPhonenumberToList(char phonenumber[14])
{
  if (isPhonenumberOnTheList((String)phonenumber))
  {
    DEBUG_PRINT("Phonenumber " + (String)phonenumber + " was on the list. Not saved.");
    return true;
  }
  for (int i = 0; i < 10; i++)
  {
    String tempStr = authorizedNumbers[i];
    String countryCode = tempStr.substring(0, 4);
    DEBUG_PRINT("Countrycode: " + (String)countryCode);
    
    // To handle un-initialized EEPROM memory. If we find char array starting with 
    // Finnish country code it is most propably a valid number and not some gibberish
    if (!countryCode.equals("+358"))
    {
      DEBUG_PRINT("countryCode true when index " + String(i));
      strcpy(authorizedNumbers[i], phonenumber);
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
  for (int i = 0; i < 10; i++)
  {
    String tempStr = authorizedNumbers[i];
    if (tempStr.equals(phonenumber))
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
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 14; j++)
    {
      emptyArray[i][j] = 0;
    }
  }
  EEPROM.put(0, emptyArray);
}
