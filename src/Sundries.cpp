#include "Sundries.h"
#include <Preferences.h>
#include "DisplayPanel.h"

#include "DigitDisplay.h"

#include "RefProvider.h"
//#include "UiEnums.h"

// for esp_now satellite (remote listener)
#include "espSatellite.h"



void Sundries::begin(RefProvider *refProvider) {
  this->refProvider = refProvider;

  displaySchemeInit();

  displayDoor_init();

  DSF_init();

  buzzer_init(BUZZER_PWM_PIN);
  
  lightMeter_init(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_DEFAULT_MTREG);

  // the following just does basic initialisation,
  // it does not start/enable the espNow stuff
  initEspNow();

}

void Sundries::initEspNow() {
  ES_listenerInit();
}
void Sundries::startEspNow() {
  // Initilize ESP-NOW / listener stuff
Serial.println("about to ES_espNowStart()");
  ES_espNowStart();
}
void Sundries::stopEspNow() {
  ES_espNowStop();
}

void Sundries::loop() {
  checkWifiPrioritised();

  displayDoor_checkIdleTimeout();
  displayDoor_checkTweakingTimeout();

  DSF_loop();

  lightMeter_loop();

  // do any satellite data updates before servicing esp-now/satellite stuff
  if ( ES_haveListenerReady() ) {
    TimeSource &timeSource = refProvider->timeSource;
    ES_setTimerRunning( timeSource.selectedTimerRunning() );
    ES_setTimerRemainPer240( 240 - timeSource.getTimerProgress(240) );
    ES_setBuzzActive( buzzerOn );
    DigitDisplay &digitDisplay = refProvider->digitDisplay;
    ES_setFlashActive( digitDisplay.getFlashing() );
    ES_setHueValue( digitDisplay.getCurrentHue() );
    ES_setSatValue( digitDisplay.getCurrentSat() );
  }
  // service (static) listener stuff
  ES_listenerLoop(millis());
  // see if we have some command from the remote
  ESPNOW_command rxCommand = ES_getRxCommand();
  if ( rxCommand != ENC_nop ) {
    //UiEvent highLevelUiEvent = (rxCommand == ENC_button_press_short) ? UiEvent::ueAccept : UiEvent::ueAltAccept;
    bool bLongClick = rxCommand == ENC_button_press_long;
    Serial.printf("Sundries::loop() remote click (long=%d)\n", bLongClick);
    refProvider->timeSource.processRemoteButtonClick(bLongClick);
  }

}

// ********************************************************************************************************************************
// ********************************************************************************************************************************
// displayDoor / servo stuff
// ********************************************************************************************************************************

void Sundries::displayDoor_init() {
  displayDoor_loadConfig();
  if ( DD_doorPresent ) {
    ledcSetup(DISPLAY_DOOR_PWM_CHANNEL, DISPLAY_DOOR_PWM_FREQUENCY, DISPLAY_DOOR_PWM_TIMER_RESOLUTION);
    ledcAttachPin(DISPLAY_DOOR_PWM_SERVO_PIN, DISPLAY_DOOR_PWM_CHANNEL);

    displayDoor_open(true);
  }
}

void Sundries::displayDoor_loadConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgDisplayDoorNamespace, true);
  DD_doorPresent = preferences.getBool(cfgDisplayDoorPresent, DD_doorPresent);
  DD_dutyOpen = preferences.getUInt(cfgDisplayDoorDutyOpen, DD_dutyOpen);
  DD_dutyShut = preferences.getUInt(cfgDisplayDoorDutyShut, DD_dutyShut);
  preferences.end();
}

void Sundries::displayDoor_saveConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgDisplayDoorNamespace, false);
  preferences.putBool(cfgDisplayDoorPresent, DD_doorPresent);
  preferences.putUInt(cfgDisplayDoorDutyOpen, DD_dutyOpen);
  preferences.putUInt(cfgDisplayDoorDutyShut, DD_dutyShut);
  preferences.end();
}


