//MARK 2023-12-06
#include <Preferences.h>
#include "TimeSource.h"
#include "RefProvider.h"
#include "Sundries.h"
#include "DigitDisplay.h"
#include "DisplayPanel.h"

#include <ESP32Time.h>

ESP32Time rtc;

void RtcBegin() {

}
time_t RtcGet() {
  unsigned long rtcEpoch = rtc.getEpoch();
  Serial.printf("RtcGet(): rtcEpoch=%d\n", rtcEpoch);
  return rtcEpoch;
}
void RtcSet(time_t tt) {
  rtc.setTime(tt);
}

void TimeSource::begin(RefProvider *refProvider) {
  this->refProvider = refProvider;

  bDisableUI = false;

  editMode = tsemNone;

  setSelectedTimer(-1);

  RtcBegin();
  
  // set defaults for timers
  for ( uint8_t timerIdx = 0; timerIdx < qtyTimers; timerIdx++ ) {
    timersData[timerIdx].timerIdx = timerIdx;
    sprintf(timersData[timerIdx].name, "t%01d", timerIdx);
    sprintf(timersData[timerIdx].cfgKeyPreset, "%sPreset", timersData[timerIdx].name);
    sprintf(timersData[timerIdx].cfgKeyBuzzer, "%sBuzzer", timersData[timerIdx].name);
    timersData[timerIdx].captionContent = ccNothing;
    timersData[timerIdx].buzzerDurationSecs = 30*60;
    timersData[timerIdx].timerExpired = false;
    timersData[timerIdx].currentDurationSecs = 0;
    timersData[timerIdx].currentEndTimeSecs = 0;
    timersData[timerIdx].buzzerEndTimeSecs = 0;
  }

  timersData[0].presetDurationSecs = 60 *  5;
  timersData[1].presetDurationSecs = 60 * 10;
  timersData[2].presetDurationSecs = 60 * 15;
  timersData[3].presetDurationSecs = 60 * 30;
  timersData[4].presetDurationSecs = 60 * 60;

  daylightDayOfMonth = 0;
  bHaveDaylightHours = false;
  modDaylightBegins = 0;
  modDaylightEnds = 0;


  loadConfig();

  applyOffsetAndDst();

}

void TimeSource::kickstartNtpClientEtc() {
  //Serial.println("TimeSource::kickstartNtpClientEtc()");
  // hoping it's safe to repeatedly call the following
  ntpTimeClient.begin();
  //I think the following gets done soon anyway, as a consequence of setSyncProvider()
  //ntpTimeClient.update();
  // leftover...
  Serial.print("NTP time = ");
  Serial.println(ntpTimeClient.getFormattedTime());

  // the following looks safe to repeatedly call
  // and doing so should cause an immediate resync (ie: go for NTP time)
  setSyncProvider(&TimeSource::syncProvider);
}

void TimeSource::forceTimerDisplayMode(bool bPaintDigitSeparator) {
  DigitDisplayMode digitDisplayMode = refProvider->digitDisplay.getActiveDigitDisplayMode();
  //Serial.printf("forceTimerDisplayMode(%d): digitDisplayMode=%d\n", bPaintDigitSeparator, digitDisplayMode);
  if ( (digitDisplayMode != DigitDisplayModeTimer) && (digitDisplayMode != DigitDisplayModeEdit) ) {
    Serial.println("TimeSource::forceTimerDisplayMode()");  
    refProvider->digitDisplay.setActiveDigitDisplayMode(DigitDisplayModeTimer, true);
    if ( bPaintDigitSeparator ) {
      // tell sundries that the digit separator needs to be redrawn
      refProvider->sundries.redrawDigitSeparator();
    }
  }
}


bool TimeSource::getDstActive() {
  return bDstActive;
}
void TimeSource::setDstActive(bool value) {
  bDstActive = value;
  applyOffsetAndDst();
}
uint8_t TimeSource::getUtcOffsetIdx() {
  return utcOffsetIdx;
}
void TimeSource::setUtcOffsetIdx(uint8_t idx) {
  utcOffsetIdx = idx;
  applyOffsetAndDst();  
}

