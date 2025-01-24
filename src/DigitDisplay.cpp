//MARK 2023-12-08 before change of globalBrightness to desiredBrightness & ledBrightness
//#include <stdlib.h>

#include <Preferences.h>
#include "DigitDisplay.h"
#include "RefProvider.h"
#include "DisplayPanel.h"

#include "Sundries.h"

#include "lib8tion.h"

#include "LedDefs.h"

void DigitDisplay::begin(RefProvider *refProvider) {
  this->refProvider = refProvider;

  lerpTestVal = -1;

  bDoingSoakTest = false;
  bDoingNYC = false;
  nycCurrentYear = 0;
  bDoingGammaValueAdjust = false;

  // set some initial flashing vars
  bFlashing = false;
  bFlashFullNow = false;
  flashPhaseChangeTimeMS = 0;

  blankAllLeds();

  pauseUpdates();

  resetAnimationData();

  invalidateAllDigits();

  // set up initial mode data

  // time
  digitDisplayModeDataList[DigitDisplayModeTime].digitDisplayMode = DigitDisplayModeTime;
  digitDisplayModeDataList[DigitDisplayModeTime].name = "modeTime";
  digitDisplayModeDataList[DigitDisplayModeTime].seriffed = true;
  digitDisplayModeDataList[DigitDisplayModeTime].softened = true;
  digitDisplayModeDataList[DigitDisplayModeTime].fxEnabled = false;
  digitDisplayModeDataList[DigitDisplayModeTime].fxStroke = true;
  digitDisplayModeDataList[DigitDisplayModeTime].fxUnstroke = false;
  digitDisplayModeDataList[DigitDisplayModeTime].fxGrow = false;
  digitDisplayModeDataList[DigitDisplayModeTime].fxShrink = true;
  digitDisplayModeDataList[DigitDisplayModeTime].fxFadeIn = true;
  digitDisplayModeDataList[DigitDisplayModeTime].fxFadeOut = true;
  digitDisplayModeDataList[DigitDisplayModeTime].flashReducePct = 0;
  digitDisplayModeDataList[DigitDisplayModeTime].colourData1.hue = 160;
  digitDisplayModeDataList[DigitDisplayModeTime].colourData1.sat = 240;
  digitDisplayModeDataList[DigitDisplayModeTime].colourData2.hue = 160+256;
  digitDisplayModeDataList[DigitDisplayModeTime].colourData2.sat = 240;
  
  // date
  digitDisplayModeDataList[DigitDisplayModeDate].digitDisplayMode = DigitDisplayModeDate;
  digitDisplayModeDataList[DigitDisplayModeDate].name = "modeDate";
  digitDisplayModeDataList[DigitDisplayModeDate].seriffed = true;
  digitDisplayModeDataList[DigitDisplayModeDate].softened = true;
  digitDisplayModeDataList[DigitDisplayModeDate].fxEnabled = false;
  digitDisplayModeDataList[DigitDisplayModeDate].fxStroke = false;
  digitDisplayModeDataList[DigitDisplayModeDate].fxUnstroke = false;
  digitDisplayModeDataList[DigitDisplayModeDate].fxGrow = false;
  digitDisplayModeDataList[DigitDisplayModeDate].fxShrink = false;
  digitDisplayModeDataList[DigitDisplayModeDate].fxFadeIn = true;
  digitDisplayModeDataList[DigitDisplayModeDate].fxFadeOut = true;
  digitDisplayModeDataList[DigitDisplayModeDate].flashReducePct = 0;
  digitDisplayModeDataList[DigitDisplayModeDate].colourData1.hue = 160;
  digitDisplayModeDataList[DigitDisplayModeDate].colourData1.sat = 60;
  digitDisplayModeDataList[DigitDisplayModeDate].colourData2.hue = 160+256;
  digitDisplayModeDataList[DigitDisplayModeDate].colourData2.sat = 60;

  // timer
  digitDisplayModeDataList[DigitDisplayModeTimer].digitDisplayMode = DigitDisplayModeTimer;
  digitDisplayModeDataList[DigitDisplayModeTimer].name = "modeTimer";
  digitDisplayModeDataList[DigitDisplayModeTimer].seriffed = false;
  digitDisplayModeDataList[DigitDisplayModeTimer].softened = false;
  digitDisplayModeDataList[DigitDisplayModeTimer].fxEnabled = false;
  digitDisplayModeDataList[DigitDisplayModeTimer].fxStroke = false;
  digitDisplayModeDataList[DigitDisplayModeTimer].fxUnstroke = false;
  digitDisplayModeDataList[DigitDisplayModeTimer].fxGrow = false;
  digitDisplayModeDataList[DigitDisplayModeTimer].fxShrink = false;
  digitDisplayModeDataList[DigitDisplayModeTimer].fxFadeIn = true;
  digitDisplayModeDataList[DigitDisplayModeTimer].fxFadeOut = true;
  digitDisplayModeDataList[DigitDisplayModeTimer].flashReducePct = 0;
  digitDisplayModeDataList[DigitDisplayModeTimer].colourData1.hue = 96;
  digitDisplayModeDataList[DigitDisplayModeTimer].colourData1.sat = 48;
  digitDisplayModeDataList[DigitDisplayModeTimer].colourData2.hue = 0;
  digitDisplayModeDataList[DigitDisplayModeTimer].colourData2.sat = 255;

  // edit (used internally, not configurable)
  digitDisplayModeDataList[DigitDisplayModeEdit].digitDisplayMode = DigitDisplayModeEdit;
  digitDisplayModeDataList[DigitDisplayModeEdit].name = "modeTime";
  digitDisplayModeDataList[DigitDisplayModeEdit].seriffed = true;
  digitDisplayModeDataList[DigitDisplayModeEdit].softened = true;
  digitDisplayModeDataList[DigitDisplayModeEdit].fxEnabled = false;
  digitDisplayModeDataList[DigitDisplayModeEdit].fxStroke = false;
  digitDisplayModeDataList[DigitDisplayModeEdit].fxUnstroke = false;
  digitDisplayModeDataList[DigitDisplayModeEdit].fxGrow = false;
  digitDisplayModeDataList[DigitDisplayModeEdit].fxShrink = false;
  digitDisplayModeDataList[DigitDisplayModeEdit].fxFadeIn = false;
  digitDisplayModeDataList[DigitDisplayModeEdit].fxFadeOut = false;
  digitDisplayModeDataList[DigitDisplayModeEdit].flashReducePct = 20;
  digitDisplayModeDataList[DigitDisplayModeEdit].colourData1.hue = 0;
  digitDisplayModeDataList[DigitDisplayModeEdit].colourData1.sat = 0;
  digitDisplayModeDataList[DigitDisplayModeEdit].colourData2.hue = 0;
  digitDisplayModeDataList[DigitDisplayModeEdit].colourData2.sat = 0;

  bEffectsPreview = false;

  currentLuxX100 = 0;

  bAmbientAutoAdjustTempDisabled = false;

  // set reasonable defaults in case there's no config saved)
  minLuxX100 = 1;
  setMinBrightness_Hi(MIN_BRIGHTNESS);
  maxLuxX100 = 1800;
  setMaxBrightness_Hi(50);

  setDaylightBoost(0);

  loadConfig();

  setActiveDigitDisplayMode(DigitDisplayModeTime, true);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(ledBuffer, QTY_LEDS);
  setRgbCorrection(rgbCorrection, false);

  actualLedBrightness = 255;
  setTargetLedBrightness(0, true);

  brightness_Hi = 0;
  setUserBrightness(50);

  ambientAdjustBrightness(false);

  // we exit with the updates disabled, it is up to the caller to expressly enable (and update) when things are ready
}

void DigitDisplay::loadConfig() {
  loadDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeTime] );
  loadDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeDate] );
  loadDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeTimer] );

  // get the sundry config (which includes gammaValue)...
  loadSundriesConfig();

  // ...before all the ambient adjust stuff (which uses the gammaValue)
  loadAmbientAdjustConfig();
}
// unused
// void DigitDisplay::saveConfig() {
//   saveDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeTime] );
//   saveDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeDate] );
//   saveDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeTimer] );
//
//    saveSundriesConfig();
//
//    saveAmbientAdjustConfig();
// }

void DigitDisplay::clearConfig() {
  clearDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeTime] );
  clearDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeDate] );
  clearDigitDisplayModeData( &digitDisplayModeDataList[DigitDisplayModeTimer] );

  clearSundriesConfig();

  clearAmbientAdjustConfig();
  
}