void Sundries::displayDoor_tweakDutyValueBegin(bool bAdjustOpenDuty) {
  DD_dutyTweakForDoorOpen = bAdjustOpenDuty;
  // go to mid position and start timer
  displayDoor_tweakingGotoMid();
  displayDoor_primeTweakingTimeout();
}
void Sundries::displayDoor_tweakDutyValueEnd() {
  DD_dutyTweakingTimeoutTimeMS = 0;
  displayDoor_open(true, true);
}

uint32_t Sundries::displayDoor_getMinDuty() {
  return DD_minDuty;
}
uint32_t Sundries::displayDoor_getMaxDuty() {
  return DD_maxDuty;
}
uint32_t Sundries::displayDoor_getMidDuty() {
  return DD_midDuty;
}


uint32_t Sundries::displayDoor_getDutyOpen() {
  return DD_dutyOpen;
}
void Sundries::displayDoor_setDutyOpen(uint32_t value) {
  DD_dutyOpen = value;
  displayDoor_open(true, false);
}

uint32_t Sundries::displayDoor_getDutyShut() {
  return DD_dutyShut;
}
void Sundries::displayDoor_setDutyShut(uint32_t value) {
  DD_dutyShut = value;
  displayDoor_shut(true, false);
}

void Sundries::displayDoor_primeIdleTimeout() {
  DD_timeoutTimeMS = millis() + DD_TimeoutPeriodMS;
}
void Sundries::displayDoor_checkIdleTimeout() {
  if ( (DD_timeoutTimeMS > 0) && (millis() > DD_timeoutTimeMS) ) {
    displayDoor_idle();
    // if we're closing the door, now display the digit separator
    if ( DD_doorOpen == false ) {
      render_digit_separator();
    }
  }
}

void Sundries::displayDoor_idle() {
  if ( DD_doorPresent ) {
    ledcWrite(DISPLAY_DOOR_PWM_CHANNEL, 0); // duty-cycle=0 = hopefully stops servo chatter
  }
  DD_timeoutTimeMS = 0;
}

bool Sundries::displayDoor_getPresent() {
  return DD_doorPresent;
}
void Sundries::displayDoor_setPresent(bool value) {
  if ( DD_doorPresent != value ) {
    DD_doorPresent = value;
    displayDoor_saveConfig();
    if ( DD_doorPresent ) {
      displayDoor_init();
    }
  }
}


void Sundries::displayDoor_open(bool bForce, bool bSubdueServo) {
  if ( !DD_doorOpen || bForce ) {
    DD_doorOpen = true;
    DD_dutyTweakInMidPosition = false;
    displayDoor_setDuty(DD_dutyOpen, bSubdueServo);
  }
}
void Sundries::displayDoor_shut(bool bForce, bool bSubdueServo) {
  if ( DD_doorOpen || bForce ) {
    // hack... only clear the display if we're not doing servo tweaking
    if ( DD_dutyTweakingTimeoutTimeMS == 0 ) {
      refProvider->displayPanel.clearDisplay();
    }
    DD_doorOpen = false;
    DD_dutyTweakInMidPosition = false;
    displayDoor_setDuty(DD_dutyShut, bSubdueServo);
  }
}
void Sundries::displayDoor_tweakingGotoMid() {
  displayDoor_setDuty(DD_midDuty, false);
  DD_dutyTweakInMidPosition = true;
}

void Sundries::displayDoor_setDuty(uint32_t duty, bool bSubdueServo) {
  if ( DD_doorPresent ) {
    //Serial.printf("servo duty:%i\n", duty);      
    ledcWrite(DISPLAY_DOOR_PWM_CHANNEL, duty);
  }

  // if reqd (most normal circumstances) we prime a timer
  // which, when expired, disables the servo PWM 
  // in order to eliminate continuous servo chatter
  if ( bSubdueServo ) {
    displayDoor_primeIdleTimeout();
  }
  
}

