#include "UI.h"

#include "RefProvider.h"
#include "DisplayPanel.h"
#include "TimeSource.h"

UI::UI(RefProvider &refProvider) : 
  refProvider(refProvider),
  topMenu(refProvider, "OPTIONS") {
    refProvider.ui = this;
}

void UI::begin() {

  rotEncoder = RotEncoder::getInstance(
      ROT_ENCODER_A, INPUT, 
      ROT_ENCODER_B, INPUT, 
      ROT_ENCODER_BUTTON, INPUT, 
      ROT_ENCODER_VCC, ROT_ENCODER_STEPS
  );
  rotEncoder->disableAcceleration();
  resetMutedEncoderSteps();

  bAltMode = false;
  bDoingAltIncreaseDecrease = false;
  pendingState = UiEvent::ueNoEvent;

  bShutDownUI = false;
  disable();

  setDefaultUiTimeoutPeriod();
  
  topMenu.initialise(&itemEntryNULL);
  topMenu.setHierarchy(NULL);

}

bool UI::buttonIsPressed() {
  bool value = (rotEncoder->getButtonPressedState() == BUT_PRESSED_STATE_DOWN);
  return value;
}

void UI::enable() {
  if ( uiState == UiState::usIdleDisabled ) {  
    //Serial.println("UI::enable()");
    uiState = UiState::usIdleEnabled;
  }
}
void UI::disable() {
  if ( uiState == UiState::usIdleEnabled ) {
    //Serial.println("UI::disable()");
    uiState = UiState::usIdleDisabled;
  }
}


UiState UI::getUiState() { 
  return uiState;
}

const char *getLLE(UiLowLevelEvent lle) {
  switch (lle) {
  case UiLowLevelEvent::ulleNone:
    return "ulleNone";
  case UiLowLevelEvent::ulleExit:
    return "ulleExit";
  case UiLowLevelEvent::ulleEnter:
    return "ulleEnter";
  case UiLowLevelEvent::ulleLeft:
    return "ulleLeft";
  case UiLowLevelEvent::ulleRight:
    return "ulleRight";
  }
  return "**unknown**";
}

UiEvent UI::processAltMode(bool bAltModeInput, UiEvent normalEvent, UiEvent altEvent) {
  UiEvent resultEvent;
  if ( bAltModeInput != bAltMode ) {
    // when entering or exiting altMode, we discard(and sadly lose) the current event
    if ( bAltMode ) {
      //Serial.println("Exiting alt mode");
      pendingState = normalEvent;
      resultEvent = UiEvent::ueAltModeEnd;
      // 2024-08-22 I don't know why I needed this
      // but have nopped it out as it prevents the automatic
      // return to normal display if you've cycled the display
      // (maybe I will later learn why I orginally put this in)
      //clearIdleTimeout();
    } else {
      //Serial.println("Entering alt mode");
      pendingState = altEvent;
      resultEvent = UiEvent::ueAltModeBegin;
      // 2024-08-22 I don't know why I needed this
      // but have nopped it out as it prevents the automatic
      // return to normal display if you've cycled the display
      // (maybe I will later learn why I orginally put this in)
      //clearIdleTimeout();
    }
    bAltMode = bAltModeInput;
  } else {
    resultEvent = bAltMode ? altEvent : normalEvent;
  }
  return resultEvent;
}

