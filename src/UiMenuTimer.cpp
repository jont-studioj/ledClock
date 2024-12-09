#include "UiMenuTimer.h"
#include "TimeSource.h"

enum ItemIdent {
  itemIdentBack,
  itemIdentStart,
  itemIdentPresetDuration,
  itemIdentBuzzerDuration
};

enum ItemIdx {
  idxBack = 0,
  idxStartTimer = 1,
  
  idxTimerDuration = 3,
  idxBuzzerDuration = 4
};


UiMenuTimer::UiMenuTimer(RefProvider &refProvider, const char *menuTitle, TimerData &timerData) : 
  UiMenu(refProvider, timerData.name),
  timerData(timerData),
  editingItemIdx(idxBack) {
}


void UiMenuTimer::initMenuContent() {
  addMenuItem(idxStartTimer, "start", false, 0, itemIdentStart, NULL);

  addMenuItem(idxTimerDuration, "duration", false, -1, itemIdentPresetDuration, NULL);
  addMenuItem(idxBuzzerDuration, "buzz time", false, -1, itemIdentBuzzerDuration, NULL);
}

UiEvent UiMenuTimer::processItemEnter(int8_t itemIdx, bool bLongClick) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  if ( itemIdx == idxStartTimer ) {
      
    // if this timer has a valid duration then act like we're going 
    // to use it immediately else do nothing until we have a good duration

    //Serial.printf("UiEvent UiMenuTimer::processItemEnter(%d, %d), timerIdx=%d, timerData.currentDurationSecs=%d\n", itemIdx, bLongClick, timerData.timerIdx, timerData.currentDurationSecs);
    if ( timerData.presetDurationSecs > 0 ) {
        refProvider.timeSource.pauseSelectedTimer(false);
        highLevelEvent = UiEvent::ueShutDownUI;
    }//endif timer has a usable duration
  }//endif start-timer item

  if ( highLevelEvent == UiEvent::ueNoEvent ) {
    highLevelEvent = UiMenu::processItemEnter(itemIdx, bLongClick);
  }

  return highLevelEvent;
}

void UiMenuTimer::menuItemOptionChange(UiMenuItem *menuItem) {

  uint8_t itemIdx = menuItem->itemIdx;
  if ( (itemIdx == idxTimerDuration) || (itemIdx == idxBuzzerDuration) ) {
    if ( menuItem->selected ) {
      startEditing(itemIdx);
    } else {
      stopEditing(true);
    }
  }
}

UiEvent UiMenuTimer::processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent uiEvent = UiEvent::ueNoEvent;
  if ( (editingItemIdx == idxTimerDuration) || (editingItemIdx == idxBuzzerDuration) ) {
    refProvider.timeSource.processUiEvent( bAltModeInput ? UiEvent::ueAltDecrease : UiEvent::ueDecrease );
  } else {
    uiEvent = UiMenu::processMenuDecrease(lowLevelEventSource, bAltModeInput);
  }
  return uiEvent;
}

UiEvent UiMenuTimer::processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent uiEvent = UiEvent::ueNoEvent;  
  if ( (editingItemIdx == idxTimerDuration) || (editingItemIdx == idxBuzzerDuration) ) {
    refProvider.timeSource.processUiEvent( bAltModeInput ? UiEvent::ueAltIncrease : UiEvent::ueIncrease );
  } else {
    uiEvent = UiMenu::processMenuIncrease(lowLevelEventSource, bAltModeInput);
  }
  return uiEvent;
}


void UiMenuTimer::activateThyself(bool bRepaint) {
  refProvider.timeSource.setSelectedTimer(timerData.timerIdx);
  UiMenu::activateThyself(bRepaint);
}


void UiMenuTimer::deactivateThyself(bool bRepaint) {
  //Serial.print("\tUiMenuTimer::deactivateThyself()");
  
  // in case we're exiting because of timeout, make sure we don't
  // get stuck in any timer editmode, discard any edited value
  stopEditing(false);

  // unless we started this timer (and are being unloaded because we're force-exiting the UI)
  // then deselect the timer (and thus restore the the old display mode)
  if ( !refProvider.timeSource.selectedTimerRunning() ) {
    refProvider.timeSource.setSelectedTimer(-1);
  }

  if ( didAnythingChange(true, false) ) {
    //Serial.println(" - something changed, saving");
    refProvider.timeSource.saveTimerData(timerData.timerIdx);
  } else {
    //Serial.println(" - nothing changed");
  }
  UiMenu::deactivateThyself(bRepaint);
}


void UiMenuTimer::startEditing(uint8_t itemIdx) {
  editingItemIdx = itemIdx;
  //Serial.printf("start editing: %d\n", editingItemIdx);
  refProvider.timeSource.startEditing( (itemIdx == idxTimerDuration) ? tsemTimerPresetDuration : tsemTimerBuzzerDuration );
}

void UiMenuTimer::stopEditing(bool bAcceptValue) {
  if ( editingItemIdx != idxBack ) {
    //Serial.printf("stop editing: %d, bAcceptValue=%d\n", editingItemIdx, bAcceptValue);
    uiMenuItems[editingItemIdx]->selected = false;
    refProvider.timeSource.stopEditing(bAcceptValue);
    editingItemIdx = idxBack;
    rememberSomethingChanged(true);
  }
}