// timer for tweak
void Sundries::displayDoor_primeTweakingTimeout() {
  DD_dutyTweakingTimeoutTimeMS = millis() + DD_dutyTweakingTimeoutPeriodMS;
}
void Sundries::displayDoor_checkTweakingTimeout() {
  if ( (DD_dutyTweakingTimeoutTimeMS > 0) && (millis() > DD_dutyTweakingTimeoutTimeMS) ) {
    displayDoor_primeTweakingTimeout();
    // if we're in the mid position then go to either open or shut position
    if ( DD_dutyTweakInMidPosition ) {
      if ( DD_dutyTweakForDoorOpen ) {
        displayDoor_open(true, false);
      } else {
        displayDoor_shut(true, false);
      }
      displayDoor_primeTweakingTimeout();
    } else {
      // go to mid position
      displayDoor_tweakingGotoMid();
    }
  }
}


// ********************************************************************************************************************************

// ********************************************************************************************************************************
// ********************************************************************************************************************************
// digit separator display
// ********************************************************************************************************************************

void Sundries::setDigitSeparatorFlashing(bool value) {
  if ( bDigitSeparatorFlashing != value ) {
    bDigitSeparatorFlashing = value;
    if ( bDigitSeparatorFlashing ) {
      DSF_primeTimeout();
    } else {
      DSF_TimeoutTimeMS = 0;
      DSF_setDisplayFullBrightness(true);
    }
  }
}

void Sundries::DSF_init() {
  bDigitSeparatorFlashing = false;
  DSF_TimeoutTimeMS = 0;
}

void Sundries::DSF_primeTimeout() {
  DSF_TimeoutTimeMS = millis() + DSF_PeriodMS;
}

void Sundries::DSF_loop() {
  if ( DSF_TimeoutTimeMS != 0 ) {
    if ( millis() > DSF_TimeoutTimeMS ) {
      if ( bDigitSeparatorFlashing ) {
        DSF_setDisplayFullBrightness(!bDigitSeparatorFlashFull);
      }
      DSF_primeTimeout();
    }
  }
}

void Sundries::DSF_setDisplayFullBrightness(bool value) {
  if ( bDigitSeparatorFlashFull != value ) {
    bDigitSeparatorFlashFull = value;
    refProvider->displayPanel.setFlashingDimmed(!value);
  }
}

void Sundries::redrawDigitSeparator() {
  render_digit_separator();
}