UiEvent UI::loop() {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  UiLowLevelEvent lowLevelEvent = UiLowLevelEvent::ulleNone;
  UiLowLevelEventSource lowLevelEventSource = UiLowLevelEventSource::ullesNone;

  uint32_t nowMS = millis();


  // check for idle timeout before checking any user input
  // we should only be able to timeout if we were actually doing something
  if ( bShutDownUI || ((uiTimeoutTimeMS > 0) && (nowMS > uiTimeoutTimeMS)) ) {
    highLevelEvent = shutDownUI(UiLowLevelEventSource::ullesIdleTimeout);

  } else {
    // no timeout, go for user input from h/w of some sort, then process it

    // unless we have a state pending (from entering altMode), 
    // in which case return that instead of looking for more input
    if ( pendingState != UiEvent::ueNoEvent ) {
      highLevelEvent = pendingState;
      pendingState = UiEvent::ueNoEvent;
    } else {

      // flag to indicate whether mode-button or encoder-switch is currently pressed
      bool bAltModeInput = false;

      bool bLongClick = false;


      // ****************************************************************
      // rotary encoder input
      //
      // ...

      //todo: this was quickly hacked in, maybe not right thing as there is no debounce
      bAltModeInput = buttonIsPressed();


      // we only look for other input if the display are powered up
      if ( refProvider.displayPanel.isPoweredUp() ) {
        ButtonEvent buttonEvent = rotEncoder->getButtonEvent();

        // if button just pressed, remember the time it happened
        // (so we can detect long-press-click)
        if ( buttonEvent == BUT_EVENT_PRESSED ) {
          uiButtonDownTimeMS = nowMS;
        } else if ( buttonEvent == BUT_EVENT_RELEASED ) {
          //if we were doing alt-right/left then eat this 
          if ( bDoingAltIncreaseDecrease ) {
            // eat the button release
            lowLevelEvent = UiLowLevelEvent::ulleNone;
            bDoingAltIncreaseDecrease = false;
          } else {
            //only treat this as a click if we're considering the button was down
            //(we stop considering the button as being down if we've interpreted the press as a press-hold)
            if (uiButtonDownTimeMS != 0) {
              lowLevelEvent = UiLowLevelEvent::ulleEnter;
            }
          }
          // remember that we no longer have button down
          uiButtonDownTimeMS = 0;
        } else {
          // no button down/up events
          // maybe we're increasing/decreasing, or failing that, maybe look for a press-hold
          int8_t encoderStep = readMutedEncoder();
          if ( encoderStep < 0 ) {
            lowLevelEvent = UiLowLevelEvent::ulleLeft;
            bDoingAltIncreaseDecrease = bAltModeInput;
          } else if ( encoderStep > 0 ) {
            lowLevelEvent = UiLowLevelEvent::ulleRight;
            bDoingAltIncreaseDecrease = bAltModeInput;
          }
          // if we've not initiated an alt increase/decrease at all..
          if ( !bDoingAltIncreaseDecrease ) {
            // if we're in button down mode and we're down long enough
            // then treat as a button-hold (aka long press) and trigger
            // on it immediately (as a long click) without waiting for the button release
            if ( (uiButtonDownTimeMS != 0) && ((nowMS - uiButtonDownTimeMS) > longPressMS) ) {
              lowLevelEvent = UiLowLevelEvent::ulleEnter;
              bLongClick = true;
              // stop considering the button as pressed
              uiButtonDownTimeMS = 0;
            }
          }
        }
      }

      if ( lowLevelEvent != UiLowLevelEvent::ulleNone ) {
        lowLevelEventSource = UiLowLevelEventSource::ullesButton;
      }

      // ****************************************************************

      // only bother doing anything if we had some kind of event
      if ( lowLevelEvent == UiLowLevelEvent::ulleNone ) {
        // detect exiting altMode while no other low level event is happening
        if ( bAltModeInput == false ) {
          highLevelEvent = processAltMode(bAltModeInput, UiEvent::ueNoEvent, UiEvent::ueNoEvent);
        }

      } else {
        //Serial.print("lowLevelEvent=");
        //Serial.println( getLLE(lowLevelEvent) );

        primeIdleTimeout();

        if ( uiState == UiState::usActive ) {
          // we are active
          switch ( lowLevelEvent ) {
          case UiLowLevelEvent::ulleExit:
            // ueExit current menu or stop ui if we're at top menu
            highLevelEvent = processMenuExit(lowLevelEventSource);
            break;
          case UiLowLevelEvent::ulleEnter:
            highLevelEvent = processMenuEnter(lowLevelEventSource, bLongClick);
            break;
          case UiLowLevelEvent::ulleLeft:
            highLevelEvent = processMenuDecrease(lowLevelEventSource, bAltModeInput);
            break;
          case UiLowLevelEvent::ulleRight:
            highLevelEvent = processMenuIncrease(lowLevelEventSource, bAltModeInput);
            break;
          }
        } else {
          // we are not active
          switch ( lowLevelEvent ) {
//TODO: this looks like leftover from ips-clock and not relevant in this project            
          case UiLowLevelEvent::ulleExit:
            // special case... while inactive, if power-button rising and mode-button is held down, we reboot
            if ( bAltModeInput ) {
              esp_restart();
            } else {
              // if we're disabled then this is likely an alternate use of the h/w input and this represents some kind of cancel event
              // otherwise (we're enabled) then this is interpreted as the powerToggle event
              highLevelEvent = ( uiState == UiState::usIdleDisabled ) ? UiEvent::ueCancel : UiEvent::uePowerToggle;
              clearIdleTimeout();
            }
            break;
          case UiLowLevelEvent::ulleEnter:
            // this one potentially makes us active (if we're enabled)
            if ( uiState == UiState::usIdleDisabled ) {
              highLevelEvent = bLongClick ? UiEvent::ueAltAccept : UiEvent::ueAccept;
              clearIdleTimeout();
            } else {
              highLevelEvent = processMenuEnter(lowLevelEventSource, bLongClick);
            }
            break;
          case UiLowLevelEvent::ulleLeft:
            highLevelEvent = processAltMode(bAltModeInput, UiEvent::ueDecrease, UiEvent::ueAltDecrease);
            break;
          case UiLowLevelEvent::ulleRight:
            highLevelEvent = processAltMode(bAltModeInput, UiEvent::ueIncrease, UiEvent::ueAltIncrease);
            break;
          }
        }//endif ui is active/inactive
      }//endif some event happened
    }//endif we did/didn't have a state pending
  }//endif had timeout or not
  return highLevelEvent; 
}


