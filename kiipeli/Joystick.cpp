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
    // Copy inputs to byte bit by bit
  bitWrite(inputs, 0, _joystickInputs.up);
  bitWrite(inputs, 1, _joystickInputs.down);
  bitWrite(inputs, 2, _joystickInputs.left);
  bitWrite(inputs, 3, _joystickInputs.right);
  bitWrite(inputs, 4, _joystickInputs.buttonPressed);  

  // Make AND operation with prev complement to detect positive change in inputs for 1 cycle
  pulseInputs = inputs & ~prevInputs;

  // Copy input bits to return inputs struct
  _returnInputs.up = _joystickInputs.up;
  _returnInputs.down = _joystickInputs.down;
  _returnInputs.left = _joystickInputs.left;
  _returnInputs.right = _joystickInputs.right;
  _returnInputs.buttonPressed = _joystickInputs.buttonPressed;

  // Copy pulse input bits to return inputs struct
  _returnInputs.upPulse = bitRead(pulseInputs, 0);
  _returnInputs.downPulse = bitRead(pulseInputs, 1);
  _returnInputs.leftPulse = bitRead(pulseInputs, 2);
  _returnInputs.rightPulse = bitRead(pulseInputs, 3);
  _returnInputs.buttonPressedPulse = bitRead(pulseInputs, 4);

  // Copy joystick inputs to memory
  prevInputs = inputs;
  return _returnInputs;
}