const char *TimeSource::getUtcOffsetLabel(uint8_t idx) {
  return utcOffsetData[idx].label;
}



bool TimeSource::loop() {

  // take a snapshot of the time into a local var,
  // all time-value getters will use this snapshot and calculate the relevant value,
  // doing it this way avoids any digit rollover part way through a sequence of calls
  // (in case that can actually happen, I'm not really sure)
  timeSnapShot = rtc.getEpoch();
  timeSnapShotNoOffset = rtc.getLocalEpoch();

  calculateDaylightPeriodOnDayChange();
  bool bNewInDaylight = inDaylight();
  if ( bNewInDaylight != bOldInDaylight ) {
    bOldInDaylight = bNewInDaylight;
    refProvider->digitDisplay.checkDaylightBoost(false);
    refProvider->displayPanel.checkDaylightBoost(false);
  }

  loopSelectedTimer();

  return bDisableUI;
}

void TimeSource::calculateDaylightPeriodOnDayChange() {
  // if daylightBoost enabled and we have a valid time
  if ( couldShouldCalculateDaylightHours() ) {  
    // if a change of day then recalc sunrise & sunset times
    uint8_t dayOfMonth = day(timeSnapShot);
    if ( daylightDayOfMonth != dayOfMonth ) {
      daylightDayOfMonth = dayOfMonth;
      calculateDaylightPeriod();
    }
  }
}

void TimeSource::calculateDaylightPeriod() {
  // Note: it is understood that this calculation fails if combinations
  //  of date, latitude & desired sun-altitude, cause us to be never 
  //  - or always - in so called daylight - and it can't tell the difference
  //  between being in the "never" or "always" zone.
  // if daylightBoost enabled and we have a valid time
  if ( couldShouldCalculateDaylightHours() ) {
    sun.setPosition(locLat, locLon, (double)(rtc.offset / 3600));
    sun.setCurrentDate(getYear(), getMonth(), getDay());

    // note we allow config of "sunAlt" which is degrees *above* horizon that we consider we are in
    // daylight and we use this to calculate the library's desired sun-angle as described below:
    //
    // for the sun. library funcs:
    // [These] function[s] will return the [sunrise/sunset] in local time for your location for any
    // angle over the horizon, where < 90 would be above the horizon, and > 90 would be at or below.
    double sunAngle = 90 - sunAlt;

    double dblModDaylightBegins = sun.calcCustomSunrise(sunAngle);
    double dblModDaylightEnds = sun.calcCustomSunset(sunAngle);

    bHaveDaylightHours = ( !isnan(dblModDaylightBegins) && !isnan(dblModDaylightEnds) );
    if ( bHaveDaylightHours ) {
      modDaylightBegins = sun.calcCustomSunrise(sunAngle);
      modDaylightEnds = sun.calcCustomSunset(sunAngle);
    } else {
      modDaylightBegins = 0;
      modDaylightEnds = 0;
    }

  //Serial.printf("modDaylightBegins=%d, modDaylightEnds=%d\n", modDaylightBegins, modDaylightEnds);
  } else {
    bHaveDaylightHours = false;
  }//endif can and want to do daylight boost calculations
}


