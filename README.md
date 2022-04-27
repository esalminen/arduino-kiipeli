# Kiipeli

Kiipeli is a Arduino controlled spare key holder device for OAMK software course project

## Description

Kiipeli holds spare key e.g. to your house, summer cottage or similar place. When spare key is needed, user sends a predefined sms code from authorized phone number to Kiipeli. Sms is then checked and controller releases spare key to sms sender. Device is located inside the house to prevent spare key being stolen.
A proper delivery path must be installed between Kiipeli and outside of the house.

## Components of Kiipeli

* Arduino Uno
* Sim800L V2
* LCD 16x2 display
* RGB Led
* Mini-joystick (analog x and y axis and a pushbutton)
* Opto-relay
* Solenoid

## Wiring diagram
![Kiipeli_wiring_diagram](https://user-images.githubusercontent.com/49938344/164885777-150c7f17-8b95-4020-8190-c3e1361135d6.jpg)

## Authors

OAMK TVT22KMO Group 7

## Version History

* 0.1
    * Initial Release

## Acknowledgments

Inspiration, code snippets, etc.
* [RGBLed library for Arduino](https://github.com/wilmouths/RGBLed)
