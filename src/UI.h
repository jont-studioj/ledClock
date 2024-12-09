#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include "RotEncoder.h"
#include "UiEnums.h"
#include "UiMenuTop.h"

struct RefProvider;

class UI {
public:
  UI(RefProvider &refProvider);

  void begin();
  bool buttonIsPressed();
  void enable();
  void disable();
  UiState getUiState();
  UiEvent loop();

  void setDefaultUiTimeoutPeriod();
  uint32_t getUiTimeoutPeriodMS();
  void setUiTimeoutPeriodMS(uint32_t newTimeoutPeriodMS);
  

  uint8_t getSignificantMutedEncoderSteps();
  void setSignificantMutedEncoderSteps(uint8_t newValue);

private:
//  Buttons buttons;
  RotEncoder *rotEncoder;

  RefProvider &refProvider;
  UiMenuTop topMenu;

  const uint32_t defaultUiTimeoutPeriodMS = 10000;
  uint32_t uiTimeoutPeriodMS;               // timeout and return to idle after this amount of inactivity
  uint32_t uiTimeoutTimeMS = 0;
  uint32_t uiButtonDownTimeMS = 0;
  const uint32_t longPressMS = 1500;

  bool bAltMode;
  bool bDoingAltIncreaseDecrease;
  UiEvent pendingState;
  
  UiState uiState;

  // hack to force UI shutdown
  bool bShutDownUI;

  void startUiActivity(UiLowLevelEventSource lowLevelEventSource);
  void stopUiActivity(UiLowLevelEventSource lowLevelEventSource);
  UiEvent processAltMode(bool bAltModeInput, UiEvent normalEvent, UiEvent altEvent);
//  UiEvent processIdleTimeout(UiLowLevelEventSource lowLevelEventSource);
  UiEvent shutDownUI(UiLowLevelEventSource lowLevelEventSource);
  UiEvent processMenuExit(UiLowLevelEventSource lowLevelEventSource);
  UiEvent processMenuEnter(UiLowLevelEventSource lowLevelEventSource, bool bLongClick);
  UiEvent processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);
  UiEvent processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);
  
  void clearIdleTimeout();
  void primeIdleTimeout();

  uint8_t significantMutedEncoderSteps = 3;
  int8_t mutedEncoderSteps;
  int8_t readMutedEncoder();
  void resetMutedEncoderSteps();

};

#endif // UI_H