void TimeSource::processUiEvent(UiEvent highLevelUiEvent) {

  // deal with any UI input
  if ( highLevelUiEvent != UiEvent::ueNoEvent ) {
      
    if ( selectedTimerIdx != -1 ) {
      TimerData &timerData = timersData[selectedTimerIdx];

      // deal with increase/decrease - valid only while in some kind of edit mode
      if ( editMode != tsemNone ) {
        const int16_t valueStepLo = 60;
        const int16_t valueStepHi = 300;
        int16_t valueStep = 0;

        switch ( highLevelUiEvent ) {
        case UiEvent::ueDecrease:
          valueStep = -valueStepLo;
          break;

        case UiEvent::ueIncrease: 
          valueStep = +valueStepLo;
          break;

        case UiEvent::ueAltDecrease: 
          valueStep = -valueStepHi;
          break;

        case UiEvent::ueAltIncrease: 
          valueStep = +valueStepHi;
          break;
        }

        if ( valueStep != 0 ) {
          int32_t newEditValueSecs = editValueSecs;

          // force the new value to a multiple of valueStepLo
          newEditValueSecs -= (newEditValueSecs % valueStepLo);

          // adjust by step value
          newEditValueSecs += valueStep;

          // get min max values, so we can...
          int32_t minValue = (editMode == tsemTimerBuzzerDuration) ? 0 : 60;
          int32_t maxValue = (editMode == tsemTimerBuzzerDuration) ? 3*60*60 : 24*60*60-60;

          // ...force to be within min/max
          if ( newEditValueSecs < minValue ) {
            newEditValueSecs = minValue;
          } else if ( newEditValueSecs > maxValue ) {
            newEditValueSecs = maxValue;
          }

          editValueSecs = newEditValueSecs;
        }//endif had a step change

      }//endif in some kind of edit

      // now deal with accept/altAccept (applicable whether or not we're in edit)
      switch (highLevelUiEvent) {
      case UiEvent::ueAccept: 
        // if timer expired then leave timer mode altogether
        if ( timerData.timerExpired ) {
          leaveTimerMode();
        } else {
          // else (if timer not expired) then toggle pause / run
          if ( editMode == tsemNone ) {
            pauseSelectedTimer(true);
            startEditing(tsemTimerCurrentDuration);
          } else {
            stopEditing(true);
            pauseSelectedTimer(false);
          }
        }
        break;
      case UiEvent::ueAltAccept: 
        // cancel timer altogether
        stopEditing(false);     // ensure we aren't left in edit mode
        leaveTimerMode();
        break;
      }
    }//endif we have a timer selected
  }//endif there was some ui input

}

void TimeSource::processRemoteButtonClick(bool bLongClick) {

  if ( selectedTimerIdx != -1 ) {
    TimerData &timerData = timersData[selectedTimerIdx];
    if ( editMode == tsemNone ) {
      // if the timer has expired then any kind of remote input dismisses it
      if ( timerData.timerExpired ) {
        leaveTimerMode();
      } else {
        // only accept long click - which cancels a running timer
        if ( bLongClick ) {
          if ( selectedTimerRunning() ) {
            leaveTimerMode();
          }
        }
      }
    }//endif not-editing
  }

}

void TimeSource::processWifiStateChange(WifiState newWifiState) {
  // deal with any wifi change
  if ( newWifiState != WS_null ) {
    // if just (re)connected then (re)kickstart the NTP shit
    if ( newWifiState == WS_connected ) {
      delay(100);
      kickstartNtpClientEtc();
    }

    // TODO: decide whether we want to take notice of any other wifi state changes
  }
}

bool TimeSource::timeValid() {
  return timeStatus() == timeSet;
}

void TimeSource::applyOffsetAndDst() {
  rtc.offset = (utcOffsetData[utcOffsetIdx].offsetMinutes + (bDstActive ? 60 : 0) ) * 60;
}


// Static methods used for sync provider to TimeLib library.
time_t TimeSource::syncProvider() {
Serial.println("TimeSource::syncProvider()");
  time_t ntp_now, rtc_now, retVal;

  retVal = RtcGet();

  if (millis() - millis_last_ntp > refresh_ntp_every_ms || millis_last_ntp == 0) {
    // It's time to resync with NTP

    // can only do that if we have Wifi
//TODO: would prefer this use WifiExecutive - but can't easily as I am in a static and that's an instance variable
    if (WiFi.status() == WL_CONNECTED) {
  
      Serial.print("Getting NTP...");
//      ntpTimeClient.forceUpdate();  // maybe this breaks the NTP requests as this should not be done more than every minute.
      if (ntpTimeClient.update()) {
        Serial.print(".");
        ntp_now = ntpTimeClient.getEpochTime();
        Serial.printf("NTP time: %s\n", ntpTimeClient.getFormattedTime().c_str());
//      if (ntp_now > 1644601505) { //is it valid - reasonable number?
        // Sync the RTC to NTP if needed.
        rtc_now = RtcGet();        
        Serial.printf("NTP:%d, RTC:%d, diff:%d\n", ntp_now, rtc_now, ntp_now-rtc_now);
        if (ntp_now != rtc_now) {
          Serial.println("Updating RTC");
          RtcSet(ntp_now);
        }
        millis_last_ntp = millis();
        Serial.println("Using NTP time");
        retVal = ntp_now;
      } else {  // NTP valid
        Serial.println("Invalid NTP response, using RTC time");
      }
    } else {
      // no wifi
      Serial.println("Wanting NTP-sync but no WiFi, using RTC time");
    }
  } else {
    Serial.println("Not yet time for NTP-sync, using RTC time");
  }

  return retVal;
}


