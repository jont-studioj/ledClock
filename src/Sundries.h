#ifndef SUNDRIES_H
#define SUNDRIES_H

#include "global_defines.h"

//todo: for debugging / analysis, remove at some point
//#define BH1750_DEBUG

#include <BH1750.h>

struct RefProvider;

// ui/menu colours
// (note: this stuff is here, rather than in UiMenu, for reasons)
struct DisplayPanelColourScheme {
  uint8_t idx;
  const char *name;
  uint16_t menuBg;
  uint16_t focusRing;
  uint16_t titleBg;
  uint16_t titleFg;
  uint16_t unselectedBg;
  uint16_t unselectedFg;
  uint16_t selectedBg;
  uint16_t selectedFg;
};


class Sundries {
public:
  void begin(RefProvider *refProvider);
  void initEspNow();
  void startEspNow();
  void stopEspNow();
  

  void loop();

  bool displayDoor_getPresent();
  void displayDoor_setPresent(bool value);
  void displayDoor_open(bool bForce, bool bSubdueServo = true);
  void displayDoor_shut(bool bForce, bool bSubdueServo = true);

  void displayDoor_tweakDutyValueBegin(bool bAdjustOpenDuty);
  void displayDoor_tweakDutyValueEnd();

  uint32_t displayDoor_getMinDuty();
  uint32_t displayDoor_getMaxDuty();
  uint32_t displayDoor_getMidDuty();

  uint32_t displayDoor_getDutyOpen();
  void displayDoor_setDutyOpen(uint32_t value);
  uint32_t displayDoor_getDutyShut();
  void displayDoor_setDutyShut(uint32_t value);
  void displayDoor_saveConfig();

  void setDigitSeparatorFlashing(bool value);
  void redrawDigitSeparator();

  void buzzer_start();
  void buzzer_stop();
  
  void saveDisplaySchemeIdx();
  void setDisplaySchemeIdx(uint8_t idx, bool bPersist);
  uint8_t getDisplaySchemeIdx();
  DisplayPanelColourScheme &getDisplayScheme();

  BH1750::Mode lightMeter_getMode();
  void lightMeter_setMode(BH1750::Mode mode);

  uint8_t lightMeter_getMtRegVal();
  void lightMeter_setMtRegVal(uint8_t value);
  uint32_t lightMeter_getLuxX100();

  void lightMeter_saveSmoothingQtySamples();

  uint8_t lightMeter_getSmoothingQtySamples();
  void lightMeter_setSmoothingQtySamples(uint8_t value);

  bool getWifiPrioritised();
  void setWifiPrioritised(bool value);

//  uint8_t gammaCorrect(uint8_t maxInput, uint8_t minOutput, uint8_t maxOutput, float_t gamma);

  // a hacky flag to indicate if we are in special commissioning mode
  // (ie: have extra menu config items that are only needed very rarely)
  // we enter this mode by holding down the button while booting
  bool bCommissioningMode = false;


private:
  RefProvider *refProvider;

  // ********************************************************************************************************************************
  // ********************************************************************************************************************************
  // displayDoor / servo stuff
  // ********************************************************************************************************************************

	// flag whether this device has a servo-driven-door over the display 
	// the (dark, translucent) display-door opens when the UI is active and closes when not
  bool DD_doorPresent = false;

  bool DD_doorOpen = false;
  const uint16_t DD_TimeoutPeriodMS = 300;
  uint32_t DD_timeoutTimeMS = 0;

  // PWM duty values, configurable, with default values
  uint32_t DD_dutyOpen = 4660;
  uint32_t DD_dutyShut = 5700;

  const uint32_t DD_minDuty = 3276;
  const uint32_t DD_maxDuty = 6552;
  const uint32_t DD_midDuty = (DD_minDuty + DD_maxDuty) / 2;

  const uint16_t DD_dutyTweakingTimeoutPeriodMS = 2000;
  uint32_t DD_dutyTweakingTimeoutTimeMS = 0;
  bool DD_dutyTweakForDoorOpen = false;   // whether we are tweaking the open(true) or closed(false) door position
  bool DD_dutyTweakInMidPosition = false; // when tweaking, whether we are currently in the mid position

  void displayDoor_init();
  void displayDoor_loadConfig();
  void displayDoor_primeIdleTimeout();
  void displayDoor_checkIdleTimeout();
  void displayDoor_idle();
  void displayDoor_setDuty(uint32_t duty, bool bSubdueServo);
  void displayDoor_tweakingGotoMid();
  void displayDoor_primeTweakingTimeout();
  void displayDoor_checkTweakingTimeout();


