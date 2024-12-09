#ifndef REF_PROVIDER_H
#define REF_PROVIDER_H

//#include <Preferences.h>

class UI;
class Preferences;
class DigitDisplay;
class DisplayPanel;
class TimeSource;
class Sundries;
class WifiExecutive;

struct RefProvider {
  DigitDisplay &digitDisplay;
  DisplayPanel &displayPanel;
  Preferences &preferences;
  TimeSource &timeSource;
  Sundries &sundries;
  WifiExecutive &wifiExecutive;
  UI *ui;
  RefProvider(
    DigitDisplay &digitDisplay,
    DisplayPanel &displayPanel,
    Preferences &preferences,
    TimeSource &timeSource,
    Sundries &sundries,
    WifiExecutive &wifiExecutive
  ) :
    digitDisplay(digitDisplay),
    displayPanel(displayPanel),
    preferences(preferences),
    timeSource(timeSource),
    sundries(sundries),
    wifiExecutive(wifiExecutive)
    {
  }
};

#endif //REF_PROVIDER_H