void TimeSource::loadConfig() {
  loadTimeOffsetData();
  loadDaylightData();
  for ( uint8_t timerIdx = 0; timerIdx < qtyTimers; timerIdx++ ) {
    loadTimerData(timerIdx);
  }
}

void TimeSource::saveTimeOffsetData() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgTimeSourceNamespace, false);
  preferences.putUChar(cfgTimeSourceUtcOffsetIdx, utcOffsetIdx);
  preferences.putBool(cfgTimeSourceDstActive, bDstActive);
  preferences.end();
}
void TimeSource::loadTimeOffsetData() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgTimeSourceNamespace, true);
  utcOffsetIdx = preferences.getUChar(cfgTimeSourceUtcOffsetIdx, utcOffsetIdx);
  bDstActive = preferences.getBool(cfgTimeSourceDstActive, bDstActive);
  preferences.end();
}

void TimeSource::saveDaylightData() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgTimeSourceNamespace, false);
  preferences.putBool(cfgTimeSourceDaylightBoostEnabled, bDaylightBoostEnabled);
  preferences.putShort(cfgTimeSourceLocationLat, locLat);
  preferences.putShort(cfgTimeSourceLocationLon, locLon);
  preferences.putChar(cfgTimeSourceDaylightSunAlt, sunAlt);
  preferences.end();
}
void TimeSource::loadDaylightData() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgTimeSourceNamespace, true);
  bDaylightBoostEnabled = preferences.getBool(cfgTimeSourceDaylightBoostEnabled, bDaylightBoostEnabled);
  locLat = preferences.getShort(cfgTimeSourceLocationLat, locLat);
  locLon = preferences.getShort(cfgTimeSourceLocationLon, locLon);
  sunAlt = preferences.getChar(cfgTimeSourceDaylightSunAlt, sunAlt);
  preferences.end();
}



void TimeSource::saveTimerData(uint8_t timerIdx) {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgTimeSourceNamespace, false);
  preferences.putULong(timersData[timerIdx].cfgKeyPreset, timersData[timerIdx].presetDurationSecs);
  preferences.putULong(timersData[timerIdx].cfgKeyBuzzer, timersData[timerIdx].buzzerDurationSecs);
  preferences.end();
}

void TimeSource::loadTimerData(uint8_t timerIdx) {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgTimeSourceNamespace, true);
  timersData[timerIdx].presetDurationSecs = preferences.getULong(timersData[timerIdx].cfgKeyPreset, timersData[timerIdx].presetDurationSecs);
  timersData[timerIdx].buzzerDurationSecs = preferences.getULong(timersData[timerIdx].cfgKeyBuzzer, timersData[timerIdx].buzzerDurationSecs);
  preferences.end();
}
void TimeSource::clearConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgTimeSourceNamespace, false);
  preferences.clear();
  preferences.end();
}


bool TimeSource::couldShouldCalculateDaylightHours() {
  return bDaylightBoostEnabled && timeValid();
}
bool TimeSource::haveValidDaylightHours() {
  return bHaveDaylightHours;
}

//todo: hack
DebugForceDaylight TimeSource::getForceInDaylight() {
  return debugForceDaylight;
}
void TimeSource::setForceInDaylight(DebugForceDaylight value) {
  debugForceDaylight = value;
}

