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
char keyDropEvents[10][30];

// LCD display
const uint8_t rs = 12, en = 13, d4 = 5, d5 = 6, d6 = 7, d7 = 8, contrast = 3;
uint8_t contrastSetting = 110;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// LCD Menu
enum MenusStates 
{
  Wait_0,
  SavedNumbers_1,
  BrowseNumber_1_1,
  KeyDrops_2,
  BrowseKeyDrops_2_1,
  Testing_3,
  JoystickTest_3_1  
};

MenusStates menu = Wait_0;
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
String dateSms = "";

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

  // Read keydrop events from EEPROM
  EEPROM.get(140, keyDropEvents);

  // Activate serial transmitting in debug mode
#ifdef DEBUG
  Serial.begin(9600);
  DEBUG_PRINT("Authorized numbers list:");
  for (int i = 0; i < 10; i++)
  {
    DEBUG_PRINT(authorizedNumbers[i]);
  }

  DEBUG_PRINT("Keydrop events list:");
  for (int i = 0; i < 10; i++)
  {
    DEBUG_PRINT(keyDropEvents[i]);
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
 * Arduino software reset function
 */
void(* resetFunc) (void) = 0;

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
      if(joystickInputs.buttonPressed) menu = SavedNumbers_1;
      break;
      
    case SavedNumbers_1:     
      lcd.clear();
      lcd.print("1. Saved numbers");
      if(joystickInputs.up) menu = Testing_3;
      if(joystickInputs.down) menu = KeyDrops_2;
      if(joystickInputs.left)
      {
        lcd.clear();
        menu = Wait_0;  
      }
      if(joystickInputs.right) 
      {
        menuIndex = 0; 
        menu = BrowseNumber_1_1; 
      }
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
      if(joystickInputs.left)
      {
        lcd.clear();
        menu = Wait_0;  
      }
      if(joystickInputs.right)
      {
        menuIndex = 0;
        menu = BrowseKeyDrops_2_1;
      }
      break;
      
    case BrowseKeyDrops_2_1:
      lcd.clear();
      lcd.print(menuIndex + 1);
//      lcd.print(":");
//      String tempEvents = keyDropEvents[menuIndex];
//      lcd.print(tempEvents.substring(0,13));
//      lcd.setCursor(0,1);
//      lcd.print(" ");
//      lcd.print(tempEvents.substring(13));
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
      if(joystickInputs.left) menu = KeyDrops_2;
      break;
      
    case Testing_3:
      lcd.clear();
      lcd.print("3. Testing");
      if(joystickInputs.up) menu = KeyDrops_2;
      if(joystickInputs.down) menu = SavedNumbers_1;
      if(joystickInputs.left)
      {
        lcd.clear();
        menu = Wait_0;  
      }
      if(joystickInputs.right) menu = JoystickTest_3_1;
      break;
      
    case JoystickTest_3_1:
      lcd.clear();
      lcd.print("U/D:lcd, R:open");
      lcd.setCursor(0,1);
      lcd.print("lcd:" + String(contrastSetting) + " B:led");
      if(joystickInputs.up)
      {
        contrastSetting++;
        analogWrite(contrast, contrastSetting);
      }
      if(joystickInputs.down)
      {
        contrastSetting--;
        analogWrite(contrast, contrastSetting);
      }
      if(joystickInputs.left) menu = Testing_3;
      if(joystickInputs.buttonPressed)
      {
        led.setColor(RGBLed::RED);
        delay(1000);
        led.setColor(RGBLed::GREEN);
        delay(1000);
        led.setColor(RGBLed::BLUE);
        delay(1000);
        led.setColor(RGBLed::WHITE);
        delay(1000);
        led.off();  
      };
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

  // Remove beginning before sender phonenumber
  tempStr.remove(0, indexOfSeparator + 1);

  // Find the end of sender phonenumber
  indexOfSeparator = tempStr.indexOf('"');

  // Save sender phonenumber to variable
  numberSms = tempStr.substring(0, indexOfSeparator);
  DEBUG_PRINT("Sender: "); Serial.println(numberSms);

  // Find beginning of timestamp from "/" and subtract 2 to get beginning of the timestamp
  indexOfSeparator = tempStr.indexOf('/');
  indexOfSeparator -= 2;
  tempStr.remove(0, indexOfSeparator);
  dateSms = tempStr.substring(0, 14); // Date value is 14 chars long excluding seconds
  DEBUG_PRINT("Date: "); Serial.println(dateSms);

  // Find beginning of sms from last quote
  indexOfSeparator = tempStr.lastIndexOf('"');

  // Remove string until the last quote + cr + lf
  tempStr.remove(0, indexOfSeparator + 3);

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
    String temp = numberSms + ":" + dateSms;
    char tempCharArray[30];
    temp.toCharArray(tempCharArray, 30);
    DEBUG_PRINT("Adding event from keydrop: " + temp);
    addKeydropEventToList(tempCharArray);   
  }
  else if (isPhonenumberOnTheList(numberSms) && textSms.equals(clearEEPROMCmd))
  {
    // Clear EEPROM
    DEBUG_PRINT("Starting to clear EEPROM.");
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
  Saves keydrop event to EEPROM
  @param char eventInfo[28] Event to be saved in EEPROM
  @return true if event was saved to EEPROM
*/
bool addKeydropEventToList(char eventInfo[28])
{
  for (int i = 0; i < 10; i++)
  {    
    String tempStr = keyDropEvents[i];
    String countryCode = tempStr.substring(0, 4);
    // To handle un-initialized EEPROM memory. If we find char array starting with 
    // Finnish country code it is most propably a valid number and not some gibberish
    if (!countryCode.equals("+358")) 
    {
      DEBUG_PRINT("Event saved to index " + String(i));
      strcpy(keyDropEvents[i], eventInfo);
      EEPROM.put(140, keyDropEvents);
      return true;
    }
  }
  DEBUG_PRINT("Eventlist full. Event was not saved");
  return false;
}

/**
  Fills EEPROM area with zeros
*/
void clearEEPROM()
{
  for(int i = 0; i < EEPROM.length();i++)
  {
    EEPROM.write(i,0);  
  }
  DEBUG_PRINT("EEPROM filled with zeroes, resetting Arduino");
  resetFunc();
}
