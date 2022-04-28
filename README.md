# Kiipeli

Kiipeli is a Arduino controlled spare key holder device for OAMK software course project

## Description

Kiipeli holds spare key to your house, summer cottage or similar place. When spare key is needed, user sends a predefined sms code from authorized phone number to Kiipeli. Sms is then checked and controller releases spare key to sms sender. Device is located inside the house to prevent spare key being stolen.
A proper key delivery path (frame has 32 mm fit for round pipe) must be installed between Kiipeli and outside of the house.

![3D Model of Kiiepli](/images/kiipeli.png)

## Components of Kiipeli

* Arduino Uno - microcontroller
* Sim800L V2 - Sms receiver
* LCD 16x2 display - displays menu and info messages
* RGB Led - indicates status of the device
* Mini-joystick (analog x and y axis and a pushbutton) - for controlling display menu
* Opto-relay - for solenoid control
* Solenoid - for dropping the key

## Wiring diagram
![Kiipeli_wiring_diagram](https://user-images.githubusercontent.com/49938344/164885777-150c7f17-8b95-4020-8190-c3e1361135d6.jpg)

## Authors

OAMK TVT22KMO Group 7
* [Anssi Kulotie](https://www.linkedin.com/in/anssi-kulotie-659a6021a/)
* [Esa Salminen](https://www.linkedin.com/in/esa-salminen-9398421ba/)
* Janika Savela
* [Miikka Tyvelä](https://www.linkedin.com/in/miikka-tyvel%C3%A4-090366225/)
* Hannu Väliahde