void DigitDisplay::loadDigitDisplayModeData(DigitDisplayModeData *digitDisplayModeData) {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(digitDisplayModeData->name, true);
  digitDisplayModeData->seriffed =            preferences.getBool(  cfgModeDataKeySeriffed,       digitDisplayModeData->seriffed);
  digitDisplayModeData->softened =            preferences.getBool(  cfgModeDataKeySoftened,       digitDisplayModeData->softened);
  digitDisplayModeData->fxEnabled =           preferences.getBool(  cfgModeDataKeyFxEnabled,      digitDisplayModeData->fxEnabled);
  digitDisplayModeData->fxStroke =            preferences.getBool(  cfgModeDataKeyFxStroke,       digitDisplayModeData->fxStroke);
  digitDisplayModeData->fxUnstroke =          preferences.getBool(  cfgModeDataKeyFxUnstroke,     digitDisplayModeData->fxUnstroke);
  digitDisplayModeData->fxGrow =              preferences.getBool(  cfgModeDataKeyFxGrow,         digitDisplayModeData->fxGrow);
  digitDisplayModeData->fxShrink =            preferences.getBool(  cfgModeDataKeyFxShrink,       digitDisplayModeData->fxShrink);
  digitDisplayModeData->fxFadeIn =            preferences.getBool(  cfgModeDataKeyFxFadeIn,       digitDisplayModeData->fxFadeIn);
  digitDisplayModeData->fxFadeOut =           preferences.getBool(  cfgModeDataKeyFxFadeOut,      digitDisplayModeData->fxFadeOut);
  digitDisplayModeData->colourData1.hue =     preferences.getUShort(cfgModeDataKeyColour1Hue,     digitDisplayModeData->colourData1.hue);
  digitDisplayModeData->colourData1.sat =     preferences.getUChar( cfgModeDataKeyColour1Sat,     digitDisplayModeData->colourData1.sat);
  digitDisplayModeData->colourData2.hue =     preferences.getUShort(cfgModeDataKeyColour2Hue,     digitDisplayModeData->colourData2.hue);
  digitDisplayModeData->colourData2.sat =     preferences.getUChar( cfgModeDataKeyColour2Sat,     digitDisplayModeData->colourData2.sat);
  preferences.end();
}
void DigitDisplay::saveDigitDisplayModeData(DigitDisplayModeData *digitDisplayModeData) {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(digitDisplayModeData->name, false);
  preferences.putBool(cfgModeDataKeySeriffed,       digitDisplayModeData->seriffed);
  preferences.putBool(cfgModeDataKeySoftened,       digitDisplayModeData->softened);
  preferences.putBool(cfgModeDataKeyFxEnabled,      digitDisplayModeData->fxEnabled);
  preferences.putBool(cfgModeDataKeyFxStroke,       digitDisplayModeData->fxStroke);
  preferences.putBool(cfgModeDataKeyFxUnstroke,     digitDisplayModeData->fxUnstroke);
  preferences.putBool(cfgModeDataKeyFxGrow,         digitDisplayModeData->fxGrow);
  preferences.putBool(cfgModeDataKeyFxShrink,       digitDisplayModeData->fxShrink);
  preferences.putBool(cfgModeDataKeyFxFadeIn,       digitDisplayModeData->fxFadeIn);
  preferences.putBool(cfgModeDataKeyFxFadeOut,      digitDisplayModeData->fxFadeOut);
  preferences.putUShort(cfgModeDataKeyColour1Hue,   digitDisplayModeData->colourData1.hue);
  preferences.putUChar(cfgModeDataKeyColour1Sat,    digitDisplayModeData->colourData1.sat);
  preferences.putUShort(cfgModeDataKeyColour2Hue,   digitDisplayModeData->colourData2.hue);
  preferences.putUChar(cfgModeDataKeyColour2Sat,    digitDisplayModeData->colourData2.sat);
  preferences.end();
}
void DigitDisplay::clearDigitDisplayModeData(DigitDisplayModeData *digitDisplayModeData) {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(digitDisplayModeData->name, false);
  preferences.clear();
  preferences.end();
}

void DigitDisplay::loadAmbientAdjustConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgAaNamespace, true);
  minLuxX100 = preferences.getULong(cfgAaLuxMin, minLuxX100);
  setMinBrightness_Hi(preferences.getUChar(cfgAaLedMin, minBrightness_Hi));
  maxLuxX100 = preferences.getULong(cfgAaLuxMax, maxLuxX100);
  setMaxBrightness_Hi(preferences.getUChar(cfgAaLedMax, maxBrightness_Hi));
  setDaylightBoost(preferences.getUChar(cfgAaDayBoost, daylightBoost_Hi));
  preferences.end();
  //Serial.printf("DigitDisplay::loadAmbientAdjustConfig():  minLuxX100=%d, minBrightness_Hi=%d, maxLuxX100=%d, maxBrightness_Hi=%d\n", minLuxX100, minBrightness_Hi, maxLuxX100, maxBrightness_Hi);
}
void DigitDisplay::saveAmbientAdjustConfig() {
  //Serial.printf("DigitDisplay::saveAmbientAdjustConfig():  minLuxX100=%d, minBrightness_Hi=%d, maxLuxX100=%d, maxBrightness_Hi=%d\n", minLuxX100, minBrightness_Hi, maxLuxX100, maxBrightness_Hi);
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgAaNamespace, false);
  preferences.putULong(cfgAaLuxMin, minLuxX100);
  preferences.putUChar(cfgAaLedMin, minBrightness_Hi);
  preferences.putULong(cfgAaLuxMax, maxLuxX100);
  preferences.putUChar(cfgAaLedMax, maxBrightness_Hi);
  preferences.putUChar(cfgAaDayBoost, daylightBoost_Hi);
  preferences.end();
}
void DigitDisplay::clearAmbientAdjustConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgAaNamespace, false);
  preferences.clear();
  preferences.end();
}

void DigitDisplay::loadSundriesConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgSundriesNamespace, true);
  rgbCorrection = preferences.getULong(cfgSundriesRgbCorrection, rgbCorrection);
  gammaValue = preferences.getFloat(cfgSundriesGammaValue, gammaValue);
  preferences.end();
}
void DigitDisplay::saveSundriesConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgSundriesNamespace, false);
  preferences.putULong(cfgSundriesRgbCorrection, rgbCorrection);
  preferences.putFloat(cfgSundriesGammaValue, gammaValue);
  preferences.end();
}
void DigitDisplay::clearSundriesConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgSundriesNamespace, false);
  preferences.clear();
  preferences.end();
}

void DigitDisplay::setRgbCorrection(uint32_t value, bool bSave) {
  rgbCorrection = value;
  FastLED.setCorrection(rgbCorrection);
  if ( bSave ) {
    saveSundriesConfig();
  }
}

void DigitDisplay::setDoingAbnormalOperation(bool value) {
  if ( value ) {
    blankAllLeds();
    tempDisableAmbientAutoAdjust(true);
  } else {
    invalidateAllDigits();
    blankAllLeds();
    resetAnimationData();
    actualLedBrightness = 0;  // deeply hacky, this forces the change to happen (I should code this better)
    tempDisableAmbientAutoAdjust(false);
  }
}

void DigitDisplay::setSoakTestMode(bool value) {
  if ( bDoingSoakTest != value ) {
    bDoingSoakTest = value;
    setDoingAbnormalOperation(bDoingSoakTest);
    if ( bDoingSoakTest ) {
      initialiseColourSweep();
      // set all leds to be some non-zero value to indicate they should be swept
      for (uint16_t pixelIdx = 0; pixelIdx < QTY_LEDS; pixelIdx++) {
        ledBuffer[pixelIdx].setHSV(0, 0, 1);
      }
    }
  }
}

