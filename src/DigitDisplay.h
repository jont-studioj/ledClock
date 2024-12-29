//MARK 2023-12-08 before change of globalBrightness to desiredBrightness & ledBrightness
#ifndef DIGIT_DISPLAY_H
#define DIGIT_DISPLAY_H

#include "BrightnessAdjustModeEnum.h"

#include "global_defines.h"
#include <FastLED.h>

enum DigitDisplayMode {
  DigitDisplayModeNone,
  DigitDisplayModeTime,
  DigitDisplayModeDate,
  DigitDisplayModeTimer,

  DigitDisplayModeEdit,       // used when we are editing some value (timer-duration, buzzer-duration, maybe time/date too)

  _DigitDisplayModeCount
};

// include this _after_ defining the DigitDisplayMode because it's referenced in this
#include "TimeSource.h"               // where I get the time/date etc

struct ColourData {
  int16_t hue;      // 0..255 twice over
  uint8_t sat;      // 0..255
};

struct DigitDisplayModeData {
  DigitDisplayMode digitDisplayMode;
  const char *name;
  bool seriffed;
  bool softened;
  bool fxEnabled;
  bool fxGrow;                // digit animation, mutually exclusive with stroke
  bool fxShrink;              // digit animation, mutually exclusive with unstroke
  bool fxStroke;              // digit animation, mutually exclusive with grow
  bool fxUnstroke;            // digit animation, mutually exclusive with shrink
  bool fxFadeIn;              // pixel animation: if both fade in and fade out are set then..
  bool fxFadeOut;             // pixel animation: ..treat as crossfade and ignore digit animations fxGrow/fxShrink
  uint8_t flashReducePct;     // used internally for edit modes, percent to dim by; 0 = no reduction (& no flash), 100 = full reduction (off)
  ColourData colourData1;
  ColourData colourData2;
};

// may want DigitDisplayAnimation separate from effects like fade/colour so they can be used in combination
enum DigitDisplayAnimation {
  DigitDisplayAnimationNone,
  DigitDisplayAnimationCrawl,
  DigitDisplayAnimationFade         
};


struct LedDigitStyleStruct {
  uint16_t maxStepsStrokeUnstroke;
  uint16_t maxStepsGrowShrink;
  uint16_t digitDefs[10][QTY_LEDS_PER_DIGIT];
};

enum DigitAnimationPhase {
  dapIdle,                    // not doing anything, nothing (more) to do
  dapStart,                   // initiate & do 1st step
  dapDecayOld,                // decaying old digit
  dapStartBuildNew,           // waiting to start buildNew
  dapBuildNew,                // building new digit
  dapDecayOldAndBuildNew      // for simultaneous decay/build (eg: cross fade) - maybe don't need this, just leave it to pixelProcess?
};

struct DigitAnimationDataStruct {
  DigitAnimationPhase digitAnimationPhase;
  uint8_t digitValueFrom;      // transiting from this...
  uint8_t digitValueTo;        // ...to this digit value
  uint8_t qtySteps;
  uint8_t stepNo;
  uint8_t qtyActivePixelAnimations;
  bool pixelAnimationPending;
};

enum PixelAnimationPhase {
  papIdle,                    // not doing anything, not yet started
  papStart,                   // initiate & do 1st step
  papDoing,                   // in process
  papDone                     // done, do I really need this?
};

struct PixelAnimationDataStruct {
  PixelAnimationPhase pixelAnimationPhase;
  uint8_t lumValueCurrent;      // this should be what we actually send?
  uint8_t lumValueTarget;
  int8_t lumStepValue;
  bool bActive;
};

struct RefProvider;

class DigitDisplay {
public:
  void begin(RefProvider *refProvider);

  //void saveConfig();  // unused
  void clearConfig();
  void saveDigitDisplayModeData(DigitDisplayModeData *digitDisplayModeData);
  void saveAmbientAdjustConfig();
  void saveSundriesConfig();