void Sundries::render_digit_separator() {

  // choose a colour that has about the same brightness as the digits,
  // we use a lighter colour if we have a display door becaue the
  // door attenuates the light to a certain degree, without a door
  // we need an extra dark colour
  int32_t sepColour = DD_doorPresent ? 0xce7f : 0x528A;

  DisplayPanel &displayPanel = refProvider->displayPanel;
  displayPanel.clearDisplay();

  DigitDisplayMode digitDisplayMode = refProvider->digitDisplay.getActiveDigitDisplayMode();
  //Serial.printf("digitDisplayMode=%d\n", digitDisplayMode);
  switch (digitDisplayMode) {
  case DigitDisplayModeTime: {
    const int32_t sepWd = 45;
    const int32_t sepHt = 45;

    const int32_t sepX = (TFT_WIDTH / 2) - (sepWd / 2);
    const int32_t sepY1 = 0;
    const int32_t sepY2 = TFT_HEIGHT - sepY1 - sepHt;

    //Serial.printf("x=%i\n", hhmmSepX);
    displayPanel.fillRect(sepX, sepY1, sepWd, sepHt, sepColour);
    displayPanel.fillRect(sepX, sepY2, sepWd, sepHt, sepColour);
    break;
  }
  
  case DigitDisplayModeDate: {
    const uint8_t vPitch = 32;
    const uint8_t hPitch = 18;
    const uint8_t dayHt = vPitch - 10;
    const uint8_t dayWd = hPitch - 6;
    
    uint8_t dayNo = 0;
    uint8_t rowY = 48;
    for (uint8_t weekNo = 0; weekNo < 5; weekNo++) {
      uint8_t colX = 8;
      for (uint8_t dow = 0; dow < 7; dow++) {
        dayNo++;
        if ( (dayNo < 3) || (dayNo > 32) ) {
          displayPanel.drawRect(colX, rowY, dayWd, dayHt, sepColour);
        } else {
          displayPanel.fillRect(colX, rowY, dayWd, dayHt, sepColour);
        }
        colX += hPitch;
      }
      rowY += vPitch;
    }
    break;
  }

  case DigitDisplayModeEdit:
  case DigitDisplayModeTimer: {
    const uint8_t hCentre = (TFT_WIDTH / 2);
    const uint8_t hEllipseRad = 40;
    const uint8_t vEllipseOffset = 55;
    const uint8_t vEllipseRad = 65;
    const uint8_t blockOuterWd = TFT_WIDTH;
    const uint8_t blockOuterHt = 40;
    const uint8_t blockBorderWd = 12;
    const uint8_t yNudgeUp = 3;

    displayPanel.fillEllipse(hCentre, vEllipseOffset-yNudgeUp, hEllipseRad, vEllipseRad, sepColour);
    displayPanel.fillEllipse(hCentre, TFT_HEIGHT - vEllipseOffset-yNudgeUp, hEllipseRad, vEllipseRad, sepColour);

    displayPanel.fillRect(0, -yNudgeUp, blockOuterWd, blockOuterHt, TFT_BLACK);
    displayPanel.fillRect(blockBorderWd, blockBorderWd-yNudgeUp, blockOuterWd - blockBorderWd*2, blockOuterHt-blockBorderWd*2, sepColour);

    displayPanel.fillRect(0, TFT_HEIGHT - blockOuterHt-yNudgeUp, blockOuterWd, blockOuterHt+yNudgeUp, TFT_BLACK);
    displayPanel.fillRect(blockBorderWd, TFT_HEIGHT-blockOuterHt+blockBorderWd-yNudgeUp, blockOuterWd - blockBorderWd*2, blockOuterHt-blockBorderWd*2, sepColour);
    break;
  }
  }

}
// ********************************************************************************************************************************




// ********************************************************************************************************************************
// buzzer
// ********************************************************************************************************************************
void Sundries::buzzer_init(u_int8_t buzzerPin) {
  ledcSetup(BUZZER_PWM_CHANNEL, BUZZER_PWM_FREQUENCY, BUZZER_PWM_TIMER_RESOLUTION);
  ledcAttachPin(buzzerPin, BUZZER_PWM_CHANNEL);
  buzzer_setDuty(0);
  buzzerOn = false;
}
void Sundries::buzzer_stop() {
  if ( buzzerOn ) {
    //Serial.println("Sundries::buzzer_stop()");
    buzzer_setDuty(0);
    buzzerOn = false;
  }
}
void Sundries::buzzer_start() {
  if ( !buzzerOn ) {
    //Serial.println("Sundries::buzzer_start()");    
    buzzer_setDuty(96);  
    buzzerOn = true;
  }
}
void Sundries::buzzer_setDuty(uint32_t duty) {
  //Serial.printf("buzzer duty:%i\n", duty);      
  ledcWrite(BUZZER_PWM_CHANNEL, duty);
}
// ********************************************************************************************************************************


