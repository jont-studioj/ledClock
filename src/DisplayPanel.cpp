#include "DisplayPanel.h"

#include "RefProvider.h"
#include <Preferences.h>
#include "TimeSource.h"

#include "Sundries.h"

//#include "DisplayCfg.h"

void DisplayPanel::begin(RefProvider *refProvider) {
  this->refProvider = refProvider;
  
  // power the display down (might help stop us seeing random crap on the display?)
  powerDown();

  // Initialize the super class
  init();

  // clear display
  clearDisplay();
  
  setFlashingDimmed(false);

  initPwmDimming();

  currentLuxX100 = 0;

  bAmbientAutoAdjustTempDisabled = false;

  // set reasonable defaults in case there's no config saved)
  minLuxX100 = 1;
  setMinBrightness_Hi(1);
  maxLuxX100 = 1800;
  setMaxBrightness_Hi(60);

  setDaylightBoost(0);

  loadConfig();

  // default to vaguely mid brightness, until the config value is available
//todo:
//setDesiredBrightness(80, 0);
pwmDutyValueActual = 0;
brightness_Hi = 0;
setUserBrightness(50);

  // power the displays back up
  powerUp();
}

void DisplayPanel::loop() {
  seekTargetBrightnessDuty();
}

//todo: if we want this setable dynamically via config/menus
// then we may need to tear down old stuff before setting new stuff
void DisplayPanel::initPwmDimming() {
  // configure LED PWM functionalitites
  ledcSetup(DISPLAY_BACKLIGHT_PWM_LED_CHANNEL, DISPLAY_BACKLIGHT_PWM_FREQ, DISPLAY_BACKLIGHT_PWM_RESOLUTION);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(pwmDimmingPin, DISPLAY_BACKLIGHT_PWM_LED_CHANNEL);
}

void DisplayPanel::loadConfig() {
  loadAmbientAdjustConfig();
}

void DisplayPanel::loadAmbientAdjustConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgAaNamespace, true);
  minLuxX100 = preferences.getULong(cfgAaLuxMin, minLuxX100);
  setMinBrightness_Hi(preferences.getUChar(cfgAaPanMin, minBrightness_Hi));
  maxLuxX100 = preferences.getULong(cfgAaLuxMax, maxLuxX100);
  setMaxBrightness_Hi(preferences.getUChar(cfgAaPanMax, maxBrightness_Hi));
  setDaylightBoost(preferences.getUChar(cfgAaDayBoost, daylightBoost_Hi));
  preferences.end();
  //Serial.printf("DisplayPanel::loadAmbientAdjustConfig():  minLuxX100=%d, minBrightness_Hi=%d, maxLuxX100=%d, maxBrightness_Hi=%d\n", minLuxX100, minBrightness_Hi, maxLuxX100, maxBrightness_Hi);
}
void DisplayPanel::saveAmbientAdjustConfig() {
  //Serial.printf("DisplayPanel::saveAmbientAdjustConfig():  minLuxX100=%d, minBrightness_Hi=%d, maxLuxX100=%d, maxBrightness_Hi=%d\n", minLuxX100, minBrightness_Hi, maxLuxX100, maxBrightness_Hi);
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgAaNamespace, false);
  preferences.putULong(cfgAaLuxMin, minLuxX100);
  preferences.putUChar(cfgAaPanMin, minBrightness_Hi);
  preferences.putULong(cfgAaLuxMax, maxLuxX100);
  preferences.putUChar(cfgAaPanMax, maxBrightness_Hi);
  preferences.putUChar(cfgAaDayBoost, daylightBoost_Hi);
  preferences.end();
}
void DisplayPanel::clearAmbientAdjustConfig() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(cfgAaNamespace, false);
  preferences.clear();
  preferences.end();
}

//todo: need to call this on change? AND after ambientAdjust (but what if there no ambient adjust happens after the crossover to/from daylight?)
void DisplayPanel::checkDaylightBoost(bool bImmediate) {
  //Serial.println("DisplayPanel::checkDaylightBoost()");
  // is called by UiMenuSystemBrightness::menuItemOptionChange() when set/clear enable
  // also called when setDaylightBoost(uint8_t value) called
  ambientAdjustBrightness(false);
}