bool TimeSource::getDaylightBoostEnabled() {
  return bDaylightBoostEnabled;
}
void TimeSource::setDaylightBoostEnabled(bool value) {
  bDaylightBoostEnabled = value;
  calculateDaylightPeriod();
}
int16_t TimeSource::getLatitude() {
  return locLat;
}
void TimeSource::setLatitude(int16_t value) {
  locLat = value;
  calculateDaylightPeriod();
}
int16_t TimeSource::getLongitude() {
  return locLon;
}
void TimeSource::setLongitude(int16_t value) {
  locLon = value;
  calculateDaylightPeriod();
}
int8_t TimeSource::getDaylightSunAlt() {
  return sunAlt;
}
void TimeSource::setDaylightSunAlt(int8_t value) {
  sunAlt = value;
  calculateDaylightPeriod();
}

bool TimeSource::inDaylight() {
  // default to being outside daylight period if not enabled/capable
  // or we don't have valid daylight start/end times
  bool retVal = false;
  if ( haveValidDaylightHours() ) {
    uint16_t minuteOfDay = getHour() * 60 + getMinute();
    retVal = ( (minuteOfDay >= modDaylightBegins) && (minuteOfDay < modDaylightEnds) );
  }

  //debug hack
  if ( debugForceDaylight == DebugForceDaylight::dfdForceDay ) {
    retVal = true;
  } else if ( debugForceDaylight == DebugForceDaylight::dfdForceNight ) {
    retVal = false;
  }
  
  return retVal;
}

uint16_t TimeSource::getModDaylightBegins() {
  return modDaylightBegins;
}
uint16_t TimeSource::getModDaylightEnds() {
  return modDaylightEnds;
}



// ****************************************************************
// timer stuff
TimerData &TimeSource::getTimerData(int8_t timerIdx) {
  return timersData[timerIdx];
}

char *TimeSource::getTimerCaption(int8_t timerIdx, CaptionContent captionContent) {
  char *retVal = (char *)"";
  if ( (timerIdx >=0) && (timerIdx < qtyTimers) ) {
    TimerData &timerData = timersData[timerIdx];
    // generate this caption & remember we have it
    uint32_t duration = 0;
    if (captionContent == ccDefault) {
      duration = timerData.presetDurationSecs;
    } else if (captionContent == ccCurrent) {
      duration = timerData.currentDurationSecs;
    }
    uint8_t ss = duration % 60;
    duration = duration / 60;
    uint8_t mm = duration % 60;
    duration = duration / 60;
    uint8_t hh = duration;
    sprintf(&timerData.caption[0], "%s [ %02d:%02d ]", timerData.name, hh, mm);
    timerData.captionContent = captionContent;

    retVal = timerData.caption;
  }
  return retVal;
}

void TimeSource::leaveTimerMode() {
  setSelectedTimer(-1);

  // tell sundries that the digit separator needs to be redrawn
  refProvider->sundries.redrawDigitSeparator();

}

void TimeSource::setSelectedTimer(int8_t timerIdx) {
  // unselect any existing selectedTimer
  // (for now I can't cope with multiple timers active)
  resetSelectedTimer();
  selectedTimerIdx = timerIdx;
  if ( selectedTimerIdx == -1 ) {
    refProvider->digitDisplay.autoSelectDigitDisplayMode();
  } else {
    resetSelectedTimer();
    bDisableUI = true;
    forceTimerDisplayMode(false);
  }
}

int8_t TimeSource::getSelectedTimer() {
  return selectedTimerIdx;
}
void TimeSource::resetSelectedTimer() {
  if ( selectedTimerIdx != -1 ) {
    TimerData &timerData = timersData[selectedTimerIdx];
    timerData.timerExpired = false;
    refProvider->digitDisplay.setFlashReducePct(0);             // equiv to flashOff
    primeWorkingDurations(timerData);
    timerData.currentEndTimeSecs = 0;   // sets as paused
    timerData.buzzerEndTimeSecs = 0;
    bDisableUI = false;
    refProvider->sundries.buzzer_stop();
  }
}

void TimeSource::primeWorkingDurations(TimerData &timerData) {
  timerData.workingPresetSecs = timerData.presetDurationSecs;
  timerData.currentDurationSecs = timerData.workingPresetSecs;
}