  void setRgbCorrection(uint32_t value, bool bSave);

  void setSoakTestMode(bool value);

  void setDoingGammaValueAdjust(bool value);
  void setGammaValue(float value, bool bRepaint = true);
  float getGammaValue();

  uint8_t getUserBrightness();
  void setUserBrightness(uint8_t value);

  void checkDaylightBoost(bool bImmediate);
  uint8_t getDaylightBoost();
  void setDaylightBoost(uint8_t value);

  void setUserBrightnessBegin(UserBrightnessAdjustMode adjustMode);
  void setUserBrightnessEnd(UserBrightnessAdjustMode adjustMode);
  void changedLuxX100(uint32_t newLuxX100, bool bSmoothing);

  void previewColourData(ColourData *colourData);
  void forceFullSat(bool bForce);

  int16_t lerpTestVal;
  void setLerpTestVal(int16_t val);
  int16_t getLerpTestVal();

  uint16_t getHueSatLerp16();

  uint16_t getHue1();
  void setHue1(uint16_t value);
  uint8_t getSat1();
  void setSat1(uint8_t value);
  
  uint16_t getHue2();
  void setHue2(uint16_t value);
  uint8_t getSat2();
  void setSat2(uint8_t value);

  uint8_t getCurrentHue();
  uint8_t getCurrentSat();

  void setFlashReducePct(uint8_t value);
  bool getFlashing();

  bool getDigitStyleSeriffed();
  void setDigitStyleSeriffed(bool value);

  bool getDigitStyleSoftened();
  void setDigitStyleSoftened(bool value);

  void setTempDisableFx(bool value);

  DigitDisplayMode getActiveDigitDisplayMode();
  bool setActiveDigitDisplayMode(DigitDisplayMode digitDisplayMode, bool bDisplayNow);
  DigitDisplayModeData *getDigitDisplayModeData(DigitDisplayMode digitDisplayMode);
  bool autoSelectDigitDisplayMode();
  void cycleDigitDisplayMode(int8_t direction);

  void applyActiveDigitDisplayModeSettings();
  void applySettingsAndDisplayNow();

  void previewEffects(bool value);

  void startEditMode();
  void stopEditMode();
  
void dump(uint8_t digitIdx);


  void loop();

  void blankAllLeds();

  void invalidateAllDigits();
  void pauseUpdates();
  void allowUpdates();

  void blankThisDigit(uint8_t digitIdx);
  void setThisDigitWithThisDigitValue(uint8_t digitIdx, uint8_t digitValue);

  bool bDebug = false;

//TODO: should really be private and have getter/setters
//they are public at the moment so main cli stuff can futz with them
uint8_t pixelAnimStepsFadeIn = 20;
uint8_t pixelAnimStepsFadeOut = 24;

private:
  RefProvider *refProvider;

  // config namespaces & keys

  // display mode data, these use the mode name for the namespace
  const char *cfgModeDataKeySeriffed =        "seriffed";
  const char *cfgModeDataKeySoftened =        "softened";
  const char *cfgModeDataKeyFxEnabled =       "fxEnabled";
  const char *cfgModeDataKeyFxStroke =        "fxStroke";     // mutually exlusive with grow
  const char *cfgModeDataKeyFxUnstroke =      "fxUnstroke";   // mutually exlusive with shrink
  const char *cfgModeDataKeyFxGrow =          "fxGrow";       // mutually exlusive with stroke
  const char *cfgModeDataKeyFxShrink =        "fxShrink";     // mutually exlusive with unstroke
  const char *cfgModeDataKeyFxFadeIn =        "fxFadeIn";
  const char *cfgModeDataKeyFxFadeOut =       "fxFadeOut";
  const char *cfgModeDataKeyColour1Hue =      "col1Hue";
  const char *cfgModeDataKeyColour1Sat =      "col1Sat";
  const char *cfgModeDataKeyColour2Hue =      "col2Hue";
  const char *cfgModeDataKeyColour2Sat =      "col2Sat";

