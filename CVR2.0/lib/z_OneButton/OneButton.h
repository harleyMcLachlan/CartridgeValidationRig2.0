// -----
// OneButton.h - Library for detecting button clicks, doubleclicks and long

//EDITED BY HARLEY SEPT 2020

// -----

#ifndef OneButton_h
#define OneButton_h

#include "Arduino.h"
#include "_task_cfg.h"

// ----- Callback function types -----

extern "C" {
typedef void (*callbackFunction)(void);
typedef void (*parameterizedCallbackFunction)(void*);
}

enum btn_state{
    BTN_TICKING_NOT_ACTIVE,
    BTN_TICKING_ACTIVE,
};


class OneButton
{
public:
  // ----- Constructor -----
  OneButton();

  OneButton(task_enum taskENUM, int pin = -1, int activeLow = true, bool pullupActive = false){
      // OneButton();
  _pin = pin;

  if (activeLow) {
    // the button connects the input pin to GND when pressed.
    _buttonPressed = LOW;

  } else {
    // the button connects the input pin to VCC when pressed.
    _buttonPressed = HIGH;
  } // if

  if (pullupActive) {
    // use the given pin as input and activate internal PULLUP resistor.
    pinMode(pin, INPUT_PULLUP);
  } else {
    // use the given pin as input
    pinMode(pin, INPUT);
  } // if

  task = taskENUM;


  ticking_state = BTN_TICKING_ACTIVE;
  }

  // ----- Set runtime parameters -----

  // set # millisec after safe click is assumed.
  void setDebounceTicks(int ticks);

  // set # millisec after single click is assumed.
  void setClickTicks(int ticks);

  // set # millisec after press is assumed.
  void setPressTicks(int ticks);

  // attach functions that will be called when button was pressed in the
  // specified way.
  void attachClick(callbackFunction newFunction);
  void attachClick(parameterizedCallbackFunction newFunction, void* parameter);
  void attachDoubleClick(callbackFunction newFunction);
  void attachDoubleClick(parameterizedCallbackFunction newFunction, void* parameter);
  void attachPress(
      callbackFunction newFunction); // DEPRECATED, replaced by longPressStart,
                                     // longPressStop and duringLongPress
  void attachLongPressStart(callbackFunction newFunction);
  void attachLongPressStart(parameterizedCallbackFunction newFunction, void* parameter);
  void attachLongPressStop(callbackFunction newFunction);
  void attachLongPressStop(parameterizedCallbackFunction newFunction, void* parameter);
  void attachDuringLongPress(callbackFunction newFunction);
  void attachDuringLongPress(parameterizedCallbackFunction newFunction, void* parameter);

  // ----- State machine functions -----

  /**
   * @brief Call this function every some milliseconds for checking the input
   * level at the initialized digital pin.
   */
  void tick(void);

  /**
   * @brief Call this function every time the input level has changed.
   * Using this function no digital input pin is checked because the current
   * level is given by the parameter.
   */
  void tick(bool level);

  bool isLongPressed();
  int getPressedTicks();
  void reset(void);

  void stop();
  void start();
  bool ticking_state;
  task_enum task;

private:
  int _pin; // hardware pin number.
  unsigned int _debounceTicks = 50; // number of ticks for debounce times.
  unsigned int _clickTicks = 600; // number of ticks that have to pass by
                                  // before a click is detected.
  unsigned int _pressTicks = 1000; // number of ticks that have to pass by
                                   // before a long button press is detected

  int _buttonPressed;

  bool _isLongPressed = false;

  // These variables will hold functions acting as event source.
  callbackFunction _clickFunc = NULL;
  parameterizedCallbackFunction _paramClickFunc = NULL;
  void* _clickFuncParam = NULL;

  callbackFunction _doubleClickFunc = NULL;
  parameterizedCallbackFunction _paramDoubleClickFunc = NULL;
  void* _doubleClickFuncParam = NULL;

  callbackFunction _pressFunc = NULL;

  callbackFunction _longPressStartFunc = NULL;
  parameterizedCallbackFunction _paramLongPressStartFunc = NULL;
  void* _longPressStartFuncParam = NULL;

  callbackFunction _longPressStopFunc = NULL;
  parameterizedCallbackFunction _paramLongPressStopFunc = NULL;
  void* _longPressStopFuncParam;

  callbackFunction _duringLongPressFunc = NULL;
  parameterizedCallbackFunction _paramDuringLongPressFunc = NULL;
  void* _duringLongPressFuncParam = NULL;

  // These variables that hold information across the upcoming tick calls.
  // They are initialized once on program start and are updated every time the
  // tick function is called.
  int _state = 0;
  unsigned long _startTime; // will be set in state 1
  unsigned long _stopTime; // will be set in state 2
};

#endif