// "user brightness" deals with 1..100 brightness_Hi
uint8_t DisplayPanel::getUserBrightness() {
  return brightness_Hi;
}
void DisplayPanel::setUserBrightness(uint8_t value) {
//if ( brightness_Hi != value ) {
    brightness_Hi = value;
    // note: when setting UserBrightness directly
    // we use the gamma-corrected BUT NOT ambient-light-adjusted value
    // (we might be trying to set up the upper/lower mappings of lux vs led-brightness)
    brightness_Hi_GC_duty = gammaCorrectToDuty(brightness_Hi);
//Serial.printf("DisplayPanel::setUserBrightness(%d): brightness_Hi_GC_duty=%d\n", value, brightness_Hi_GC_duty);
    setTargetBrightnessDuty(brightness_Hi_GC_duty, true);
//}
}

void DisplayPanel::setUserBrightnessBegin(UserBrightnessAdjustMode adjustMode) {
  if ( adjustMode == UserBrightnessAdjustMode::ubamMin ) {
    tempDisableAmbientAutoAdjust(true);
    setUserBrightness(minBrightness_Hi);
  } else if ( adjustMode == UserBrightnessAdjustMode::ubamMax ) {
    tempDisableAmbientAutoAdjust(true);
    setUserBrightness(maxBrightness_Hi);
  }// we do nothing special for ubamDaylightBoost
}
void DisplayPanel::setUserBrightnessEnd(UserBrightnessAdjustMode adjustMode) {
  if ( adjustMode == UserBrightnessAdjustMode::ubamMin ) {
    setMinBrightness_Hi(brightness_Hi);
    minLuxX100 = currentLuxX100;
    tempDisableAmbientAutoAdjust(false);
  } else if ( adjustMode == UserBrightnessAdjustMode::ubamMax ) {
    setMaxBrightness_Hi(brightness_Hi);
    maxLuxX100 = currentLuxX100;
    tempDisableAmbientAutoAdjust(false);
  }// we do nothing special for ubamDaylightBoost
}

void DisplayPanel::tempDisableAmbientAutoAdjust(bool bDisable) {
  bAmbientAutoAdjustTempDisabled = bDisable;
  // if re-enabling auto-adjust then recalc and show now
  if ( !bAmbientAutoAdjustTempDisabled ) {
    //Serial.printf("tempDisableAmbientAutoAdjust(%d): minLuxX100=%d, minBrightness_Hi=%d, minBrightness_Hi_GC=%d, maxLuxX100=%d, maxBrightness_Hi=%d, maxBrightness_Hi_GC=%d\n", bDisable, minLuxX100, minBrightness_Hi, minBrightness_Hi_GC, maxLuxX100, maxBrightness_Hi, maxBrightness_Hi_GC);
    ambientAdjustBrightness(true);
  }
}
void DisplayPanel::changedLuxX100(uint32_t newLuxX100, bool bSmoothing) {
  currentLuxX100 = newLuxX100;
  ambientAdjustBrightness( !bSmoothing );
}

void DisplayPanel::setTargetBrightnessDuty(uint32_t value, bool bImmediate) {
  //Serial.printf("DisplayPanel::setTargetBrightnessDuty(%d, %d)\n", value, bImmediate);
  pwmDutyValueTarget = value;
  // if immediate - or if wifi is needing minimal interference, go directly to target 
  //  (and thus avoid repeated futzes with h/w
  if ( bImmediate || refProvider->sundries.getWifiPrioritised() ) {  
    setActualBrightnessDuty(pwmDutyValueTarget);
  }
}

void DisplayPanel::setActualBrightnessDuty(uint32_t value) {
  if ( pwmDutyValueActual != value ) {
    pwmDutyValueActual = value;
    //Serial.printf("DisplayPanel::setActualBrightnessDuty(%d)\n", value);  
    ledcWrite(DISPLAY_BACKLIGHT_PWM_LED_CHANNEL, pwmDutyValueActual);
  }
}