// ********************************************************************************************************************************
// displayScheme
// ********************************************************************************************************************************
void Sundries::displaySchemeInit() {
  displaySchemeIdx = 0; // default = classic cyan/grey
  loadDisplaySchemeIdx();
  setDisplaySchemeIdx(displaySchemeIdx, false);
}
void Sundries::setDisplaySchemeIdx(uint8_t idx, bool bPersist) {
  if ( ( idx >=0 ) && ( idx < qtySchemes ) ) {
    //Serial.printf("Sundries::setDisplaySchemeIdx(%d, %d)\n", idx, bPersist);
    displaySchemeIdx = idx;
    displayScheme = schemes[displaySchemeIdx];
    if ( bPersist ) {
      saveDisplaySchemeIdx();
    }
  }
}

uint8_t Sundries::getDisplaySchemeIdx() {
  return displaySchemeIdx;
}

DisplayPanelColourScheme &Sundries::getDisplayScheme() {
  return displayScheme;
}

void Sundries::saveDisplaySchemeIdx() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgDisplaySchemeNamespace, false);
  preferences.putUChar(cfgDisplaySchemeIdx, displaySchemeIdx);
  preferences.end();
}
void Sundries::loadDisplaySchemeIdx() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgDisplaySchemeNamespace, true);
  displaySchemeIdx = preferences.getUChar(cfgDisplaySchemeIdx, displaySchemeIdx);
  preferences.end();
}
// ********************************************************************************************************************************

// ********************************************************************************************************************************
// ambient light measurement (BH1750) stuff
// ********************************************************************************************************************************
void Sundries::lightMeter_init(BH1750::Mode mode, uint8_t MTreg) {
  
  // required by BH1750 ambient light sensor (library)
  Wire.begin();

  // this is what the library defaults to
  lightMeter_mode = mode;
  lightMeter_MTreg = MTreg;

  // this init code assumes the two values are both valid
  lightMeter.begin(lightMeter_mode);
  lightMeter.setMTreg(lightMeter_MTreg);

  lightMeter_loadSmoothingQtySamples();
  
}

void Sundries::lightMeter_saveSmoothingQtySamples() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgLightMeterNamespace, false);
  preferences.putUChar(cfgLightMeterSmoothingQtySamples, lightMeter_qtySamples);
  preferences.end();
}
void Sundries::lightMeter_loadSmoothingQtySamples() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgLightMeterNamespace, true);
  lightMeter_setSmoothingQtySamples(preferences.getUChar(cfgLightMeterSmoothingQtySamples, lightMeter_qtySamples));
  preferences.end();
}

uint8_t Sundries::lightMeter_getSmoothingQtySamples() {
  return lightMeter_qtySamples;
}
void Sundries::lightMeter_setSmoothingQtySamples(uint8_t value) {
  if ( value < 1 ) {
    value = 1;
  } else if ( value > 10 ) {
    value = 10;
  }
  if ( lightMeter_qtySamples != value ) {;
    lightMeter_qtySamples = value;
    lightMeter_damped_sample = 0.0f;
    lightMeter_runningSampleSum = 0.0f;
    lightMeter_avgLuxX100 = 0;
  }
}


BH1750::Mode Sundries::lightMeter_getMode() {
  return lightMeter_mode;
}
void Sundries::lightMeter_setMode(BH1750::Mode mode) {
  if ( lightMeter.configure(mode) ) {
    lightMeter_mode = mode;
  }
}

uint8_t Sundries::lightMeter_getMtRegVal() {
  return lightMeter_MTreg;
}
void Sundries::lightMeter_setMtRegVal(uint8_t value) {
  if ( value != lightMeter_MTreg ) {
    if ( lightMeter.setMTreg(value) ) {
      lightMeter_MTreg = value;
      //Serial.printf("lightMeter_setMtRegVal(%d)\n", lightMeter_MTreg);
    }
  }
}

uint32_t Sundries::lightMeter_getLuxX100() {
  return lightMeter_avgLuxX100;
}


