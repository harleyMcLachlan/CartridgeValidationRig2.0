/*
Switch Modified by Harley Sept 2020
*/

#ifndef SWITCH_H
#define SWITCH_H
#include <Arduino.h>
#include "_task_cfg.h"

typedef void (*switchCallback_t)(void*);

enum switch_state{
    SW_TICKING_NOT_ACTIVE,
    SW_TICKING_ACTIVE,
};


class Switch
{
public:
  Switch(task_enum taskENUM, byte _pin, byte PinMode=INPUT_PULLUP, bool polarity=LOW, int debounceDelay=100, int longPressDelay=400, int doubleClickDelay=250);
  bool poll(); // Returns 1 if switched
  bool switched(); // will be refreshed by poll()
  bool on();
  bool pushed(); // will be refreshed by poll()
  bool released(); // will be refreshed by poll()
  bool longPress(); // will be refreshed by poll()
  bool longPressLatch();
  bool doubleClick(); // will be refreshed by poll()
  bool ticking_state;
  task_enum task;
  void stop();
  void start();

  unsigned long _switchedTime, pushedTime;

  unsigned long pushedDuration();  // returns duration of press in milliseconds, 0 if not pressed

  // Set methods for event callbacks
  void setPushedCallback(switchCallback_t cb, void* param = nullptr);
  void setReleasedCallback(switchCallback_t cb, void* param = nullptr);
  void setLongPressCallback(switchCallback_t cb, void* param = nullptr);
  void setDoubleClickCallback(switchCallback_t cb, void* param = nullptr);

protected:
  const byte pin;
  const unsigned long debounceDelay, longPressDelay, doubleClickDelay;
  const bool polarity;
  bool level, _switched, _longPress, _longPressLatch, _doubleClick;

  // Event callbacks
  switchCallback_t _pushedCallback = nullptr;
  switchCallback_t _releasedCallback = nullptr;
  switchCallback_t _longPressCallback = nullptr;
  switchCallback_t _doubleClickCallback = nullptr;

  void* _pushedCallbackParam = nullptr;
  void* _releasedCallbackParam = nullptr;
  void* _longPressCallbackParam = nullptr;
  void* _doubleClickCallbackParam = nullptr;

};

#endif