void DisplayPanel::seekTargetBrightnessDuty() {
  int32_t dutyDiff = pwmDutyValueTarget - pwmDutyValueActual;
  if ( dutyDiff != 0 ) {
    int32_t newDuty;
    if ( dutyDiff < 0 ) {
      newDuty = pwmDutyValueActual - ( dutyDiff < -100 ? 100 : 10 );
      if ( newDuty < pwmDutyValueTarget ) {
        newDuty = pwmDutyValueTarget;
      }
    } else if ( dutyDiff > 0 ) {
      newDuty = pwmDutyValueActual + ( dutyDiff > +100 ? 100 : 10 );
      if ( newDuty > pwmDutyValueTarget ) {
        newDuty = pwmDutyValueTarget;
      }
    }
    //Serial.printf("DisplayPanel::seekTargetBrightnessDuty(): pwmDutyValueTarget=%d, pwmDutyValueActual=%d, dutyDiff=%d, newDuty=%d\n", pwmDutyValueTarget, pwmDutyValueActual, dutyDiff, newDuty);
    setActualBrightnessDuty(newDuty);
  }
}

void DisplayPanel::calculateBrightnessValuesHi() {
  brightness_Hi = maxBrightness_Hi;
  brightness_Hi_GC_duty = gammaCorrectToDuty(maxBrightness_Hi);
  brightness_Hi_GC_duty_AA = ambientAdjustDutyValue(brightness_Hi_GC_duty, minBrightness_Hi_GC, daylightBoost_Hi_GC);
  //Serial.printf("DisplayPanel::calculateBrightnessValuesHi(): maxBrightness_Hi=%d, brightness_Hi=%d, brightness_Hi_GC_duty=%d, brightness_Hi_GC_AA=%d\n", maxBrightness_Hi, brightness_Hi, brightness_Hi_GC_duty, brightness_Hi_GC_duty_AA);
}

void DisplayPanel::calculateBrightnessValuesLo() {
  // calculate the reduced brightness value from the max-brightness_Hi *before* GC or ambient adjust
  // then do our own GC and ambient adjust for this Lo value
  brightness_Lo = maxBrightness_Hi / 2;
  brightness_Lo_GC_duty = gammaCorrectToDuty(brightness_Lo);

  daylightBoost_Lo = daylightBoost_Hi / 2;
  daylightBoost_Lo_GC = (daylightBoost_Lo == 0) ? 0 : gammaCorrectToDuty(daylightBoost_Lo);
  brightness_Lo_GC_duty_AA = ambientAdjustDutyValue(brightness_Lo_GC_duty, 60, daylightBoost_Lo_GC);
  //Serial.printf("DisplayPanel::calculateBrightnessValuesLo(): maxBrightness_Hi=%d, brightness_Lo=%d, brightness_Lo_GC_duty=%d, brightness_Lo_GC_AA=%d\n", maxBrightness_Hi, brightness_Lo, brightness_Lo_GC_duty, brightness_Lo_GC_duty_AA);  
}

void DisplayPanel::setMinBrightness_Hi(uint8_t value) {
  minBrightness_Hi = value;
  minBrightness_Hi_GC = gammaCorrectToDuty(minBrightness_Hi);
}
void DisplayPanel::setMaxBrightness_Hi(uint8_t value) {
  maxBrightness_Hi = value;
  maxBrightness_Hi_GC = gammaCorrectToDuty(maxBrightness_Hi);
}

uint8_t DisplayPanel::getDaylightBoost() {
  return daylightBoost_Hi;
}
void DisplayPanel::setDaylightBoost(uint8_t value) {
  daylightBoost_Hi = value;
  daylightBoost_Hi_GC = (daylightBoost_Hi == 0) ? 0 : gammaCorrectToDuty(daylightBoost_Hi);
  // apply that change
  checkDaylightBoost(true);
}