void UI::startUiActivity(UiLowLevelEventSource lowLevelEventSource) {
  resetMutedEncoderSteps();
  uiState = UiState::usActive;
}

void UI::stopUiActivity(UiLowLevelEventSource lowLevelEventSource) {
  clearIdleTimeout();
  uiState = UiState::usIdleEnabled;
}


//UiEvent UI::processIdleTimeout(UiLowLevelEventSource lowLevelEventSource) {
UiEvent UI::shutDownUI(UiLowLevelEventSource lowLevelEventSource) {  
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

Serial.printf("shutDownUI() uiState=%d\n", uiState);

  // if we're active then close all menus and stop ui activity
  // else this was a timeout for some alternate h/w input use, treat as cancel
  if ( uiState == UiState::usActive ) {
    //Serial.println("shutDownUI() uiState == UiState::usActive");
    // we are active, close menus until we've closed the top one
    //Serial.println("while active");
    uint8_t maxMenuDepth = 10;
    for (uint8_t idxMenu = 0; idxMenu < maxMenuDepth ; idxMenu++ ) {
      //Serial.println("shutDownUI() processMenuExit()");
      highLevelEvent = processMenuExit(lowLevelEventSource);
      if ( highLevelEvent == UiEvent::ueUiStopped ) {
        break;
      }
    }
  } else {
    //Serial.println("while inactive");
    // assume we had some timeout while the h/w input was being used for something else
    // disable timeout
    clearIdleTimeout();
    highLevelEvent = UiEvent::ueCancel;
  }

  bShutDownUI = false;

  bDoingAltIncreaseDecrease = false;
  return highLevelEvent;
}

UiEvent UI::processMenuExit(UiLowLevelEventSource lowLevelEventSource) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  if ( uiState == UiState::usActive ) {
    // pass to my top menu, it'll possibly rattle down from there
    highLevelEvent = topMenu.processMenuExit(lowLevelEventSource);

    // if we're exiting the top menu then we're done doing UI
    if ( highLevelEvent == UiEvent::ueMenuDeactivating ) {
      stopUiActivity(lowLevelEventSource);
      highLevelEvent = UiEvent::ueUiStopped;
    } else if ( highLevelEvent == UiEvent::ueShutDownUI ) {
      bShutDownUI = true;
      highLevelEvent = UiEvent::ueNoEvent;
    }
  }

  return highLevelEvent;
}