//TODO: I think I thought that this would be better as HI & LO like edit
// and put the is-it-hhmm-or--mmss decision in here rather than in the DigitDisplay
//...not sure why, maybe something to do with digits dancing around between edit & run etc
void TimeSource::getTimerDigits(uint8_t *hh, uint8_t *mm, uint8_t *ss) {
  uint8_t HH = 23;
  uint8_t MM = 59;
  uint8_t SS = 59;
  if ( selectedTimerIdx != -1 ) {
    TimerData &timerData = timersData[selectedTimerIdx];
    uint32_t timeValueSecs = 0;

    if ( editMode == tsemNone ) {
      // show either preset duration, current duration, or overrun time, depending on whether timer is running/expired
      if ( selectedTimerRunning() ) {
        timeValueSecs = timerData.currentDurationSecs;
      } else if ( timerData.timerExpired ) {
        // timer has expired, so return value is the overrun value else is the time remaining
        timeValueSecs = timeSnapShotNoOffset - timerData.currentEndTimeSecs;
        // cap the overrun time to 99:59:59
        if ( timeValueSecs > maxOverrunSecs ) {
          timeValueSecs = maxOverrunSecs;
        }
      } else {
        // must be displaying preset duration
        timeValueSecs = timerData.presetDurationSecs;
      }
    }

    HH = hour(timeValueSecs);
    MM = minute(timeValueSecs);
    SS = second(timeValueSecs);
  }

  *hh = HH;
  *mm = MM;
  *ss = SS;
}

void TimeSource::getEditDigits(uint8_t *hi, uint8_t *lo) {
  uint8_t HI = 23;
  uint8_t LO = 59;

  switch (editMode) {
  case tsemTimerPresetDuration:
    // timer preset is always HHMM, never seconds
    HI = hour(editValueSecs);
    LO = minute(editValueSecs);
    break;
  case tsemTimerCurrentDuration:
    // current duration shows either HHMM or MMSS depending on magnitude
    if ( editValueSecs < 3600 ) {
      HI = minute(editValueSecs);
      LO = second(editValueSecs);
    } else {
      HI = hour(editValueSecs);
      LO = minute(editValueSecs);
    }
    break;
  case tsemTimerBuzzerDuration:
    // buzzer duration is always HHMM
    HI = hour(editValueSecs);
    LO = minute(editValueSecs);
    break;
  }
  *hi = HI;
  *lo = LO;

}

uint16_t TimeSource::getTimerProgress(uint16_t maxValue) {
  uint16_t retVal = 0;
  if ( selectedTimerIdx != -1 ) {
    TimerData &timerData = timersData[selectedTimerIdx];
    if ( editMode != tsemNone ) {
      retVal = 0;
    } else if (timerData.timerExpired) {
      retVal = maxValue;
    } else {
      if ( timerData.workingPresetSecs != 0 ) {
        uint32_t timerElapsedSecs = timerData.workingPresetSecs - timerData.currentDurationSecs;
        retVal = (1L * maxValue * timerElapsedSecs) / timerData.workingPresetSecs;
      }
    }
  }
  return retVal;
}

bool TimeSource::selectedTimerRunning() {
  bool retVal= false;
  if ( selectedTimerIdx != -1 ) {
    TimerData &timerData = timersData[selectedTimerIdx];
    retVal = (timerData.currentEndTimeSecs != 0) && !timerData.timerExpired;
    //Serial.printf("TimeSource::selectedTimerRunning() retVal=%d\n", retVal);
  }
  return retVal;
}
bool TimeSource::selectedTimerExpired() {
  bool retVal= false;
  if ( selectedTimerIdx != -1 ) {
    TimerData &timerData = timersData[selectedTimerIdx];
    retVal = timerData.timerExpired;
  }
  return retVal;
}

void TimeSource::pauseSelectedTimer(bool bDoPause) {
  if ( selectedTimerIdx != -1 ) {
    forceTimerDisplayMode(true);
    TimerData &timerData = timersData[selectedTimerIdx];
    if ( bDoPause ) {
      timerData.currentEndTimeSecs = 0;
    } else {
      // unpausing, (re)prime end time
      timerData.currentEndTimeSecs = timeSnapShotNoOffset + timerData.currentDurationSecs;
    }
  }
}