uint32_t DisplayPanel::gammaCorrectToDuty(uint8_t inputBrightess) {
  int32_t retVal;

  // convert the 0-100 brightess level to an appropriate PWM duty cycle
  //
  // I found that there was not a linear relationship so had to futz around
  // a lot trying to find an acceptable conversion that at least _felt_ more linear.
  //
  // I'm no mathematician so it took a lot of futzing with:
  // https://www.desmos.com/calculator
  // playing with various equations:
  //  y = exp( (x-1) * e ) - .05
  //    nope, try again...
  //
  //  y = exp( (x-1) * 2 * e ) + .015
  //    bit better, but not quite...
  //
  //  y = exp( 0.15 + (x - 1.035) * 1.75 * e)
  //    that'll bloody well do, so long as I nudge the bottom end up a tad
  //
  // So I settled on 3 (yes, the one with the most fudges in it)
  // then I fudged it further in code by adding a low end offset (the "y + 10" below)
  // (because, with the pwm / h/w setup that I have, you didn't seem to get any visible
  //  illumination until a cetain PWM duty was reached)
  //
  // It's just occurred to me that maybe a sin or cos would've been better... but I'm
  // sticking with what I've got for now.
  //
  // Note: I'm not worried too much about the complexity of the following code because
  // in the normal run of things it won't be being called very often
  
  // note: this is currently hard-coded for a fixed 12bit pwm resolution (so, duty goes from 0-4095)
  const float e=2.71828;
  float x = inputBrightess/100.0;
  float y = exp(0.15 + (x - 1.035) * e * 1.75);
  retVal = 4096.0 * y + 10;
  if ( retVal < 0 ) {
    retVal = 0;
  } else if ( retVal > 4095 ) {
    retVal = 4095;
  }
  // don't laugh, I'm keen to get on with something else so the above,
  // as they say, works ok and will have to do for now.
  //
  // (and, for the record, I wrote this a long time before I wrote
  //  the equivalent (but much simpler) gamma-correct code that's in
  //  DigitDisplay and I should probably do much the same here, or
  //  even use a common function for both, albeit with parameters)

  return retVal;
}


void DisplayPanel::ambientAdjustBrightness(bool bImmediate) {
  //Serial.print("DisplayPanel::ambientAdjustBrightness()");
  // only do stuff if ambient adjust is enabled
  if ( !bAmbientAutoAdjustTempDisabled && !refProvider->sundries.getWifiPrioritised() ) {
    if ( bFlashingDimmed ) {
      // flash lo
      calculateBrightnessValuesLo();
      setTargetBrightnessDuty(brightness_Lo_GC_duty_AA, bImmediate);
    } else {
      // flash hi or normal
      calculateBrightnessValuesHi();
      //Serial.printf("DisplayPanel::ambientAdjustBrightness(): brightness_Hi=%d, brightness_Hi_GC_duty=%d, brightness_Hi_GC_AA=%d\n", brightness_Hi, brightness_Hi_GC_duty, brightness_Hi_GC_duty_AA);      
      setTargetBrightnessDuty(brightness_Hi_GC_duty_AA, bImmediate);
    }
  }
}

uint32_t DisplayPanel::ambientAdjustDutyValue(uint32_t brightnessValue_GC_duty, uint32_t minBrightnessGCduty, uint32_t daylightBoostValue) {
  uint32_t result = brightnessValue_GC_duty;
    
  // have current currentLuxX100  which is likely to be somewhere between:
  // have minLuxX100              & maxLuxX100
  // then want to adjust brightnessValue to be proportionally between:
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
    //Serial.printf("DisplayPanel::ambientAdjustDutyValue(%d, %d) ( currentLuxX100Offset < 0 ) setting currentLuxX100Offset to 0\n", brightnessValue_GC_duty, minBrightnessGCduty);
    currentLuxX100Offset = 0;
  } else if ( currentLuxX100Offset > minMaxLuxX100Range ) {
    //Serial.printf("ambientAdjustDutyValue(%d, %d) ( currentLuxX100Offset > minMaxLuxX100Range ) setting currentLuxX100Offset to minMaxLuxX100Range\n", brightnessValue_GC_duty, minBrightnessGCduty);
    currentLuxX100Offset = minMaxLuxX100Range;
  }

  float currentAmbientLightProportion = ((float)currentLuxX100Offset / (float)minMaxLuxX100Range);

  int32_t outputRange = brightnessValue_GC_duty - minBrightnessGCduty;
  result = currentAmbientLightProportion * outputRange + minBrightnessGCduty;

  // now apply any daylightBoost if applicable
  if ( refProvider->timeSource.inDaylight() ) {
//Serial.printf("DisplayPanel::ambientAdjustDutyValue(%d, %d, %d): boosting %d to %d\n", brightnessValue_GC_duty, minBrightnessGCduty, daylightBoostValue, result, result+daylightBoostValue);
    result += daylightBoostValue;
  }

  if ( result < minBrightnessGCduty ) {
    result = minBrightnessGCduty;
  } else if ( result > maxBrightness_Hi_GC ) {
//Serial.printf("DisplayPanel::ambientAdjustDutyValue() result(%d) > maxBrightness_Hi_GC(%d)\n", result, maxBrightness_Hi_GC);
    result = maxBrightness_Hi_GC;
  }

  //Serial.printf("DisplayPanel::ambientAdjustDutyValue(%d, %d): minLuxX100=%d, maxLuxX100=%d, minMaxLuxX100Range=%d, currentLuxX100Offset=%d, currentAmbientLightProportion=%.2f, minBrightnessGCduty=%d, maxBrightness_Hi_GC=%d, result=%d\n", brightnessValue_GC_duty, minBrightnessGCduty, minLuxX100, maxLuxX100, minMaxLuxX100Range, currentLuxX100Offset, currentAmbientLightProportion, minBrightnessGCduty, maxBrightness_Hi_GC, result);

  return result;
}

