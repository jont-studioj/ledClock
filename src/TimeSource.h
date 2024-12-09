//MARK 2023-12-06
#ifndef TIMESOURCE_H
#define TIMESOURCE_H

#include <stdint.h>
#include "global_defines.h"
#include <TimeLib.h>
#include "Ui.h"


// For NTP
#include <WiFi.h>
#include "WifiExecutive.h"
#include "NTPClient_AO.h"

//for daylight calcs
#include <sunset.h>

struct RefProvider;

enum DebugForceDaylight {
  dfdForceNothing,
  dfdForceDay,
  dfdForceNight
};

enum TimeSourceEditMode {
  tsemNone,
  tsemTimerPresetDuration,
  tsemTimerCurrentDuration,
  tsemTimerBuzzerDuration
};

struct UtcOffsetDataType {
  int16_t offsetMinutes;
  const char *label;
};

#define UTC_OFFSET_INDEX_DEFAULT 14
#define UTC_OFFSET_INDEX_MIN 0
#define UTC_OFFSET_INDEX_MAX 37
static UtcOffsetDataType utcOffsetData[] = {
  {-720, "UTC-12:00"},  // 00
  {-660, "UTC-11:00"},  // 01
  {-600, "UTC-10:00"},  // 02
  {-570, "UTC-09:30"},  // 03
  {-540, "UTC-09:00"},  // 04
  {-480, "UTC-08:00"},  // 05
  {-420, "UTC-07:00"},  // 06
  {-360, "UTC-06:00"},  // 07
  {-300, "UTC-05:00"},  // 08
  {-240, "UTC-04:00"},  // 09
  {-210, "UTC-03:30"},  // 10
  {-180, "UTC-03:00"},  // 11
  {-120, "UTC-02:00"},  // 12
  { -60, "UTC-01:00"},  // 13
  {   0, "UTC+00:00"},  // 14 default
  { +60, "UTC+01:00"},  // 15
  {+120, "UTC+02:00"},  // 16
  {+180, "UTC+03:00"},  // 17
  {+210, "UTC+03:30"},  // 18
  {+240, "UTC+04:00"},  // 19
  {+270, "UTC+04:30"},  // 20
  {+300, "UTC+05:00"},  // 21
  {+330, "UTC+05:30"},  // 22
  {+345, "UTC+05:45"},  // 23
  {+360, "UTC+06:00"},  // 24
  {+390, "UTC+06:30"},  // 25
  {+420, "UTC+07:00"},  // 26
  {+480, "UTC+08:00"},  // 27
  {+525, "UTC+08:45"},  // 28
  {+540, "UTC+09:00"},  // 29
  {+570, "UTC+09:30"},  // 30
  {+600, "UTC+10:00"},  // 31
  {+630, "UTC+10:30"},  // 32
  {+660, "UTC+11:00"},  // 33
  {+720, "UTC+12:00"},  // 34
  {+765, "UTC+12:45"},  // 35
  {+780, "UTC+13:00"},  // 36
  {+840, "UTC+14:00"}   // 37
};

enum CaptionContent {
  ccNothing,
  ccDefault,
  ccCurrent
};

struct TimerData {
  uint8_t timerIdx;               // 0... (captioned/labelled as 1...)
  char name[3];                   // tn  n=0...4
  char cfgKeyPreset[10];          // TnPreset - used for config key for duration preset
  char cfgKeyBuzzer[10];          // TnBuzzer - used for config key for buzzer duration (secs)
  CaptionContent captionContent;
  char caption[12];               // "tn hh:mm:ss"
  bool timerExpired;
  uint32_t presetDurationSecs;      // the preset value as user defined and saved/loaded
  uint32_t workingPresetSecs;       // the working preset value (set to current duration at unpause/unedit, normally same as presetDurationSecs)
  uint32_t currentDurationSecs;     // the current value (which is primed with the default but might be adjusted up or down during pause (eg:before start))
  uint32_t buzzerDurationSecs;      // max time buzzer should sound (0 = no buzzer)
  uint32_t currentEndTimeSecs;      // the ending time (epoch), of the timer end time (and used to calculate currentDurationSecs)
  uint32_t buzzerEndTimeSecs;       // the ending time (epoch), of the buzzer end time
};