  const char *cfgDisplayDoorNamespace = "displayDoor";
  const char *cfgDisplayDoorPresent = "present";
  const char *cfgDisplayDoorDutyOpen = "dutyOpen";
  const char *cfgDisplayDoorDutyShut = "dutyShut";

  // ********************************************************************************************************************************
  // ********************************************************************************************************************************
  // digit separator display
  // ********************************************************************************************************************************
  enum DigitSeparatorIcon {
    DS_none,
    DS_time,
    DS_date,
    DS_timer
  };

  DigitSeparatorIcon digitSeparatorIcon = DS_none;
  void render_digit_separator();


  bool bDigitSeparatorFlashing;
  bool bDigitSeparatorFlashFull;
  const uint16_t DSF_PeriodMS = 500;
  uint32_t DSF_TimeoutTimeMS;
  void DSF_init();
  void DSF_primeTimeout();
  void DSF_loop();
  void DSF_setDisplayFullBrightness(bool value);

  

  // // ********************************************************************************************************************************
  // // buzzer
  // // ********************************************************************************************************************************
  bool buzzerOn;
  void buzzer_init(u_int8_t buzzerPin);
  void buzzer_setDuty(uint32_t duty);


  // ********************************************************************************************************************************
  // displaySchemeInit
  // ********************************************************************************************************************************
  // config namespaces & keys

  const char *cfgDisplaySchemeNamespace = "dispScheme";
  const char *cfgDisplaySchemeIdx = "schemeIdx";

  uint8_t displaySchemeIdx;
  DisplayPanelColourScheme displayScheme;
  void displaySchemeInit();
  void loadDisplaySchemeIdx();

  const uint8_t qtySchemes = 2;
  const DisplayPanelColourScheme schemes[2] = {
    {
      0,              // idx
      "cyan/grey",    // name
      0x0000,         // menuBg
      0x07FF,         // focusRing
      0x03EF,         // titleBg
      0xFFFF,         // titleFg
      0x0000,         // unselectedBg
      0xce79,         // unselectedFg
      0x4208,         // selectedBg
      0xe73c          // selectedFg
    },
    {
      1,              // idx
      "amber",        // name
      0x0000,         // menuBg
      0xf62b,         // focusRing
      0x9343,         // titleBg
      0xFFFF,         // titleFg
      0x0000,         // unselectedBg
      0xd5ac,         // unselectedFg
      0x4a02,         // selectedBg
      0xee4e          // selectedFg
    }
  };


  // ********************************************************************************************************************************
  // ambient light measurement (BH1750) stuff
  // ********************************************************************************************************************************
  // config namespaces & keys
  const char *cfgLightMeterNamespace = "lightMeter";
  const char *cfgLightMeterSmoothingQtySamples = "qtySamples";


  BH1750 lightMeter;
  BH1750::Mode lightMeter_mode;
  uint8_t lightMeter_MTreg;

  void lightMeter_init(BH1750::Mode mode, uint8_t MTreg);

  void lightMeter_loadSmoothingQtySamples();


  // BH1750 Mode:
  // UNCONFIGURED = 0,
  // 
  // CONTINUOUS_HIGH_RES_MODE =    0x10, // 16 Measurement at 1   lux resolution. Measurement time is approx 120ms
  // CONTINUOUS_HIGH_RES_MODE_2 =  0x11, // 17 Measurement at 0.5 lux resolution. Measurement time is approx 120ms
  // CONTINUOUS_LOW_RES_MODE =     0x13, // 19 Measurement at 4   lux resolution. Measurement time is approx  16ms
  // 
  // ONE_TIME_HIGH_RES_MODE =      0x20, // 32 Measurement at 1   lux resolution. Measurement time is approx 120ms
  // ONE_TIME_HIGH_RES_MODE_2 =    0x21, // 33 Measurement at 0.5 lux resolution. Measurement time is approx 120ms
  // ONE_TIME_LOW_RES_MODE =       0x23  // 35 Measurement at 4   lux resolution. Measurement time is approx  16ms

  uint8_t lightMeter_qtySamples = 0;  // 0 is invalid, forces the setter to do something at start up
  float lightMeter_runningSampleSum;
  float lightMeter_damped_sample;
  uint32_t lightMeter_avgLuxX100;
  void lightMeter_loop();



  // ********************************************************************************************************************************
  // random stuff
  // ********************************************************************************************************************************
  void checkWifiPrioritised();
  bool wifiPrioritised;

};


#endif // SUNDRIES_H