  // values for ambient adjust                 1234567890123
  const char *cfgAaNamespace =                "digitAA";
  const char *cfgAaLuxMin =                   "aaLuxMin";
  const char *cfgAaLedMin =                   "aaLedMin";
  const char *cfgAaLuxMax =                   "aaLuxMax";
  const char *cfgAaLedMax =                   "aaLedMax";
  const char *cfgAaDayBoost =                 "aaDayBoost";
  
  // random sundries for digitDisplay config
  const char *cfgSundriesNamespace =          "digitSundries";
  const char *cfgSundriesRgbCorrection =      "rgbCorrection";
  const char *cfgSundriesGammaValue =         "gammaValue";
  

  void loadConfig();
  void loadDigitDisplayModeData(DigitDisplayModeData *digitDisplayModeData);
  void clearDigitDisplayModeData(DigitDisplayModeData *digitDisplayModeData);
  void loadAmbientAdjustConfig();
  void clearAmbientAdjustConfig();
  void loadSundriesConfig();
  void clearSundriesConfig();

  // leds seem to vary in their hue, this is a colour correction value
  // format: RRGGBB in a ulong, eg: 0xFFFFFF = normal, 0xF0FFF0 = reduce R & B compare to G
  uint32_t rgbCorrection = RGB_CORRECTION;

  bool bDoingSoakTest;
  uint16_t soakTestIdx;
  int8_t soakTestHueDir;
  int16_t soakTestHueVal;
  int8_t soakTestSatDir;
  int16_t soakTestSatVal;
  int8_t soakTestLumDir;
  int16_t soakTestLumVal;
  

  // some hacky overriding flag to indicate we're doing gamma value adjust
  // and, if we are, we disable ambient adjust, ignore actual digit display
  // and instead display 101 pixels in 101 (user, 0-100) brightnesses
  bool bDoingGammaValueAdjust;

  // gamma value (used in gammaCorrect)
  float_t gammaValue = GAMMA_VALUE;
  uint8_t brightness_Hi;                 // 1..100 (0 is valid too, but we don't let the user go that low) (flash-Hi or normal)
  uint8_t brightness_Hi_GC;              // 0..255 re-ranged and gamma corrected version of brightness_Hi (flash-Hi or normal)
  uint8_t brightness_Hi_GC_AA;           // 0..255 as above but with ambient adjustment (flash-Hi or normal)
  uint8_t brightness_Lo;                 // 1..100 (0 is valid too, but we don't let the user go that low) (flash-Lo)
  uint8_t brightness_Lo_GC;              // 0..255 re-ranged and gamma corrected version of brightness_Lo (flash-Lo)
  uint8_t brightness_Lo_GC_AA;           // 0..255  0..255 as above but with ambient adjustment (flash-Lo)

  // these are persisted as user preferences
  uint32_t minLuxX100;
  uint8_t minBrightness_Hi;
  uint8_t minBrightness_Hi_GC;
  uint32_t maxLuxX100;
  uint8_t maxBrightness_Hi;
  uint8_t maxBrightness_Hi_GC;

  uint8_t daylightBoost_Hi;
  uint8_t daylightBoost_Hi_GC;
  uint8_t daylightBoost_Lo;
  uint8_t daylightBoost_Lo_GC;

  uint32_t currentLuxX100;

  uint8_t actualLedBrightness;
  uint8_t targetLedBrightness;
  
  bool bAmbientAutoAdjustTempDisabled;

  bool bFlashing;
  uint8_t flashReducePct;
  bool bFlashFullNow;
  const uint32_t flashPhaseDurationMS = 500;
  uint32_t flashPhaseChangeTimeMS;

  void setDoingCommissioningTypeAdjustments(bool value);

  void paintPixelsWithSoakTest();
  void paintPixelsWithGammaGradient();