class TimeSource {
public:
  // The global WiFi from WiFi.h must already be .begin()'d before calling TimeSource::begin()
  void begin(RefProvider *refProvider);

  bool loop();
  void processUiEvent(UiEvent highLevelUiEvent);
  void processRemoteButtonClick(bool bLongClick);
  void processWifiStateChange(WifiState newWifiState);

  bool timeValid();
 
  // Calls NTPClient::getEpochTime() or RTC::get() as appropriate
  // This has to be static to pass to TimeLib::setSyncProvider.
  static time_t syncProvider();

  bool getDstActive();
  void setDstActive(bool value);
  uint8_t getUtcOffsetIdx();
  void setUtcOffsetIdx(uint8_t idx);
  const char *getUtcOffsetLabel(uint8_t idx);

  uint16_t getYear()        { return year(timeSnapShot); }
  uint16_t get2DigitYear()  { return getYear() %100; }
  uint8_t getMonth()        { return month(timeSnapShot); }
  uint8_t getDay()          { return day(timeSnapShot); }
  uint8_t getHour()         { return hour(timeSnapShot); }
  uint8_t getMinute()       { return minute(timeSnapShot); }
  uint8_t getSecond()       { return second(timeSnapShot); }

  // Helper functions for displaying time.
  uint8_t getHourTens()     { return getHour()/10; }
  uint8_t getHourUnits()    { return getHour()%10; }
  uint8_t getMinuteTens()   { return getMinute()/10; }
  uint8_t getMinuteUnits()  { return getMinute()%10; }
  uint8_t getSecondTens()   { return getSecond()/10; }
  uint8_t getSecondUnits()  { return getSecond()%10; }

  // Helper functions for displaying date.
  uint8_t getYearTens()     { return get2DigitYear()/10; }
  uint8_t getYearUnits()    { return get2DigitYear()%10; }
  uint8_t getMonthTens()    { return getMonth()/10; }
  uint8_t getMonthUnits()   { return getMonth()%10; }
  uint8_t getDayTens()      { return getDay()/10; }
  uint8_t getDayUnits()     { return getDay()%10; }

  // a quick and dirty offset of the day within year, 0... treats all years as leap years
  // (we don't care really for accuracy, we're only using this for lerping digit colour through year)
  uint16_t getApproxDayOfYearOffset() { return approxDayOffsetsForMonth[getMonth()-1] + getDay() -1; }

  void clearConfig();

  // timer stuff
  const static uint8_t qtyTimers = 5;

  void saveTimeOffsetData();
  void saveDaylightData();    // lat/lon, sun-alt (for calculating daylight hours)

  void saveTimerData(uint8_t timerIdx);

  bool couldShouldCalculateDaylightHours();
  bool haveValidDaylightHours();

//todo: hack
DebugForceDaylight getForceInDaylight();
void setForceInDaylight(DebugForceDaylight value);

  bool getDaylightBoostEnabled();
  void setDaylightBoostEnabled(bool value);
  int16_t getLatitude();
  void setLatitude(int16_t value);
  int16_t getLongitude();
  void setLongitude(int16_t value);
  int8_t getDaylightSunAlt();
  void setDaylightSunAlt(int8_t value);
  bool inDaylight();

  uint16_t getModDaylightBegins();
  uint16_t getModDaylightEnds();

  TimerData &getTimerData(int8_t timerIdx);
  char *getTimerCaption(int8_t timerIdx, CaptionContent captionContent);
  
  void leaveTimerMode();
  void setSelectedTimer(int8_t timerIdx);
  int8_t getSelectedTimer();
  void resetSelectedTimer();
  void primeWorkingDurations(TimerData &timerData);

  bool selectedTimerRunning();
  bool selectedTimerExpired();
  void pauseSelectedTimer(bool bDoPause);

  void startEditing(TimeSourceEditMode newEditMode);
  void stopEditing(bool bAcceptValue);