void Sundries::lightMeter_loop() {

  if ( lightMeter.measurementReady(true) ) {
//uint32_t oldAvgLuxX100 = lightMeter_avgLuxX100;

    float fSample = lightMeter.readLightLevel();

    bool bSmoothing = ( lightMeter_qtySamples > 1 );
    if ( bSmoothing ) {
      lightMeter_runningSampleSum += fSample - lightMeter_damped_sample;
      lightMeter_damped_sample = lightMeter_runningSampleSum / lightMeter_qtySamples;
    } else {
      lightMeter_damped_sample = fSample;
    }

    // adjust sensitivity for different light levels:
    // (this simplistic logic based on https://github.com/claws/BH1750/blob/master/examples/BH1750autoadjust/BH1750autoadjust.ino )
// if (damped_sample < 2.0) {
//   lightMeter_setMtRegVal(200);
// } else if (damped_sample < 4.0) {
//   lightMeter_setMtRegVal(150);
// } else if (damped_sample < 8.0) {
//   lightMeter_setMtRegVal(100);
// } else if ( damped_sample < 40000.0 ) {
//   lightMeter_setMtRegVal(BH1750_DEFAULT_MTREG);
// } else {
//   lightMeter_setMtRegVal(32);
// }

//05/06/2024 leave sampling rate at default (69), to stop alternating between two values <8 & >8
// if (lightMeter_damped_sample < 8.0) {
//   lightMeter_setMtRegVal(100);
// } else if ( lightMeter_damped_sample < 40000.0 ) {
//   lightMeter_setMtRegVal(BH1750_DEFAULT_MTREG);
// } else {
//   lightMeter_setMtRegVal(32);
// }

    uint32_t newAvgLuxX100 = (uint32_t)round(lightMeter_damped_sample * 100.0f);
    
    //Serial.printf("sample=%9.2f, lightMeter_runningSampleSum=%9.2f, lightMeter_damped_sample=%9.2f, x100=%d\n", fSample, runningSampleSum, damped_sample, newAvgLuxX100);

    // 04/06/2024 only notice & deal with the value if we're not in wifi-priority mode
    // (else, we remember the new value, pass it on to the receivers, but they may ignore it due to wifi-priority
    //  and then they never see this new value unless it changes again (in our point of view))
    // (yes, this will have the side effect that we won't set the desired brightnesses until we have wifi, but hey)
    if ( !getWifiPrioritised() ) {
      //09/03/2024 (a bright day, in office) I found the ambient light adjust excessively happening
      //and futzed around but eventually just bumped minDiff from 10.. to 20.. then to 32
      //this doesn't seem like the right way to solve this... and may bite me later
      const uint8_t minDiff = 32;
      uint32_t change = abs( static_cast<int>(newAvgLuxX100 - lightMeter_avgLuxX100) );
      if ( change != 0 ) {
        if ( (change >= minDiff) || (newAvgLuxX100 < minDiff) ) {
  //Serial.printf("lightMeter_loop() sample=%9.2f, runningSampleSum=%9.2f, damped_sample=%9.2f, avgLuxX100=%d, newAvgLuxX100=%d, change=%d\n", fSample, lightMeter_runningSampleSum, lightMeter_damped_sample, lightMeter_avgLuxX100, newAvgLuxX100, change);
          lightMeter_avgLuxX100 = newAvgLuxX100;
  //Serial.printf("dealing with change(%d) of avgLuxX100(%d)\n", change, lightMeter_avgLuxX100);
          refProvider->digitDisplay.changedLuxX100(lightMeter_avgLuxX100, bSmoothing);
          refProvider->displayPanel.changedLuxX100(lightMeter_avgLuxX100, bSmoothing);
        }
      }
    }
  }

}

// ********************************************************************************************************************************
// random stuff
// ********************************************************************************************************************************

bool Sundries::getWifiPrioritised() {
  return wifiPrioritised;
}
void Sundries::setWifiPrioritised(bool value) {
  wifiPrioritised = value;
}

void Sundries::checkWifiPrioritised() {
  wifiPrioritised = !(refProvider->wifiExecutive.connected() && refProvider->timeSource.timeValid());
}