  uint8_t gammaCorrect(uint8_t inputBrightess);
  void ambientAdjustBrightness(bool bImmediate);
  void tempDisableAmbientAutoAdjust(bool bDisable);
  uint8_t ambientAdjustValue(uint8_t brightnessGC, uint8_t minBrightnessGC, uint8_t daylightBoostValue);
  void calculateBrightnessValuesHi();
  void calculateBrightnessValuesLo();

  void setMinBrightness_Hi(uint8_t value);
  void setMaxBrightness_Hi(uint8_t value);
  
  void setTargetLedBrightness(uint8_t value, bool bImmediate);
  void setActualLedBrightness(uint8_t value);
  void seekTargetLedBrightness();

  


  // valid digit-values are simply 0..9
  const static uint8_t invalidDigitValue = 254;     // used to force a digit update
  const static uint8_t blankedDigitValue = 255;     // used to represent a blank digit
  
  // LED output buffer
  CRGB ledBuffer[QTY_LEDS];

  uint16_t hue1 = 0;
  uint8_t sat1 = 0;
  uint16_t hue2 = 0;
  uint8_t sat2 = 0;
  uint8_t currentHue = 0;
  uint8_t currentSat = 0;
  const uint8_t normalLumVal = 255;
  uint8_t dimmedLumVal = normalLumVal;

  void calculateHueSat();

  ColourData *previewingColourData = NULL;
  bool bForceFullSat = false;

  bool digitStyleSeriffed = false;  // digit seriffed style
  bool digitStyleSoftened = false;   // digit softened style
  bool digitAnimationFxBuildNew = false;
  bool digitAnimationFxDecayOld = false;
  bool digitAnimationFxStroke = false;
  bool digitAnimationFxUnstroke = false;
  bool digitAnimationFxGrow = false;
  bool digitAnimationFxShrink = false;
  bool pixelAnimationFadeIn = false;
  bool pixelAnimationFadeOut = false;

  uint32_t qtyActivePixelAnimations = 0;
  bool bTempDisableFx = false;

  bool bHueSatChanging = true;
  bool bApplyHueSatChange = false;
  bool bDigitStyleChanged = true;

  DigitDisplayMode activeDigitDisplayMode = DigitDisplayModeNone;
  DigitDisplayMode priorToEditDigitDisplayMode = DigitDisplayModeNone;

  bool updatesEnabled;

  DigitDisplayModeData digitDisplayModeDataList[_DigitDisplayModeCount];
  bool bEffectsPreview;
  uint8_t fxDigits[QTY_DIGITS];

  uint8_t digitValues[QTY_DIGITS];              // current digit-value at digit-position zero rightmost
  uint8_t previousSecondValue = 99;

  DigitAnimationDataStruct digitAnimationData[QTY_DIGITS];  // digit animation info
  PixelAnimationDataStruct pixelAnimationData[QTY_LEDS];    // pixel animation info

  void doTickingUpdate();
  void maybeUpdateAllDigits(bool bNewSecond);
  bool maybeUpdateThisDigit(uint8_t digitIdx, uint8_t digitValue);
  void updateThisDigitWithThisDigitValue(uint8_t digitIdx, uint8_t digitValue);

  void resetAnimationData();
  void doDigitAnimations(bool bAllowAnimation);
  void doDigitAnimation(bool bAllowAnimation, uint8_t digitIdx);
  void cancelDigitAnimations();
  void paintDigitPixels(const LedDigitStyleStruct *digitData, uint8_t digitIdx, uint8_t digitValue, bool bBlanking, uint8_t matchStepNo, bool growShrinkDigitFx);

  bool bPixelAnimationPending = false;

  void doPixelAnimations(bool bAllowAnimation);
  void cancelPixelAnimations();

  void maybeTriggerHueSatChangeApplication();

};

#endif // DIGIT_DISPLAY_H
