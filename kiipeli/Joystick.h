/**
  Name: Joystick
  Purpose: Converts joystick analog sensor values to boolean on/off values

  @author Esa Salminen
  @version 1.0 17.4.2022
*/

#include "Arduino.h"

struct JoystickInputs {
  bool upPulse = false, downPulse = false, leftPulse = false, rightPulse = false, buttonPressedPulse = false;
  bool up = false, down = false, left = false, right = false, buttonPressed = false;
};

class Joystick {
    byte inputs = 0, prevInputs = 0, pulseInputs = 0;
    int _xAxisPin = 0;
    int _yAxisPin = 0;
    int _buttonPin = 0;
    int _axisLowLimit = 0;
    int _axisHighLimit = 0;
    JoystickInputs _joystickInputs;
    JoystickInputs _returnInputs;


  public:
    /**
        Joystick constructor.
      @param xAxisPin analog pin of x-axis
      @param yAxisPin analog pin of y-axis
      @param buttonPin digital pin of pushbutton
      @param axisLowLimit value threshold from middlepoint to turn input off
      @param axisHighLimit value threshold from middlepoint to turn input on
    */
    Joystick(int xAxisPin, int yAxisPin, int buttonPin, int axisLowLimit, int axisHighLimit);

    /**
      Joystick initialization in setup.
    */
    void begin();

    /**
      Cyclic call in main loop to read inputs
      @returns JoysticInputs struct with inputs red
    */
    JoystickInputs readInputs();
};