UiEvent UI::processMenuEnter(UiLowLevelEventSource lowLevelEventSource, bool bLongClick) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  if ( uiState != UiState::usIdleDisabled ) {
    // pass to my top menu, it'll possibly rattle down from there
    highLevelEvent = topMenu.processMenuEnter(lowLevelEventSource, bLongClick);

    // if we're just activating the top menu then we're starting doing UI
    if ( highLevelEvent == UiEvent::ueMenuActivating ) {
      startUiActivity(lowLevelEventSource);
      highLevelEvent = UiEvent::ueUiStarted;
    } else if ( highLevelEvent == UiEvent::ueMenuDeactivating ) {
      stopUiActivity(lowLevelEventSource);
      highLevelEvent = UiEvent::ueUiStopped;
    } else if ( highLevelEvent == UiEvent::ueShutDownUI ) {
      bShutDownUI = true;
      highLevelEvent = UiEvent::ueNoEvent;
    }
  }

  return highLevelEvent;
}

UiEvent UI::processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  // pass to my top menu, it'll possibly rattle down from there
  UiEvent highLevelEvent = topMenu.processMenuDecrease(lowLevelEventSource, bAltModeInput);

  return highLevelEvent;
}

UiEvent UI::processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  // pass to my top menu, it'll possibly rattle down from there
  UiEvent highLevelEvent = topMenu.processMenuIncrease(lowLevelEventSource, bAltModeInput);

  return highLevelEvent;
}


void UI::setDefaultUiTimeoutPeriod() {
  setUiTimeoutPeriodMS(defaultUiTimeoutPeriodMS);
}

uint32_t UI::getUiTimeoutPeriodMS() {
  return uiTimeoutPeriodMS;
}
void UI::setUiTimeoutPeriodMS(uint32_t newTimeoutPeriodMS) {
  uiTimeoutPeriodMS = newTimeoutPeriodMS;
  primeIdleTimeout();
}


void UI::clearIdleTimeout() {
  //Serial.println("clearIdleTimeout()");
  uiTimeoutTimeMS = 0; 
}
void UI::primeIdleTimeout() {
  if ( uiTimeoutPeriodMS > 0 ) {
    uiTimeoutTimeMS = millis() + uiTimeoutPeriodMS;
  } else {
    clearIdleTimeout();
  }
}

int8_t UI::readMutedEncoder() {
  int8_t result = 0;
  if ( rotEncoder != NULL ) {
    long encoderValue = rotEncoder->readEncoder();

    if (encoderValue < 0) {
      // if we're just changing direction then ignore this one
      if ( mutedEncoderSteps > 0 ) {
        resetMutedEncoderSteps();
      } else {
        mutedEncoderSteps -= 1;
      }
    } else if (encoderValue > 0) {
      // if we're just changing direction then ignore this one
      if ( mutedEncoderSteps < 0 ) {
        resetMutedEncoderSteps();
      } else {
        mutedEncoderSteps += 1;
      }
    }
    rotEncoder->reset();
    //Serial.printf("mutedEncoderSteps:\%d\n", mutedEncoderSteps);
    // do more muting
    if ( abs(mutedEncoderSteps) > significantMutedEncoderSteps ) {
      result = mutedEncoderSteps / significantMutedEncoderSteps;
      resetMutedEncoderSteps();
    }
  }//endif we have the rotEncoder object

  // 2024-09-08: invert the result after having switched the
  // pin assignments around (enc-A<-->enc-B) in order to match
  // the rotary encoder's documentation which says:
  // Encoder-A = CLK          (was 37, now 38)
  // Encoder-B = DT           (was 38, now 37)
  return -result;
}

uint8_t UI::getSignificantMutedEncoderSteps() {
  return significantMutedEncoderSteps;
}
void UI::setSignificantMutedEncoderSteps(uint8_t newValue) {
  significantMutedEncoderSteps = newValue;
}
void UI::resetMutedEncoderSteps() {
  mutedEncoderSteps = 0;
  if ( rotEncoder != NULL ) {
    rotEncoder->reset();
  }
}