#include "UiMenuTop.h"

#include "DigitDisplay.h"
#include "UiMenuSetup.h"
#include "TimeSource.h"
#include "UiMenuTimer.h"

enum ItemIdx {
  idxTimer1 = 1,
  idxTimer2 = 2,
  idxTimer3 = 3,
  idxTimer4 = 4,
  idxTimer5 = 5,
  idxSetup = 6
};

void UiMenuTop::initMenuContent() {
  TimeSource &timeSource = refProvider.timeSource;
  addMenuItem(idxTimer1, NULL, false, 0, 0, new UiMenuTimer(refProvider, "", timeSource.getTimerData(0)) );
  addMenuItem(idxTimer2, NULL, false, 0, 0, new UiMenuTimer(refProvider, "", timeSource.getTimerData(1)) );
  addMenuItem(idxTimer3, NULL, false, 0, 0, new UiMenuTimer(refProvider, "", timeSource.getTimerData(2)) );
  addMenuItem(idxTimer4, NULL, false, 0, 0, new UiMenuTimer(refProvider, "", timeSource.getTimerData(3)) );
  addMenuItem(idxTimer5, NULL, false, 0, 0, new UiMenuTimer(refProvider, "", timeSource.getTimerData(4)) );

  addMenuItem(idxSetup, "configure", false, 0, 0, new UiMenuSetup(refProvider, "CONFIGURE"));
}

void UiMenuTop::paintMenuContent(bool bRepaint, bool bAsActive) {
  TimeSource &timeSource = refProvider.timeSource;
  // change/set timer captions to their configured preset values, before repainting
  for (uint8_t timerIdx = 0; timerIdx < timeSource.qtyTimers; timerIdx++) {
    TimerData &timerData = timeSource.getTimerData(timerIdx);
    uint8_t itemIdx = idxTimer1 + timerIdx;
    uiMenuItems[itemIdx]->caption = timeSource.getTimerCaption(timerIdx, ccDefault);
  }

  UiMenu::paintMenuContent(bRepaint, bAsActive);
}


UiEvent UiMenuTop::processItemEnter(int8_t itemIdx, bool bLongClick) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;


  if ( (itemIdx >= idxTimer1) && (itemIdx <= idxTimer5) ) {
    uint8_t timerIdx = itemIdx - idxTimer1;

    // we kinda invert the meaning of long click for the timer items...
    //  a short click starts the timer immediate (if valid)
    //  a long click is treated as a normal click and enters 
    //    that timer's setup sub-menu
    if ( bLongClick ) {
      bLongClick == false;
    } else {
      // short click for timer item, try to start timer immediately
      // rather than treat as child-menu entry...
      // (unless no valid time, i which case go into its setup sub-menu)
      // if this timer has a valid duration then act like we're going 
      // to use it immediately else drop into the child menu to set it up
      TimerData &timerData = refProvider.timeSource.getTimerData(timerIdx);
      //Serial.printf("UiMenuTop::processItemEnter(%d, %d), timerIdx=%d, timerData.currentDurationSecs=%d\n", itemIdx, bLongClick, timerIdx, timerData.currentDurationSecs);
      if ( timerData.presetDurationSecs > 0 ) {
          refProvider.timeSource.setSelectedTimer(timerIdx);
          refProvider.timeSource.startEditing(tsemTimerCurrentDuration);
          highLevelEvent = UiEvent::ueShutDownUI;
      }//endif timer has a usable duration
    }//endif/else was long click
  }//endif a timer item

  if ( highLevelEvent == UiEvent::ueNoEvent ) {
    highLevelEvent = UiMenu::processItemEnter(itemIdx, bLongClick);
  }

  return highLevelEvent;
}
