/**
  Name: Joystick
  Purpose: Converts joystick analog sensor values to boolean on/off values

  @author Esa Salminen
  @version 1.0 17.4.2022
*/

#include "Arduino.h"
#include "Joystick.h"

Joystick::Joystick(int xAxisPin, int yAxisPin, int buttonPin, int axisLowLimit, int axisHighLimit)
{
  _xAxisPin = xAxisPin;
  _yAxisPin = yAxisPin;
  _buttonPin = buttonPin;
  _axisLowLimit = axisLowLimit;
  _axisHighLimit = axisHighLimit;
}

void Joystick::begin()
{
  // Define digital pin for button
  pinMode(_buttonPin, INPUT_PULLUP);
}

JoystickInputs Joystick::readInputs()
{
  // Read button state
  _joystickInputs.buttonPressed = analogRead(_buttonPin) <= _axisLowLimit;

  // Map joystick analog signals
  int leftRight = map(analogRead(_xAxisPin), 0, 1023, -512, 512);
  int upDown = map(analogRead(_yAxisPin), 0, 1023, -512, 512);

  // Below zero is left
  if (leftRight < 0)
  {
    _joystickInputs.right = false;
    if (abs(leftRight) >= _axisHighLimit && !_joystickInputs.left)
    {
      _joystickInputs.left = true;
    }
    if (abs(leftRight) <= _axisLowLimit && _joystickInputs.left)
    {
      _joystickInputs.left = false;
    }
  }
  // Right
  else
  {
    _joystickInputs.left = false;
    if (abs(leftRight) >= _axisHighLimit && !_joystickInputs.right)
    {
      _joystickInputs.right = true;
    }
    if (abs(leftRight) <= _axisLowLimit && _joystickInputs.right)
    {
      _joystickInputs.right = false;
    }
  }

  // Above zero is down
  if (upDown > 0)
  {
    _joystickInputs.up = false;
    if (abs(upDown) >= _axisHighLimit && !_joystickInputs.down)
    {
      _joystickInputs.down = true;
    }
    if (abs(upDown) <= _axisLowLimit && _joystickInputs.down)
    {
      _joystickInputs.down = false;
    }
  }
  // Up
  else
  {
    _joystickInputs.down = false;
    if (abs(upDown) >= _axisHighLimit && !_joystickInputs.up)
    {
      _joystickInputs.up = true;
    }
    if (abs(upDown) <= _axisLowLimit && _joystickInputs.up)
    {
      _joystickInputs.up = false;
    }
  }

  return _joystickInputs;
}