void DisplayPanel::setFlashingDimmed(bool value) {
  bFlashingDimmed = value;
  ambientAdjustBrightness(true);
}

// void DisplayPanel::setCurrentPwmDimmingDuty(uint32_t newDutyValue) {
// //  if ( bPwmDimmable ) {
//     pwmDutyValueCurrent = newDutyValue;
//     ledcWrite(DISPLAY_BACKLIGHT_PWM_LED_CHANNEL, pwmDutyValueCurrent);
// //  }
// }
// void DisplayPanel::setActualDuty(uint32_t value) {
//   pwmDutyValueCurrent = value;
//   ledcWrite(DISPLAY_BACKLIGHT_PWM_LED_CHANNEL, pwmDutyValueCurrent);
// }


// enable / disable / toggle display
void DisplayPanel::powerUp() {
//todo: do whatever HW power up can be done
  bPoweredUp = true;
}
void DisplayPanel::powerDown() {
//todo: do whatever HW power down can be done  
  bPoweredUp = false;
}
void DisplayPanel::powerToggle() {
  if (bPoweredUp) {
    powerDown();
  }  else {
    powerUp();
  }
}
bool DisplayPanel::isPoweredUp() {
  return bPoweredUp; 
}

void DisplayPanel::clearDisplay() {
  //Serial.println("DisplayPanel::clearDisplay()");
  fillScreen(TFT_BLACK);
}

// void DisplayPanel::displayImageBuffer(uint16_t *imageBuffer) {
// 
//   uint32_t StartTime = millis();
//   
//   bool oldSwapBytes = getSwapBytes();
//   setSwapBytes(true);
//   // was: pushImage(0,0, TFT_WIDTH, TFT_HEIGHT, (uint16_t *)imageBuffer);
//   pushImage(0,0, TFT_WIDTH, TFT_HEIGHT, imageBuffer);
//   setSwapBytes(oldSwapBytes);
// 
// #ifdef DEBUG_OUTPUT
//   Serial.print("img transfer: ");  
//   Serial.print(millis() - StartTime);
//   Serial.println("ms");
// #endif
// 
// }




// // hacky test code for poking commands/data to TFTs directly
// void DisplayPanel::tftPutCmd(uint8_t cmdValue) {
//   
// //  begin_nin_write();
//   writecommand(cmdValue);
//   delayMicroseconds(10);
// //  end_nin_write();
// 
// }
// 
// void DisplayPanel::tftPutCmdAndData(uint8_t cmdValue, uint8_t dataValue) {
//   
// //  begin_nin_write();
//   writecommand(cmdValue);
// //
// //  delayMicroseconds(10);
//   writedata(dataValue);
//   delayMicroseconds(10);
// //  end_nin_write();
// 
// 
// }