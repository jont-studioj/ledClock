#ifndef UIMENU_TIMER_H
#define UIMENU_TIMER_H

#include "UiMenu.h"
#include "TimeSource.h"
#include "DigitDisplay.h"



class UiMenuTimer: public UiMenu {
public:
  UiMenuTimer(RefProvider &refProvider, const char *menuTitle, TimerData &timerData);

  void initMenuContent();
  UiEvent processItemEnter(int8_t itemIdx, bool bLongClick);
  void menuItemOptionChange(UiMenuItem *menuItem);

  UiEvent processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);
  UiEvent processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);

  void activateThyself(bool bRepaint);
  void deactivateThyself(bool bRepaint);
  
private:
  TimerData &timerData;
  uint8_t editingItemIdx;

  void startEditing(uint8_t itemIdx);
  void stopEditing(bool bAcceptValue);

};

#endif // UIMENU_TIMER_H