void TimeSource::startEditing(TimeSourceEditMode newEditMode) {
  if ( newEditMode == tsemNone ) {
    stopEditing(false);
  } else {
    if ( selectedTimerIdx != -1 ) {
      editMode = newEditMode;
      TimerData &timerData = timersData[selectedTimerIdx];
      switch (editMode) {
      case tsemTimerPresetDuration:
        editValueSecs = timerData.presetDurationSecs;
        break;
      case tsemTimerCurrentDuration:
        editValueSecs = timerData.currentDurationSecs;
        break;
      case tsemTimerBuzzerDuration:
        editValueSecs = timerData.buzzerDurationSecs;
        break;
      }
      refProvider->digitDisplay.startEditMode();
    }
  }
}
void TimeSource::stopEditing(bool bAcceptValue) {
  if ( editMode != tsemNone ) {
    if ( selectedTimerIdx != -1 ) {
      if ( bAcceptValue ) {
        TimerData &timerData = timersData[selectedTimerIdx];
        switch (editMode) {
        case tsemTimerPresetDuration:
          timerData.presetDurationSecs = editValueSecs;
          primeWorkingDurations(timerData);
          break;
        case tsemTimerCurrentDuration:
          // if the currentDurationSecs has changed value then it's anyone's
          // guess what the workingPresetSecs should be considered to be as
          // regards current progress & colour-lerping, so we will just reset
          // the workingPresetSecs to the new current value so basically we
          // consider the progression is: from now to the expiry time
          if ( editValueSecs != timerData.currentDurationSecs ) {
            timerData.workingPresetSecs = editValueSecs;
          }
          timerData.currentDurationSecs = editValueSecs;
          break;
        case tsemTimerBuzzerDuration:
          timerData.buzzerDurationSecs = editValueSecs;
          break;
        }
      }
      editMode = tsemNone;
      refProvider->digitDisplay.stopEditMode();
    }
  }
}


void TimeSource::loopSelectedTimer() {
  bool bFlashDisplayPanel = false;

  if ( selectedTimerIdx != -1 ) {
    TimerData &timerData = timersData[selectedTimerIdx];
    if ( (timerData.currentEndTimeSecs != 0) && !timerData.timerExpired ) {
      // non-zero currentEndTimeSecs (and not expired) means we are running (not paused)
      if ( timeSnapShotNoOffset < timerData.currentEndTimeSecs ) {
        timerData.currentDurationSecs = timerData.currentEndTimeSecs - timeSnapShotNoOffset;
        // if running timer >= 1hr, make displayPanel flash because you can't see the 
        // seconds ticking which is a give away that we're looking at a timer
        bFlashDisplayPanel = ( timerData.currentDurationSecs >= 3600 );
      } else {
        // timer expired
        timerData.timerExpired = true;
        bool dispModeChanged = refProvider->digitDisplay.autoSelectDigitDisplayMode();
        if ( dispModeChanged ) {
          // tell sundries that the digit separator needs to be redrawn
          refProvider->sundries.redrawDigitSeparator();
        }
        
        refProvider->digitDisplay.setFlashReducePct(20);
        // potentially raise alarm (start buzzer)          
        if ( timerData.buzzerDurationSecs > 0 ) {
          timerData.buzzerEndTimeSecs = timeSnapShotNoOffset + timerData.buzzerDurationSecs;
          // start buzzer
          refProvider->sundries.buzzer_start();
        }//endif start buzzer
      }//endif/else timer expired
    }//endif we have a timer ending time

    // see if we need to stop buzzer
    if ( timerData.buzzerEndTimeSecs != 0 ) {
      if ( timeSnapShotNoOffset >= timerData.buzzerEndTimeSecs ) {
        // stop buzzer
        refProvider->sundries.buzzer_stop();
        timerData.buzzerEndTimeSecs = 0;
      }//endif buzzer time expired
    }//endif we had a buzzer going
  }//endif we have a timer selected

  refProvider->sundries.setDigitSeparatorFlashing( bFlashDisplayPanel );
}

uint32_t TimeSource::millis_last_ntp = 0;
WiFiUDP TimeSource::ntpUDP;
NTPClient TimeSource::ntpTimeClient(ntpUDP);

