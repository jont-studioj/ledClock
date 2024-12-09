#ifndef DISPLAYPANEL_H
#define DISPLAYPANEL_H

#include "BrightnessAdjustModeEnum.h"

#include "global_defines.h"
#include <TFT_eSPI.h>

struct RefProvider;

class DisplayPanel : public TFT_eSPI {
public:
  void begin(RefProvider *refProvider);

  void loop();

  // Controls the power
  void powerUp();
  void powerDown();
  void powerToggle();
  bool isPoweredUp();

  void clearConfig();
  void saveAmbientAdjustConfig();

  uint8_t getUserBrightness();
  void setUserBrightness(uint8_t value);

  void checkDaylightBoost(bool bImmediate);
  uint8_t getDaylightBoost();
  void setDaylightBoost(uint8_t value);

  void setUserBrightnessBegin(UserBrightnessAdjustMode adjustMode);
  void setUserBrightnessEnd(UserBrightnessAdjustMode adjustMode);
  void changedLuxX100(uint32_t newLuxX100, bool bSmoothing);

  void setFlashingDimmed(bool value);

  void clearDisplay();
  
//  void displayImageBuffer(uint16_t *imageBuffer);

// // hacky test code for poking commands/data to TFTs directly
// void tftPutCmd(uint8_t cmdValue);
// void tftPutCmdAndData(uint8_t cmdValue, uint8_t dataValue);
  
private:
  RefProvider *refProvider;

  // values for ambient adjust                 1234567890123
  const char *cfgAaNamespace =                "panelAA";
  const char *cfgAaLuxMin =                   "aaLuxMin";
  const char *cfgAaPanMin =                   "aaPanMin";
  const char *cfgAaLuxMax =                   "aaLuxMax";
  const char *cfgAaPanMax =                   "aaPanMax";
  const char *cfgAaDayBoost =                 "aaDayBoost";
  

  bool bPoweredUp = false;

//  uint8_t brightnessDesired;
//  uint8_t brightnessCurrent;
  uint8_t brightness_Hi;                 // 1..100 (0 is valid too, but we don't let the user go that low) (flash-Hi or normal)
  uint32_t brightness_Hi_GC_duty;        // 0..4095 re-ranged and gamma corrected version of brightness_Hi (flash-Hi or normal)
  uint32_t brightness_Hi_GC_duty_AA;     // 0..4095 as above but with ambient adjustment (flash-Hi or normal)
  uint8_t brightness_Lo;                 // 1..100 (0 is valid too, but we don't let the user go that low) (flash-Lo)
  uint32_t brightness_Lo_GC_duty;        // 0..4095 re-ranged and gamma corrected version of brightness_Lo (flash-Lo)
  uint32_t brightness_Lo_GC_duty_AA;     // 0..4095 as above but with ambient adjustment (flash-Lo)


  // these are persisted as user preferences
  uint32_t minLuxX100;
  uint8_t minBrightness_Hi;
  uint32_t minBrightness_Hi_GC;
  uint32_t maxLuxX100;
  uint8_t maxBrightness_Hi;
  uint32_t maxBrightness_Hi_GC;

  uint8_t daylightBoost_Hi;
  uint32_t daylightBoost_Hi_GC;
  uint8_t daylightBoost_Lo;
  uint32_t daylightBoost_Lo_GC;


  uint32_t currentLuxX100;

  uint8_t pwmDimmingPin = DISPLAY_BACKLIGHT_PWM_PIN;
  uint32_t pwmDutyMax = (1 << DISPLAY_BACKLIGHT_PWM_RESOLUTION) - 1;
  uint32_t pwmDutyValueTarget;
  uint32_t pwmDutyValueActual;

  bool bAmbientAutoAdjustTempDisabled;

  void setMinBrightness_Hi(uint8_t value);
  void setMaxBrightness_Hi(uint8_t value);

  void setTargetBrightnessDuty(uint32_t value, bool bImmediate);
  void setActualBrightnessDuty(uint32_t value);
  void seekTargetBrightnessDuty();

  bool bFlashingDimmed;

  // pwm dimming
  void initPwmDimming();
//  void setCurrentPwmDimmingDuty(uint32_t newDutyValue);

// old testing: (failed)
//void setST7789dimming(uint8_t newValue);


  void loadConfig();
  void loadAmbientAdjustConfig();
  void clearAmbientAdjustConfig();

  uint32_t gammaCorrectToDuty(uint8_t inputBrightess);
  void ambientAdjustBrightness(bool bImmediate);
  void tempDisableAmbientAutoAdjust(bool bDisable);
  uint32_t ambientAdjustDutyValue(uint32_t brightnessValue_GC_duty, uint32_t minBrightnessGCduty, uint32_t daylightBoostValue);
  void calculateBrightnessValuesHi();
  void calculateBrightnessValuesLo();


};

#endif // DISPLAYPANEL_H