void DigitDisplay::setNycMode(bool value) {
  if ( bDoingNYC != value ) {
    bDoingNYC = value;
    setDoingAbnormalOperation(bDoingNYC);
    if ( bDoingNYC ) {
      initialiseColourSweep();
      // set leds appropriate for year to be some non-zero value to indicate they should be swept

      // get year to display
      uint16_t year = refProvider->timeSource.getYear();
      const LedDigitStyleStruct *digitData = ledDefsGetDigitStyle(true);

      for (uint8_t digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {
        // get this digit's value...
        uint8_t digitValue = year % 10;   // ...take the year's unit value...
        year /= 10;                       // ...then dividing the year by 10 repeatedly

        uint16_t startingDigitPixelIdx = digitIdx * QTY_LEDS_PER_DIGIT;
        for (uint16_t digitPixelOffset = 0; digitPixelOffset < QTY_LEDS_PER_DIGIT; digitPixelOffset++) {
          uint16_t pixelIdx = startingDigitPixelIdx + digitPixelOffset;                                       // absolute dest pixel index
          uint16_t pixelDefPair = digitData->digitDefs[digitValue][digitPixelOffset];                         // this is pair of pixel values, in MSB & LSB
          // the pixelDefPair contains encoded info regarding aninamtion...
          // ...but we actually don't care about the animation values, 
          // just whether the pixel has some non-zero value
          // which indicates that it forms part of the digit
          if ( pixelDefPair != 0 ) {
            ledBuffer[pixelIdx].setHSV(0, 0, 1);
          }
        }
      }
    }
  }
}

void DigitDisplay::setDoingGammaValueAdjust(bool value) {
  if ( bDoingGammaValueAdjust != value ) {
    bDoingGammaValueAdjust = value;
    setDoingAbnormalOperation(bDoingGammaValueAdjust);
    if ( bDoingGammaValueAdjust ) {
      // starting gamma value adjust
      FastLED.setBrightness(255);
      paintPixelsWithGammaGradient();
    }
  }
}

void DigitDisplay::setGammaValue(float value, bool bRepaint) {
  gammaValue = value;
  if ( bRepaint ) {
    paintPixelsWithGammaGradient();
  }
}
float DigitDisplay::getGammaValue() {
  return gammaValue;
}

void DigitDisplay::initialiseColourSweep() {
  colourSweepIdx = 0;
  colourSweepHueDir = 1;
  colourSweepHueVal = 0;
  colourSweepSatDir = 1;
  colourSweepSatVal = 0;
  colourSweepLumDir = 1;
  // Note, we avoid using a zero lum value as we are using
  // a non-zero value to indicate which leds are to be swept
  colourSweepLumVal = 1;
  FastLED.setBrightness(255);
}

void DigitDisplay::doColourSweep() {
  colourSweepIdx++;
  if ( colourSweepIdx % 13 ) {
    colourSweepSatVal += colourSweepSatDir;
    if ( colourSweepSatVal > 255 ) {
      colourSweepSatVal = 255;
      colourSweepSatDir = -colourSweepSatDir;
    } else if ( colourSweepSatVal < 0 ) {
      colourSweepSatVal = 0;
      colourSweepSatDir = -colourSweepSatDir;
    }
  }
  if ( colourSweepIdx % 17 ) {
    colourSweepLumVal += colourSweepLumDir;
    if ( colourSweepLumVal > 255 ) {
      colourSweepLumVal = 255;
      colourSweepLumDir = -colourSweepLumDir;
    } else if ( colourSweepLumVal < 1 ) {
      // Note, we avoid going down to zero lum as we are using
      // a non-zero value to indicate which leds are to be swept
      colourSweepLumVal = 1;
      colourSweepLumDir = -colourSweepLumDir;
    }
  }
  if ( colourSweepIdx % 23 ) {
    colourSweepHueVal += colourSweepHueDir;
    if ( colourSweepHueVal > 255 ) {
      colourSweepHueVal = 255;
      colourSweepHueDir = -colourSweepHueDir;
    } else if ( colourSweepHueVal < 0 ) {
      colourSweepHueVal = 0;
      colourSweepHueDir = -colourSweepHueDir;
    }
  }
  //Serial.printf("idx=%d, hue=%d, sat=%d, lum=%d\n", colourSweepIdx, colourSweepHueVal, colourSweepSatVal, colourSweepLumVal);
  // fill all non-zero pixels with identical varying hue/sat/lum
  for (uint16_t pixelIdx = 0; pixelIdx < QTY_LEDS; pixelIdx++) {
    if ( ledBuffer[pixelIdx] != 0 ) {
      ledBuffer[pixelIdx].setHSV(colourSweepHueVal, colourSweepSatVal, colourSweepLumVal);
    }
  }
}

void DigitDisplay::paintPixelsWithGammaGradient() {
  // hacky gamma adjust mode...
  // fill 101 pixels with (user brightness) 0-100 each passed through gammaCorrect()
  // using the currently set gammaValue
  for (uint16_t pixelIdx = 0; pixelIdx<101; pixelIdx++) {
    uint8_t gcVal = gammaCorrect(pixelIdx);
    ledBuffer[pixelIdx].setRGB(gcVal, gcVal, gcVal);
  }
}

//todo: need to call this on change? AND after ambientAdjust (but what if there no ambient adjust happens after the crossover to/from daylight?)
void DigitDisplay::checkDaylightBoost(bool bImmediate) {
  //Serial.println("DigitDisplay::checkDaylightBoost()");
  // is called by UiMenuSystemBrightness::menuItemOptionChange() when set/clear enable
  // also called when setDaylightBoost(uint8_t value) called
  ambientAdjustBrightness(bImmediate);
}

// "user brightness" deals with 1..100 brightness_Hi
uint8_t DigitDisplay::getUserBrightness() {
  return brightness_Hi;
}
void DigitDisplay::setUserBrightness(uint8_t value) {
//Serial.printf("setUserBrightness(%d): current brightness_Hi:%d\n", value, brightness_Hi);
  brightness_Hi = value;
  // note: when setting UserBrightness directly
  // we use the gamma-corrected BUT NOT ambient-light-adjusted value
  // (we might be trying to set up the upper/lower mappings of lux vs led-brightness)
  brightness_Hi_GC = gammaCorrect(brightness_Hi);
  setTargetLedBrightness(brightness_Hi_GC, true);
}

void DigitDisplay::setUserBrightnessBegin(UserBrightnessAdjustMode adjustMode) {
  if ( adjustMode == UserBrightnessAdjustMode::ubamMin ) {
    tempDisableAmbientAutoAdjust(true);
    setUserBrightness(minBrightness_Hi);
  } else if ( adjustMode == UserBrightnessAdjustMode::ubamMax ) {
    tempDisableAmbientAutoAdjust(true);
    setUserBrightness(maxBrightness_Hi);
  }// we do nothing special for ubamDaylightBoost
}
void DigitDisplay::setUserBrightnessEnd(UserBrightnessAdjustMode adjustMode) {
  if ( adjustMode == UserBrightnessAdjustMode::ubamMin ) {
    setMinBrightness_Hi(brightness_Hi);
    minLuxX100 = currentLuxX100;//refProvider->sundries.getLuxX100();
    tempDisableAmbientAutoAdjust(false);
  } else if ( adjustMode == UserBrightnessAdjustMode::ubamMax ) {
    setMaxBrightness_Hi(brightness_Hi);
    maxLuxX100 = currentLuxX100;//refProvider->sundries.getLuxX100();
    tempDisableAmbientAutoAdjust(false);
  }// we do nothing special for ubamDaylightBoost
}

void DigitDisplay::tempDisableAmbientAutoAdjust(bool bDisable) {
  bAmbientAutoAdjustTempDisabled = bDisable;
  // if re-enabling auto-adjust then recalc and show now
  if ( !bAmbientAutoAdjustTempDisabled ) {
    //Serial.printf("tempDisableAmbientAutoAdjust(%d): minLuxX100=%d, minBrightness_Hi=%d, minBrightness_Hi_GC=%d, maxLuxX100=%d, maxBrightness_Hi=%d, maxBrightness_Hi_GC=%d\n", bDisable, minLuxX100, minBrightness_Hi, minBrightness_Hi_GC, maxLuxX100, maxBrightness_Hi, maxBrightness_Hi_GC);
    ambientAdjustBrightness(true);
  }
}
void DigitDisplay::changedLuxX100(uint32_t newLuxX100, bool bSmoothing) {
  currentLuxX100 = newLuxX100;
  ambientAdjustBrightness( !bSmoothing );
}

void DigitDisplay::setTargetLedBrightness(uint8_t value, bool bImmediate) {
  targetLedBrightness = value;
  // if immediate - or if wifi is needing minimal interference, go directly to target 
  //  (and thus avoid repeated FastLED writes in the seek)
  if ( bImmediate || refProvider->sundries.getWifiPrioritised() ) {
    setActualLedBrightness(targetLedBrightness);
  }
}
void DigitDisplay::setActualLedBrightness(uint8_t value) {
  if ( actualLedBrightness != value ) {
    actualLedBrightness = value;
    //Serial.printf("DigitDisplay::setActualLedBrightness(%d)\n", value);
    FastLED.setBrightness(actualLedBrightness);
  }
}
void DigitDisplay::seekTargetLedBrightness() {
  if ( actualLedBrightness < targetLedBrightness  ) {
    setActualLedBrightness(actualLedBrightness + 1);
  } else if ( actualLedBrightness > targetLedBrightness ) {
    setActualLedBrightness(actualLedBrightness - 1);
  }
}

void DigitDisplay::setFlashReducePct(uint8_t value) {
  flashReducePct = value;
  if ( flashReducePct == 0 ) {
    bFlashing = false;
    // when finishing flashing, be sure to restore full brightness
    // we do it via this flag so that the switch to Hi brightness
    // can only happen _after_ the next update of a digit (if there 
    // is one about to be updated)
    bFlashFullNow = true;
    // and cancel any pending dimming
    flashPhaseChangeTimeMS = 0;
  } else {
    bFlashing = true;
    bFlashFullNow = false;
  }
}
bool DigitDisplay::getFlashing() {
  return bFlashing;
}

//TODO: possibly adjust mid-range with something like:
//https://www.desmos.com/calculator
//
//\frac{\cos\left(x\cdot\pi\cdot2\ -\ p\right)+1}{20}
//p=\pi

void DigitDisplay::calculateBrightnessValuesHi() {
  brightness_Hi = maxBrightness_Hi;
  brightness_Hi_GC = gammaCorrect(brightness_Hi);
  brightness_Hi_GC_AA = ambientAdjustValue(brightness_Hi_GC, minBrightness_Hi_GC, daylightBoost_Hi_GC);
  //Serial.printf("DigitDisplay::calculateBrightnessValuesHi(): maxBrightness_Hi=%d, brightness_Hi=%d, brightness_Hi_GC=%d, brightness_Hi_GC_AA=%d\n", maxBrightness_Hi, brightness_Hi, brightness_Hi_GC, brightness_Hi_GC_AA);
}

void DigitDisplay::calculateBrightnessValuesLo() {
  // calculate the reduced brightness value from the max-brightness_Hi *before* GC or ambient adjust
  // then do our own GC and ambient adjust for this Lo value
  brightness_Lo = (maxBrightness_Hi * (100L - flashReducePct)) / 100L;
  brightness_Lo_GC = gammaCorrect(brightness_Lo);

// why did I have this:
//daylightBoost_Lo = (maxBrightness_Hi * (100L - flashReducePct)) / 100L;
// when I think I ought to have this:
  daylightBoost_Lo = (daylightBoost_Hi * (100L - flashReducePct)) / 100L;
  daylightBoost_Lo_GC = (daylightBoost_Lo == 0) ? 0 : gammaCorrect(daylightBoost_Lo);
  brightness_Lo_GC_AA = ambientAdjustValue(brightness_Lo_GC, 1, daylightBoost_Lo_GC);
  //Serial.printf("DigitDisplay::calculateBrightnessValuesLo(): maxBrightness_Hi=%d, brightness_Lo=%d, brightness_Lo_GC=%d, brightness_Lo_GC_AA=%d, flashReducePct=%d\n", maxBrightness_Hi, brightness_Lo, brightness_Lo_GC, brightness_Lo_GC_AA, flashReducePct);
}

void DigitDisplay::setMinBrightness_Hi(uint8_t value) {
  minBrightness_Hi = value;
  minBrightness_Hi_GC = gammaCorrect(minBrightness_Hi);
}
void DigitDisplay::setMaxBrightness_Hi(uint8_t value) {
  maxBrightness_Hi = value;
  maxBrightness_Hi_GC = gammaCorrect(maxBrightness_Hi);
}

uint8_t DigitDisplay::getDaylightBoost() {
  return daylightBoost_Hi;
}
void DigitDisplay::setDaylightBoost(uint8_t value) {
  daylightBoost_Hi = value;
  daylightBoost_Hi_GC = (daylightBoost_Hi == 0) ? 0 : gammaCorrect(daylightBoost_Hi);
  // apply that change
  checkDaylightBoost(true);
}

uint8_t DigitDisplay::gammaCorrect(uint8_t inputBrightess) {
  uint8_t retVal = (uint8_t) (pow((float_t)inputBrightess / (float_t)100, gammaValue) * 253 + 2);
  //Serial.printf("inputBrightess=%d, outputBrightess=%d\n", inputBrightess, retVal);
  return retVal;
}


void DigitDisplay::ambientAdjustBrightness(bool bImmediate) {
  //Serial.print("DigitDisplay::ambientAdjustBrightness()");
  // only do stuff if ambient adjust is enabled
  if ( !bAmbientAutoAdjustTempDisabled && !refProvider->sundries.getWifiPrioritised() ) {
    // and only bother doing stuff if not currently flashing
    // (if we are flashing then the ambient adjusts happen on each flash phase change anyway)
    if ( !bFlashing && !bFlashFullNow ) {
      // (and, as we're not flashing, we only have to concern ourselves with the Hi brightness)
      calculateBrightnessValuesHi();
      //Serial.printf("DigitDisplay::ambientAdjustBrightness(): FastLED.setBrightness(%d)\n", brightness_Hi_GC_AA);
      setTargetLedBrightness(brightness_Hi_GC_AA, bImmediate);
    }
  }
}
uint8_t DigitDisplay::ambientAdjustValue(uint8_t brightnessGC, uint8_t minBrightnessGC, uint8_t daylightBoostValue) {
  uint8_t result = brightnessGC;
    
  // have current currentLuxX100  which is likely to be somewhere between:
  // have minLuxX100              & maxLuxX100
  // then want to adjust brightnessGC to be proportionally between:
  // gammaCorrect(minUserBrightness) & gammaCorrect(maxUserBrightness)

  // (maxLuxX100 - minLuxX100) gives minMaxLuxX100Range
  // (currentLuxX100 - minLuxX100) gives starting point (clamp>=0 && <=minMaxLuxRange) -> currentLuxX100Offset
  // (currentLuxX100Offset / minMaxLuxX100Range) gives fractional position within range 0..1 -> currentAmbientLightProportion
  // 2+253*currentAmbientLightProportion

  int32_t minMaxLuxX100Range = (maxLuxX100 - minLuxX100);
  if ( minMaxLuxX100Range < 1 ) {
    minMaxLuxX100Range = 1;
  }
  int32_t currentLuxX100Offset = (currentLuxX100 - minLuxX100);
  if ( currentLuxX100Offset < 0 ) {
    //Serial.printf("DigitDisplay::ambientAdjustValue(%d) ( currentLuxX100Offset < 0 ) setting currentLuxX100Offset to 0\n", brightnessGC);
    currentLuxX100Offset = 0;
  } else if ( currentLuxX100Offset > minMaxLuxX100Range ) {
    //Serial.printf("DigitDisplay::ambientAdjustValue(%d) ( currentLuxX100Offset > minMaxLuxX100Range ) setting currentLuxX100Offset to minMaxLuxX100Range\n", brightnessGC);
    currentLuxX100Offset = minMaxLuxX100Range;
  }

  float currentAmbientLightProportion = ((float)currentLuxX100Offset / (float)minMaxLuxX100Range);
  int16_t outputRange = brightnessGC - minBrightnessGC;
  int16_t result16 = currentAmbientLightProportion * outputRange + minBrightnessGC;
  //Serial.printf("ambientAdjustValue(%d), currentAmbientLightProportion=%f, minBrightnessGC=%d, outputRange=%d, result16=%d\n", brightnessGC, currentAmbientLightProportion, minBrightnessGC, outputRange, result16);

  // now apply any daylightBoost if applicable
  if ( refProvider->timeSource.inDaylight() ) {
//Serial.printf("DigitDisplay::ambientAdjustValue(%d, %d, %d): boosting %d to %d\n", brightnessGC, minBrightnessGC, daylightBoostValue, result, result+daylightBoostValue);    
    result16 += daylightBoostValue;
  }

  if ( result16 < minBrightnessGC ) {
    result16 = minBrightnessGC;
  } else if ( result16 > maxBrightness_Hi_GC ) {
//Serial.printf("DigitDisplay::ambientAdjustValue() result(%d) > maxBrightness_Hi_GC(%d)\n", result, maxBrightness_Hi_GC);    
    result16 = maxBrightness_Hi_GC;
  }
  result = result16;

  return result;
}

void DigitDisplay::previewColourData(ColourData *colourData) {
  previewingColourData = colourData;
  if ( previewingColourData == NULL ) {
    applySettingsAndDisplayNow();
  } else {
    calculateHueSat();
  }
}
void DigitDisplay::forceFullSat(bool bForce) {
  bForceFullSat = bForce;
  calculateHueSat();
}


int16_t DigitDisplay::getLerpTestVal() {
  return lerpTestVal;
}
void DigitDisplay::setLerpTestVal(int16_t val) {
  lerpTestVal = val;
  calculateHueSat();
}

uint16_t DigitDisplay::getHueSatLerp16() {
  uint16_t retVal = 0;
  TimeSource &timeSource = refProvider->timeSource;

  if ( lerpTestVal != -1 ) {
    retVal = (lerpTestVal * 65535L) / 100;
  } else {
    switch (activeDigitDisplayMode) {
    case DigitDisplayModeTime: {
      uint16_t minuteOfDay = timeSource.getHour() * 60 + timeSource.getMinute();
      const uint16_t minutesInDay = 24*60;
      retVal = (65535L * minuteOfDay) / minutesInDay;
      break;
    }
    case DigitDisplayModeDate: {
      uint16_t approxDayOffsetInYear = timeSource.getApproxDayOfYearOffset();
      const uint16_t approxDaysInYear = 366;
      retVal = (65535L * approxDayOffsetInYear) / approxDaysInYear;
      break;
    }
    case DigitDisplayModeTimer: {
      retVal = timeSource.getTimerProgress(65535);
      break;
    }
    case DigitDisplayModeEdit:
      retVal = 0;
      break;
    }
  }

  return retVal;
}


uint16_t DigitDisplay::getHue1() {
  return hue1;
}
void DigitDisplay::setHue1(uint16_t value) {
  if ( hue1 != value ) {
    hue1 = value;
    calculateHueSat();
  }
}
uint8_t DigitDisplay::getSat1() {
  return sat1;
}
void DigitDisplay::setSat1(uint8_t value) {
  if ( sat1 != value ) {
    sat1 = value;
    //Serial.printf("Sat1=%d\n", sat1);
    calculateHueSat();
  }
}

uint16_t DigitDisplay::getHue2() {
  return hue2;
}
void DigitDisplay::setHue2(uint16_t value) {
  if ( hue2 != value ) {
    hue2 = value;
    //Serial.printf("Sat2=%d\n", sat2);
    calculateHueSat();
  }
}
uint8_t DigitDisplay::getSat2() {
  return sat2;
}
void DigitDisplay::setSat2(uint8_t value) {
  if ( sat2 != value ) {
    sat2 = value;
    //Serial.printf("Sat2=%d\n", sat2);
    calculateHueSat();
  }
}

uint8_t DigitDisplay::getCurrentHue() {
  return currentHue;
}
uint8_t DigitDisplay::getCurrentSat() {
  return currentSat;
}

void DigitDisplay::calculateHueSat() {
  uint8_t oldCurrentHue = currentHue;
  uint8_t oldCurrentSat = currentSat;
  if ( previewingColourData != NULL ) {
    currentHue = previewingColourData->hue;
    currentSat = bForceFullSat ? 255 : previewingColourData->sat;
  } else {
    uint16_t lerpValue16 = getHueSatLerp16();
    currentHue = lerp16by16(hue1, hue2, lerpValue16) & 0xff;
    currentSat = lerp16by16(sat1, sat2, lerpValue16) & 0xff;
  }
  if ( (currentHue != oldCurrentHue) || (currentSat != oldCurrentSat) ) {
    //Serial.printf(": hue/sat change: currentHue[%d], currentSat[%d]", currentHue, currentSat);
    bHueSatChanging = true;
    bApplyHueSatChange = false;
  }
}


bool DigitDisplay::getDigitStyleSeriffed() {
  return digitStyleSeriffed;
}
void DigitDisplay::setDigitStyleSeriffed(bool value) {
  if ( digitStyleSeriffed != value ) {
    digitStyleSeriffed = value;
    bDigitStyleChanged = true;
  }
}

bool DigitDisplay::getDigitStyleSoftened() {
  return digitStyleSoftened;
}
void DigitDisplay::setDigitStyleSoftened(bool value) {
  if ( digitStyleSoftened != value ) {
    digitStyleSoftened = value;

    dimmedLumVal = digitStyleSoftened ? DIMMED_PIXEL_LUMVAL : normalLumVal;
    bDigitStyleChanged = true;
  }
}


void DigitDisplay::setTempDisableFx(bool value) {
  bTempDisableFx = value;
}



DigitDisplayMode DigitDisplay::getActiveDigitDisplayMode() {
  return activeDigitDisplayMode;
}
bool DigitDisplay::setActiveDigitDisplayMode(DigitDisplayMode newMode, bool bDisplayNow) {
  bool retVal = false;

  if ( newMode != activeDigitDisplayMode ) {
    retVal = true;
    //Serial.printf("setActiveDigitDisplayMode(%d), was: %d\n", newMode, activeDigitDisplayMode);
    activeDigitDisplayMode = newMode;

    // each mode carries its own preferred digit style settings etc...
    if ( bDisplayNow ) {
      //Serial.println("DigitDisplay::getActiveDigitDisplayMode() - applySettingsAndDisplayNow();");
      applySettingsAndDisplayNow();
    } else {
      applyActiveDigitDisplayModeSettings();
    }
  }

  return retVal;
}

bool DigitDisplay::autoSelectDigitDisplayMode() {
  bool retVal = false;
  // only do anything if not in an edit mode (because: reasons)
  if (activeDigitDisplayMode != DigitDisplayModeEdit) {
    if ( refProvider->timeSource.getSelectedTimer() == -1 ) {
      retVal = setActiveDigitDisplayMode(DigitDisplayModeTime, true);
    } else {
      retVal = setActiveDigitDisplayMode(DigitDisplayModeTimer, true);
    }
  }
  return retVal;
}

void DigitDisplay::cycleDigitDisplayMode(int8_t direction) {
  // ignore this if we are in soaktest or nyc mode
  if ( !bDoingSoakTest && !bDoingNYC ) {
    DigitDisplayMode newMode = activeDigitDisplayMode;
    if ( direction < 0 ) {
      switch (activeDigitDisplayMode) {
      case DigitDisplayModeTime:
        newMode = (refProvider->timeSource.getSelectedTimer() == -1) ? DigitDisplayModeDate : DigitDisplayModeTimer;
        break;
      case DigitDisplayModeDate:
        newMode = DigitDisplayModeTime;
        break;
      case DigitDisplayModeTimer:
        newMode = DigitDisplayModeDate;
        break;
      }
    } else if ( direction > 0 ) {
      switch (activeDigitDisplayMode) {
      case DigitDisplayModeTime:
        newMode = DigitDisplayModeDate;
        break;
      case DigitDisplayModeDate:
        newMode = (refProvider->timeSource.getSelectedTimer() == -1) ? DigitDisplayModeTime : DigitDisplayModeTimer;
        break;
      case DigitDisplayModeTimer:
        newMode = DigitDisplayModeTime;
        break;
      }
    }
    setActiveDigitDisplayMode(newMode, true);
  }//endif not in soaktest/nyc mode
}


void DigitDisplay::startEditMode() {
  // copy the digit style (only) values from the current mode to the edit mode data
  if ( activeDigitDisplayMode != DigitDisplayModeNone ) {
    digitDisplayModeDataList[DigitDisplayModeEdit].seriffed = digitDisplayModeDataList[activeDigitDisplayMode].seriffed;
    digitDisplayModeDataList[DigitDisplayModeEdit].softened = digitDisplayModeDataList[activeDigitDisplayMode].softened;
  }

  // save the current mode so we can return to it later
  priorToEditDigitDisplayMode = activeDigitDisplayMode;

  setActiveDigitDisplayMode(DigitDisplayModeEdit, true);
}

void DigitDisplay::stopEditMode() {
  if ( priorToEditDigitDisplayMode != DigitDisplayModeNone ) {
    // revert the mode to as before we started edit mode
    setActiveDigitDisplayMode(priorToEditDigitDisplayMode, false);
  }
}

void DigitDisplay::dump(uint8_t digitIdx) {
  Serial.print("\n\n\n");
  DigitAnimationDataStruct &dad = digitAnimationData[digitIdx];
  Serial.printf("digit[%d]:\tphase=%d,\tfrom=%d,\tto=%d,\tqtySteps=%d,\tstepNo=%d\n\n", digitIdx, dad.digitAnimationPhase, dad.digitValueFrom, dad.digitValueTo, dad.qtySteps, dad.stepNo);

  uint16_t startingDigitPixelIdx = digitIdx * QTY_LEDS_PER_DIGIT;

  for (uint16_t digitPixelOffset = 0; digitPixelOffset < QTY_LEDS_PER_DIGIT; digitPixelOffset++) {
    uint16_t pixelIdx = startingDigitPixelIdx + digitPixelOffset;
    PixelAnimationDataStruct &pad = pixelAnimationData[pixelIdx];

    Serial.printf("pad[%d]: active=%d, phase=%d, lumStepValue=%d, lumValueCurrent=%d, lumValueTarget=%d\n", pixelIdx, pad.bActive, pad.pixelAnimationPhase, pad.lumStepValue, pad.lumValueCurrent, pad.lumValueTarget);

  }
  Serial.print("\n\n\n");
}

DigitDisplayModeData *DigitDisplay::getDigitDisplayModeData(DigitDisplayMode digitDisplayMode) {
  return &digitDisplayModeDataList[digitDisplayMode];
}

void DigitDisplay::applySettingsAndDisplayNow() {
  //Serial.print("applySettingsAndDisplayNow()");
  applyActiveDigitDisplayModeSettings();
  maybeUpdateAllDigits(false);
  doDigitAnimations(false);
  doPixelAnimations(false);
}

void DigitDisplay::previewEffects(bool value) {
  bEffectsPreview = value;
  if ( bEffectsPreview ) {
    fxDigits[0] = 0;
    fxDigits[1] = 0;
    fxDigits[2] = 0;
    fxDigits[3] = 0;
  }
}

void DigitDisplay::applyActiveDigitDisplayModeSettings() {
  // each mode carries its own preferred style settings, colours and fx settings etc...
  DigitDisplayModeData digitDisplayModeData = digitDisplayModeDataList[activeDigitDisplayMode];
  setDigitStyleSeriffed(digitDisplayModeData.seriffed);
  setDigitStyleSoftened(digitDisplayModeData.softened);

  setHue1(digitDisplayModeData.colourData1.hue);
  setSat1(digitDisplayModeData.colourData1.sat);
  setHue2(digitDisplayModeData.colourData2.hue);
  setSat2(digitDisplayModeData.colourData2.sat);
  setFlashReducePct(digitDisplayModeData.flashReducePct);

  digitAnimationFxStroke = digitDisplayModeData.fxEnabled && digitDisplayModeData.fxStroke;
  digitAnimationFxUnstroke = digitDisplayModeData.fxEnabled && digitDisplayModeData.fxUnstroke;
  digitAnimationFxGrow = digitDisplayModeData.fxEnabled && digitDisplayModeData.fxGrow;
  digitAnimationFxShrink = digitDisplayModeData.fxEnabled && digitDisplayModeData.fxShrink;
  digitAnimationFxBuildNew = digitAnimationFxGrow || digitAnimationFxStroke;
  digitAnimationFxDecayOld = digitAnimationFxShrink || digitAnimationFxUnstroke;

  pixelAnimationFadeIn = digitDisplayModeData.fxEnabled && digitDisplayModeData.fxFadeIn;
  pixelAnimationFadeOut = digitDisplayModeData.fxEnabled && digitDisplayModeData.fxFadeOut;

  uint8_t pixelAnimStepsFadeInOut;
  // tweak fadein/fadeout steps depending on digit animation selections
  if (digitAnimationFxBuildNew && digitAnimationFxDecayOld) {
    pixelAnimStepsFadeInOut = 16;
  } else if (digitAnimationFxBuildNew) {
    pixelAnimStepsFadeInOut = 24;
  } else if (digitAnimationFxDecayOld) {
    pixelAnimStepsFadeInOut = 24;
  } else {
    pixelAnimStepsFadeInOut = 32;
  }

  // I think I must've done the following thinking that the fade-out/in
  // happened sequentially but I now think they happen simultaneously
  // so this halving does not make sense, so I've nopped it out for now
  // if ( pixelAnimationFadeIn && pixelAnimationFadeOut ) {
  //   pixelAnimStepsFadeInOut = pixelAnimStepsFadeInOut / 2;
  // }
  pixelAnimStepsFadeIn = pixelAnimStepsFadeInOut;
  pixelAnimStepsFadeOut = pixelAnimStepsFadeInOut;

  calculateHueSat();
}

// ********************************************************************************************************************************
// main loop
// ********************************************************************************************************************************
void DigitDisplay::loop() {
  if ( updatesEnabled ) {
    if ( bDoingSoakTest || bDoingNYC ) {
      doColourSweep();
    } else if ( !bDoingGammaValueAdjust ) {
      doTickingUpdate();

      doDigitAnimations(!bFlashing);   // if flashing then immediate draw (no animations)
      doPixelAnimations(!bFlashing);   // if flashing then immediate draw (no animations)

      seekTargetLedBrightness();
    }
  }
}
// ********************************************************************************************************************************

void DigitDisplay::pauseUpdates() {
  updatesEnabled = false;
}
void DigitDisplay::allowUpdates() {
  updatesEnabled = true;
  calculateHueSat();
  maybeUpdateAllDigits(false);
}

void DigitDisplay::invalidateAllDigits() {
  //Serial.println("invalidateAllDigits()");
  for (uint8_t digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {  
    digitValues[digitIdx] = invalidDigitValue;
  }
}


void DigitDisplay::blankAllLeds() {
  //Serial.println("DigitDisplay::blankAllLeds()");
  for (uint16_t pixelIdx = 0; pixelIdx<QTY_LEDS; pixelIdx++) {
    ledBuffer[pixelIdx] = CRGB::Black;
  }
}

void DigitDisplay::resetAnimationData() {
  for (uint8_t digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {  
    digitAnimationData[digitIdx].digitAnimationPhase = dapIdle;
    digitAnimationData[digitIdx].digitValueFrom = blankedDigitValue;
    digitAnimationData[digitIdx].digitValueTo = blankedDigitValue;
    digitAnimationData[digitIdx].qtySteps = 0;
    digitAnimationData[digitIdx].stepNo = 0;
    digitAnimationData[digitIdx].qtyActivePixelAnimations = 0;
    digitAnimationData[digitIdx].pixelAnimationPending = false;
  }
  
  for (uint16_t pixelIdx = 0; pixelIdx<QTY_LEDS; pixelIdx++) {
    pixelAnimationData[pixelIdx].pixelAnimationPhase = papIdle;
    pixelAnimationData[pixelIdx].lumValueCurrent = 0;
    pixelAnimationData[pixelIdx].lumValueTarget = 0;
    pixelAnimationData[pixelIdx].lumStepValue = 0;
    pixelAnimationData[pixelIdx].bActive = false;
  }
}

void DigitDisplay::doTickingUpdate() {

  // deal with flashing
  uint32_t nowMS = millis();
  TimeSource &timeSource = refProvider->timeSource;

  // try get flash phase aligning with the second boundary
  // (things can look odd otherwise, especially on timer expiry)
  bool bNewSecond = false;
  if ( timeSource.timeValid() ) {
    // get the year so we can notice year change
    if ( nycCurrentYear == 0 ) {
      nycCurrentYear = timeSource.getYear();
    }

    uint8_t secsNow = timeSource.getSecondUnits();
    if ( secsNow != previousSecondValue ) {
      bNewSecond = true;
      //Serial.printf("qtyActivePixelAnimations=%d\n", qtyActivePixelAnimations);
      // if ( qtyActivePixelAnimations != 0 ) {
      //   refProvider->sundries.buzzer_start();
      // } else {
      //   refProvider->sundries.buzzer_stop();
      // }
      previousSecondValue = secsNow;
      // align flash phase with this second boundary
      if ( bFlashing ) {
        // we change the brightness after 500ms, so as to switch to the
        // lo-flash setting halfway through the current second (and will
        // go back to full brightness at the start of the next second 
        // (which will be done _after_ a digit paint, if we do paint one))
        flashPhaseChangeTimeMS = nowMS + flashPhaseDurationMS;
        bFlashFullNow = true;
      }
    }
  }
  if ( bFlashing && (flashPhaseChangeTimeMS != 0) && (nowMS > flashPhaseChangeTimeMS) ) {
    // time to go to low brightness for flashing
    // (Note: switching from low to high is achieved via the bFlashFullNow flag
    //        which is checked in the doPixelAnimations() func _after_ any new
    //        digit has been painted. It is done this way so we only go back to
    //        flashing full brightness _after_ a digit has been updated, otherwise
    //        it can look crap (digit switches to being more visible - and then 
    //        maybe a fraction of a second later the digit changes)).
    // calculate the reduced brightness value(s)
    calculateBrightnessValuesLo();
    setTargetLedBrightness(brightness_Lo_GC_AA, true);
    flashPhaseChangeTimeMS = 0;
  }

  // potentially update digits...
  maybeUpdateAllDigits(bNewSecond);

  // if year has changed then initiate nyc
  if ( bNewSecond ) {
    uint16_t newYear = timeSource.getYear();
    if ( newYear != nycCurrentYear ) {
      nycCurrentYear = newYear;
      setNycMode(true);
    }
  }

}

void DigitDisplay::maybeUpdateAllDigits(bool bNewSecond) {
  //  Serial.println("DigitDisplay::maybeUpdateAllDigits()...about to debugPrintAllLeds()");
  TimeSource &timeSource = refProvider->timeSource;

  uint8_t newDigitValues[QTY_DIGITS];
  uint8_t digitIdx;
  for (digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {
    newDigitValues[digitIdx] = 8;    // all 8s to indicate invalid time (or whatever) value
  }
  digitIdx = QTY_DIGITS;

  if ( bEffectsPreview ) {
    if ( bNewSecond ) {
      fxDigits[0] = (fxDigits[0] + 1) % 10;
      if ( fxDigits[0] == 0 ) {
        fxDigits[1] = (fxDigits[1] + 1) % 10;
      }
      fxDigits[2] = (fxDigits[2] + 9) % 10;
      if ( fxDigits[2] == 9 ) {
        fxDigits[3] = (fxDigits[3] + 9) % 10;
      }
    }
    newDigitValues[--digitIdx] = fxDigits[3];
    newDigitValues[--digitIdx] = fxDigits[2];
    newDigitValues[--digitIdx] = fxDigits[1];
    newDigitValues[--digitIdx] = fxDigits[0];
  } else {

    switch (activeDigitDisplayMode) {
    case DigitDisplayModeTime:
      if ( timeSource.timeValid() ) {
        //Serial.printf("Time deemed valid: M%d, m%d : S%d, s%d\n", timeSource.getMinuteTens(), timeSource.getMinuteUnits(), timeSource.getSecondTens(), timeSource.getSecondUnits());
        // if doing effects preview then show MMSS else (normal) show HHMM
        if ( !bEffectsPreview ) {
          newDigitValues[--digitIdx] = timeSource.getHourTens();
          newDigitValues[--digitIdx] = timeSource.getHourUnits();
        }
        newDigitValues[--digitIdx] = timeSource.getMinuteTens();
        newDigitValues[--digitIdx] = timeSource.getMinuteUnits();
        if ( bEffectsPreview ) {
          newDigitValues[--digitIdx] = timeSource.getSecondTens();
          newDigitValues[--digitIdx] = timeSource.getSecondUnits();
        }
      } else {
        //Serial.println("Time deemed invalid");
      }
      break;

    case DigitDisplayModeDate:
      if ( timeSource.timeValid() ) {
        if ( QTY_DIGITS == 6 ) {
          newDigitValues[--digitIdx] = timeSource.getYearTens();
          newDigitValues[--digitIdx] = timeSource.getYearUnits();
        }
        newDigitValues[--digitIdx] = timeSource.getMonthTens();
        newDigitValues[--digitIdx] = timeSource.getMonthUnits();
        newDigitValues[--digitIdx] = timeSource.getDayTens();
        newDigitValues[--digitIdx] = timeSource.getDayUnits();
      }
      break;

    case DigitDisplayModeTimer:
      uint8_t hh;
      uint8_t mm;
      uint8_t ss;
      timeSource.getTimerDigits(&hh, &mm, &ss);
      if ( timeSource.selectedTimerRunning() || refProvider->timeSource.selectedTimerExpired() ) {
        // either running or expired, allow HHMM or MMSS
        if ( hh > 0 ) {
          newDigitValues[--digitIdx] = hh /10;
          newDigitValues[--digitIdx] = hh %10;
        }
        newDigitValues[--digitIdx] = mm /10;
        newDigitValues[--digitIdx] = mm %10;
        if ( hh == 0 ) {
          newDigitValues[--digitIdx] = ss /10;
          newDigitValues[--digitIdx] = ss %10;
        }
      } else {
        // timer neither running nor expired, must be just selected
        // always force HHMM to match what we allow for editing
        newDigitValues[--digitIdx] = hh /10;
        newDigitValues[--digitIdx] = hh %10;
        newDigitValues[--digitIdx] = mm /10;
        newDigitValues[--digitIdx] = mm %10;
      }
      break;

    case DigitDisplayModeEdit:
      uint8_t hi;
      uint8_t lo;
      timeSource.getEditDigits(&hi, &lo);
      newDigitValues[--digitIdx] = hi /10;
      newDigitValues[--digitIdx] = hi %10;
      newDigitValues[--digitIdx] = lo /10;
      newDigitValues[--digitIdx] = lo %10;
      break;

    }//end switch mode
  }//endif doing/not-doing effects preview

  // if any digit at all is changing then recalculate hue/sat lerping
  // which might force an update for all digits
  bool bUpdateRequired = false;
  for (digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {
    bUpdateRequired = (newDigitValues[digitIdx] != digitValues[digitIdx]) || bUpdateRequired;
  }
  if ( bUpdateRequired ) {
    calculateHueSat();
  }

  if ( bUpdateRequired || bDigitStyleChanged ) {
    // now do all the digit updates required
    for (digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {
      maybeUpdateThisDigit(digitIdx, newDigitValues[digitIdx]);
    }

    //    FastLED.show();
  }

}

void DigitDisplay::blankThisDigit(uint8_t digitIdx) {
  setThisDigitWithThisDigitValue(digitIdx, blankedDigitValue);
}

void DigitDisplay::setThisDigitWithThisDigitValue(uint8_t digitIdx, uint8_t digitValue) {
  updateThisDigitWithThisDigitValue(digitIdx, digitValue);

  FastLED.show();
}

bool DigitDisplay::maybeUpdateThisDigit(uint8_t digitIdx, uint8_t digitValue) {
  bool bDigitValueChanged = false;
  if ( digitIdx < QTY_DIGITS) {
    // decide whether the digit is changing value 
    // (this includes being different from the invalidDigitValue or the blankedDigitValue)
    bDigitValueChanged = (digitValues[digitIdx] != digitValue);
    if ( bDigitValueChanged || bDigitStyleChanged ) {
      //Serial.printf("DigitDisplay::maybeUpdateThisDigit(%d, %d); digitValues[%d]=%d, qtyActivePixelAnimations=%d\n", digitIdx, digitValue, digitIdx, digitValues[digitIdx], qtyActivePixelAnimations);
      updateThisDigitWithThisDigitValue(digitIdx, digitValue);
    }
  }
  return bDigitValueChanged;
}
void DigitDisplay::updateThisDigitWithThisDigitValue(uint8_t digitIdx, uint8_t digitValue) {

  if ( (digitIdx < QTY_DIGITS) && ( (digitValue <= 9) || (digitValue == blankedDigitValue) ) ) {
    //TODO: do we need to finalise any existing animation for this digit?
    //Serial.printf("updateThisDigitWithThisDigitValue(%d, %d)\n", digitIdx, digitValue);
    digitAnimationData[digitIdx].digitAnimationPhase = dapStart;
    digitAnimationData[digitIdx].digitValueFrom = digitValues[digitIdx];
    digitAnimationData[digitIdx].digitValueTo = digitValue;

    // say we have changed values immediately, to stop other code thinking we still need to update this digit
    digitValues[digitIdx] = digitValue;

  }

}

void DigitDisplay::doDigitAnimations(bool bAllowAnimation) {
  if ( !bAllowAnimation || bTempDisableFx ) {
    cancelPixelAnimations();
  }
  for (uint8_t digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {  
    doDigitAnimation(bAllowAnimation, digitIdx);
  }
}
void DigitDisplay::doDigitAnimation(bool bAllowAnimation, uint8_t digitIdx) {

  // get the digits def for current style
  const LedDigitStyleStruct *digitData = ledDefsGetDigitStyle(digitStyleSeriffed);

  uint8_t maxDigitAnimationBuildSteps = digitAnimationFxGrow ? digitData->maxStepsGrowShrink : digitAnimationFxStroke ? digitData->maxStepsStrokeUnstroke : 0;
  uint8_t maxDigitAnimationDecaySteps = digitAnimationFxShrink ? digitData->maxStepsGrowShrink : digitAnimationFxUnstroke ? digitData->maxStepsStrokeUnstroke : 0;

  DigitAnimationDataStruct &dad = digitAnimationData[digitIdx];

  if ( dad.digitAnimationPhase == dapIdle ) {
    // digit animation idle, trigger the application of any pending hueSat change
    maybeTriggerHueSatChangeApplication();
  } else {
    // digit animation not idle
    if ( (dad.digitAnimationPhase == dapStart) && (dad.pixelAnimationPending == false) && ( qtyActivePixelAnimations == 0 ) ) {
//todo: tidy comments
      // avoid all new digit animations if:
      // - we are flashing (else it looks ugly)
      // - or if neither decayOld nor buildNew wanted
      if ( !bAllowAnimation || bTempDisableFx || ( (digitAnimationFxDecayOld == false) && (digitAnimationFxBuildNew == false) ) ) {

        // jump straight to painting the new digit over the old 
        // set up the buildNew animation vars (it'll know not to animate)
        dad.digitAnimationPhase = dapStartBuildNew;
      } else {
        // if decay old not wanted (and we know buildNew must be wanted) then
        // we need to instantly blank the whole digit before the buildNew
        if ( digitAnimationFxDecayOld == false ) {
          //Serial.printf("digit[%d] paintDigitPixels(digitData, digitIdx, blankedDigitValue, true, 0, false);\n", digitIdx);
          paintDigitPixels(digitData, digitIdx, blankedDigitValue, true, 0, false);
          // set up the buildNew animation vars
          dad.digitAnimationPhase = dapStartBuildNew;
        } else {
          // we do want decayOld, set up vars for that
          // unless there's nothing to erase
          if (dad.digitValueFrom <= 9) {
            dad.qtySteps = maxDigitAnimationDecaySteps;
            dad.stepNo = maxDigitAnimationDecaySteps;
            dad.digitAnimationPhase = dapDecayOld;
          } else {
            // nothing to erase, set up the buildNew animation vars
            dad.digitAnimationPhase = dapStartBuildNew;
          }
        }//endif don't/do want decayOld
      }//endif/else just overwrite existing digit with new one
    }//endif dapStart

    if ( dad.digitAnimationPhase == dapDecayOld ) {
      paintDigitPixels(digitData, digitIdx, dad.digitValueFrom, true, dad.stepNo, digitAnimationFxShrink);
      if ( dad.stepNo > 0) {
        // more to do
        dad.stepNo--;
      } else {
        // done all decayOld
        // set up the dapBuildNew animation vars (even if we don't need them)
        dad.digitAnimationPhase = dapStartBuildNew;
      }
    }//endif doing decayOld

    if ( dad.digitAnimationPhase == dapStartBuildNew ) {
      // as we're now building the new digit, trigger the application of any pending hueSat change
      maybeTriggerHueSatChangeApplication();
      if ( (qtyActivePixelAnimations == 0) && (bPixelAnimationPending == false) ) {
        dad.qtySteps = maxDigitAnimationBuildSteps;
        dad.stepNo = 0;
        dad.digitAnimationPhase = dapBuildNew;
      }
    }

    if ( dad.digitAnimationPhase == dapBuildNew ) {
      if ( !bAllowAnimation || bTempDisableFx || (digitAnimationFxBuildNew == false) ) {
        // no grow, force an instant paint of all pixels in digit
        dad.stepNo = 0;
      } else {
        dad.stepNo++;
      }
      //Serial.printf("digit[%d] paintDigitPixels(digitData, digitIdx, %d, %d, %d, %d);\n", digitIdx, dad.digitValueTo, (dad.digitValueTo > 9), dad.stepNo, digitAnimationFxGrow);
      paintDigitPixels(digitData, digitIdx, dad.digitValueTo, (dad.digitValueTo > 9), dad.stepNo, digitAnimationFxGrow);
      if ( (dad.stepNo == 0) || (dad.stepNo == dad.qtySteps) ) {
        // no need of, or have finished, buildNew
        // switch to idle
        dad.digitAnimationPhase = dapIdle;
      }

    }//endif doing buildNew

  }//endif not idle

}

void DigitDisplay::cancelDigitAnimations() {
Serial.println("DigitDisplay::cancelDigitAnimations()");
}

void DigitDisplay::paintDigitPixels(const LedDigitStyleStruct *digitData, uint8_t digitIdx, uint8_t digitValue, bool bBlanking, uint8_t matchStepNo, bool growShrinkDigitFx) {

  // get the starting dest led index
  uint16_t startingDigitPixelIdx = digitIdx * QTY_LEDS_PER_DIGIT;

  for (uint16_t digitPixelOffset = 0; digitPixelOffset < QTY_LEDS_PER_DIGIT; digitPixelOffset++) {
    uint16_t pixelIdx = startingDigitPixelIdx + digitPixelOffset;                                       // absolute dest pixel index
    uint8_t newPixelLumCode = 0;
    uint8_t newPixelLumVal = 0;
    if (digitValue <= 9) {
      uint16_t pixelDefPair = digitData->digitDefs[digitValue][digitPixelOffset];                        // this is pair of pixel values, in MSB & LSB
      // grow/shrink animation info is stored in the MSB
      if ( growShrinkDigitFx ) {
        pixelDefPair = pixelDefPair >> 8;
      }
      newPixelLumCode = pixelDefPair & 0x007f;
      if ( !bBlanking && (newPixelLumCode != 0) ) {
        bool bDimmed = pixelDefPair & 0x0080;
        newPixelLumVal = bDimmed ? dimmedLumVal : normalLumVal;
      }
    }
    if ( (matchStepNo == 0) || (newPixelLumCode == matchStepNo) ) {
      // this digit-pixel coincides with the current stepNo (or we're doing all of them anyway),
      // paint this pixel with either blank or the dest lum value (with or without pixel animation)
      // kick off pixel animation:
      //  if the new wanted value is different to the pixel's target value
      //  or if the new wanted value is different to the pixel's current value and the pixelAnimation is idle (hoping this is never the case)
      PixelAnimationDataStruct &pad = pixelAnimationData[pixelIdx];
      if ( (newPixelLumVal != pad.lumValueTarget) 
        || ( (newPixelLumVal != pad.lumValueCurrent) && (pad.pixelAnimationPhase == papIdle) ) ) {
          pad.lumValueTarget = newPixelLumVal;
          pad.pixelAnimationPhase = papStart;
          digitAnimationData[digitIdx].pixelAnimationPending = true;
      }//endif need kick off pixel animation
    }//endif we match the stepNo (or don't care what stepNo)
  }//next pixel within digit

  // seems to work without this, not sure why I wanted it
  // // force an iteration of doPixelAnimations() immediately, so as to force qtyActivePixelAnimations to go non-zero
  // doPixelAnimations(bAllowAnimation);
  
}


void DigitDisplay::doPixelAnimations(bool bAllowAnimation) {

  bool bDidUpdate = false;
  if ( !bAllowAnimation || bTempDisableFx ) {
    // not doing pixel animations (for one reason or another)
    for (uint16_t pixelIdx = 0; pixelIdx < QTY_LEDS; pixelIdx++) {
      PixelAnimationDataStruct &pad = pixelAnimationData[pixelIdx];
      pad.lumValueCurrent = pad.lumValueTarget;
      pad.bActive = false;
      pad.pixelAnimationPhase = papIdle;
      ledBuffer[pixelIdx].setHSV(currentHue, currentSat, pad.lumValueCurrent);
    }
    bDidUpdate = true;
  } else {
    // are doing pixel animations
    for (uint16_t pixelIdx = 0; pixelIdx < QTY_LEDS; pixelIdx++) {
      PixelAnimationDataStruct &pad = pixelAnimationData[pixelIdx];
      switch (pad.pixelAnimationPhase) {
      case papIdle: {
        if ( bApplyHueSatChange ) {
          ledBuffer[pixelIdx].setHSV(currentHue, currentSat, pad.lumValueCurrent);
          bDidUpdate = true;
        }
        break;
      }
      case papStart: {
        // depending on whether we are reducing or increasing lum
        // and whether we have pixel fx fadeIn or faceOut we either
        // jump directly to the target value or set up for stepping there
        int16_t lumDiff = pad.lumValueTarget - pad.lumValueCurrent;
        if ( lumDiff == 0 ) {
          // no diff in lum... but, if hueSat has changed we still need to update the pixel
          if ( bApplyHueSatChange ) {
            ledBuffer[pixelIdx].setHSV(currentHue, currentSat, pad.lumValueCurrent);
          }
          if (pad.bActive) {
            qtyActivePixelAnimations--;
            pad.bActive = false;
          }
          pad.pixelAnimationPhase = papIdle;
          break;
        } else {
//todo: can we ever get here with !bAllowAnimation || bTempDisableFx... isn't that trapped at the top?
          if ( !bAllowAnimation || bTempDisableFx
          ||   ((lumDiff > 0) && (pixelAnimationFadeIn == false))
          ||   ((lumDiff < 0) && (pixelAnimationFadeOut == false)) ) {
            pad.lumStepValue = lumDiff;   // step once, directly to target
          } else {
            // step in smaller increments to effect fade in/out
            if ( lumDiff > 0 ) {
              pad.lumStepValue = lumDiff / pixelAnimStepsFadeIn;
            } else {
              pad.lumStepValue = lumDiff / pixelAnimStepsFadeOut;
            }
            if ( pad.lumStepValue == 0 ) {
              // hmm.. the diff must be smaller than the number of steps we're trying to do...
              // just make the lumStepValue the remaining difference so we finish off immediately
              pad.lumStepValue = lumDiff;
            }
          }
          // step to desired value
          if ( !pad.bActive ) {
            qtyActivePixelAnimations++;
            pad.bActive = true;
          }
          pad.pixelAnimationPhase = papDoing;
        }
      }
      // fall through into papDoing

      case papDoing: {
        int16_t newLumVal = pad.lumValueCurrent + pad.lumStepValue;
        if ( (pad.lumStepValue < 0) && (newLumVal <= pad.lumValueTarget) ) {
          newLumVal = pad.lumValueTarget;
          pad.pixelAnimationPhase = papDone;
        } else if ( (pad.lumStepValue > 0) && (newLumVal >= pad.lumValueTarget) ) {
          newLumVal = pad.lumValueTarget;
          pad.pixelAnimationPhase = papDone;
        }
        pad.lumValueCurrent = newLumVal;
        ledBuffer[pixelIdx].setHSV(currentHue, currentSat, pad.lumValueCurrent);
        bDidUpdate = true;

        if ( pad.pixelAnimationPhase == papDoing ) {
          break;
        }
        // must be papDone, fall through
      }
      case papDone: {
        pad.pixelAnimationPhase = papIdle;
        if (pad.bActive) {
          qtyActivePixelAnimations--;
          pad.bActive = false;
        }
        break;
      }
      }
    }
  }//endif else bAllowAnimation

  // forget that we needed to repaint this (otherwise possibly unchanging) digit for either of these reasons
  bDigitStyleChanged = false;
  bApplyHueSatChange = false;

  if ( bFlashFullNow ) {
    bFlashFullNow = false;
    // recalculate the Hi brightness value(s) for the current ambient light value
    calculateBrightnessValuesHi();
    setTargetLedBrightness(brightness_Hi_GC_AA, true);
  }
  if ( bDidUpdate ) {
//huh... if the following line is enabled then, when timerFX= stroke + unstroke + fadeOut
// then the digits aren't fully drawn. Is ok if fadeOut=false
// reconfirmed to be true on 2023-12-07
//
// 2024-02-03, I have halved the fade pixel step if we are changing digits each second
// and this fixes the partial digit draw as mentioned above
// but then I noticed that having this FastLED.show() here causes issues
// with flickering (particularly in the softened corners), so is better without anyway
//FastLED.show();    
  }

  for (uint8_t digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {  
    digitAnimationData[digitIdx].pixelAnimationPending = false;
  }

}

void DigitDisplay::cancelPixelAnimations() {
  if ( qtyActivePixelAnimations != 0 ) {
    //Serial.println("DigitDisplay::cancelPixelAnimations()");
    qtyActivePixelAnimations = 0;
    for (uint8_t digitIdx = 0; digitIdx < QTY_DIGITS; digitIdx++) {  
      digitAnimationData[digitIdx].pixelAnimationPending = false;
    }
    // we must also reset all active pixelAnimationData
    for (uint16_t pixelIdx = 0; pixelIdx < QTY_LEDS; pixelIdx++) {
      PixelAnimationDataStruct &pad = pixelAnimationData[pixelIdx];
      pad.bActive = false;
      pad.pixelAnimationPhase = papIdle;
    }
  }

}

void DigitDisplay::maybeTriggerHueSatChangeApplication() {
  if ( bHueSatChanging ) {
    bApplyHueSatChange = true;
    bHueSatChanging = false;
  }
}