  void getTimerDigits(uint8_t *hh, uint8_t *mm, uint8_t *ss);
  uint16_t getTimerProgress(uint16_t maxValue);

  void getEditDigits(uint8_t *hi, uint8_t *lo);

private:
  RefProvider *refProvider;

  SunSet sun;

  bool bDisableUI;

  void kickstartNtpClientEtc();

  // config namespaces & keys

  const char *cfgTimeSourceNamespace =              "timeSource";
  const char *cfgTimeSourceUtcOffsetIdx =           "utcOffsetIdx";
  const char *cfgTimeSourceDstActive =              "dstActive";
  // following are "daylight data" for daylight boost
  const char *cfgTimeSourceDaylightBoostEnabled =   "dbEnabled";
  const char *cfgTimeSourceLocationLat =            "locLat";
  const char *cfgTimeSourceLocationLon =            "locLon";
  const char *cfgTimeSourceDaylightSunAlt =         "daySunAlt";
  
  
  // all timer data keys are the timer name + property name, eg: "t0Preset" or "t0Buzzer"

  
  void loadConfig();
  void loadTimeOffsetData();
  void loadDaylightData();      // lat/lon/sun-alt
  void calculateDaylightPeriodOnDayChange();
  void calculateDaylightPeriod();

  void loadTimerData(uint8_t timerIdx);

  uint16_t approxDayOffsetsForMonth[12] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

  uint8_t utcOffsetIdx = UTC_OFFSET_INDEX_DEFAULT;    // default to utc
  bool bDstActive = false;                            // and no DST

  void applyOffsetAndDst();

  // the use of the ambient light detection to brighten/dim the digits & display-panel
  // works very well - *except* that during daylight hours you can get strong side lighting
  // from windows which gives strong reflections on the cloches/bell-jars making the
  // digits difficult to read - and this aspect of the light isn't detected by the ambient
  // light detector (hmm.. maybe I should have added a secondary light detector that has
  // a narrow field of view that you point at the offending window.. then this would've
  // also catered for the weather affecting the strength of the reflections. Oh well, 
  // never mind). The current solution is to calculate some time period we deem as being
  // "daylight hours" and during those hours we boost the brightness of the digits
  // (and maybe display-panel) to try to overcome the reflections. It's ugly.

  // for some reason we need both latitude and longitude (_in addition to_ TZ _and_ the current date) to
  // do sunrise/sunset calcs but the "SunSet" library claims it doesn't matter whether longitude is given
  // as +ve or -ve values which makes me wonder what it does with it
  bool bDaylightBoostEnabled = false;
  int16_t locLat = 0;
  int16_t locLon = 0;
  int8_t sunAlt = 0;    // how far sun is above horizon to consider we are in the daylight section of the day
                        // sure, this will only be rough, as weather and time of year will affect the real strength of the daylight
  
  time_t timeSnapShot = 0;                            // epoch time
  time_t timeSnapShotNoOffset = 0;                    // epoch time - without any offset

  uint8_t daylightDayOfMonth;                         // day of month we last calculated sunset/sunrise
  uint16_t modDaylightBegins;                         // minute-of-day of our definition of beginning of daylight
  uint16_t modDaylightEnds;                           // minute-of-day of our definition of ending of daylight

  // debug hack
  DebugForceDaylight debugForceDaylight = DebugForceDaylight::dfdForceNothing;
  
  bool bHaveDaylightHours = false;
  bool bOldInDaylight = false;

  // timer stuff
  TimerData timersData[qtyTimers];
  int8_t selectedTimerIdx;

  TimeSourceEditMode editMode;
  uint32_t editValueSecs;

  const uint32_t maxOverrunSecs = ((99*60)+1) * 60 - 1;
  
  void forceTimerDisplayMode(bool bPaintDigitSeparator);

  void loopSelectedTimer();

  // Static variables needed for syncProvider()
  static WiFiUDP ntpUDP;
  static NTPClient ntpTimeClient;
  static uint32_t millis_last_ntp;
  const static uint32_t refresh_ntp_every_ms = 3600000; // Get new NTP every hour, use RTC in between.
};



#endif // TIMESOURCE_H
