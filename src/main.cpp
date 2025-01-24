//MARK 2023-12-06
#include "global_defines.h"

#include <Preferences.h>
#include "DisplayPanel.h"
#include "TimeSource.h"
#include "DigitDisplay.h"
#include "Sundries.h"
#include "WifiExecutive.h"
#include "Ui.h"

#include "RefProvider.h"

// hack for testing
#include "CommandLine.h"
void cli_loop();
void cli_prompt();

Preferences preferences;
DisplayPanel displayPanel;
TimeSource timeSource;
DigitDisplay digitDisplay;
Sundries sundries;
WifiExecutive wifiExecutive;

bool seenWifiConnect = false;


RefProvider refProvider(digitDisplay, displayPanel, preferences, timeSource, sundries, wifiExecutive);

UI ui(refProvider);

//may be of interest: https://oleddisplay.squix.ch/



void setup() {
  Serial.begin(115200);
  delay(500);  // Waiting for serial monitor to catch up.
  Serial.println("");
  Serial.println("In setup()...");

  digitDisplay.begin(&refProvider);

  sundries.begin(&refProvider);
  sundries.setWifiPrioritised(true);

  displayPanel.begin(&refProvider);
  displayPanel.setRotation(2);       // 2=portrait but inverted from normal

  displayPanel.setTextColor(TFT_WHITE, TFT_BLACK);
  displayPanel.setCursor(0, 0, 4);
  displayPanel.println("setup...");


  wifiExecutive.begin(&refProvider);

  timeSource.begin(&refProvider);

  // initialise and start UI
  // Note: this causes all menus to be created and must not be called until we've
  // initialised all the code modules  that each menu is associated with
  ui.begin();
  ui.enable();


  // possibly enter commissioning mode, this may enable rarely used menu items
  // (hold down button while booting)
  delay(100);   // we need this delay otherwise the button debounce code thwarts the button held detection
  sundries.bCommissioningMode = ui.buttonIsPressed();

  // enable and update digitDisplays
  digitDisplay.allowUpdates();

  displayPanel.println("...done");
  displayPanel.println("join wifi...");

}

void maybeCycleDigitDisplayMode(int8_t direction) {
  DigitDisplayMode digitDisplayMode = refProvider.digitDisplay.getActiveDigitDisplayMode();
  // only do digit display mode cycling if we not editing a time value
  // nor are we sitting at an expired timer
  if ( (digitDisplayMode != DigitDisplayModeEdit) && (timeSource.selectedTimerExpired() == false) ) {
    digitDisplay.cycleDigitDisplayMode(direction);
    // tell sundries that the digit separator needs to be redrawn
    refProvider.sundries.redrawDigitSeparator();
  }
}

void loop() {
  UiEvent highLevelUiEvent = ui.loop();

  bool dispModeChanged;

  // deal with consequential ui events...
  if ( highLevelUiEvent != UiEvent::ueNoEvent ) {
    const char *sev;
    switch ( highLevelUiEvent ) {
    case UiEvent::ueUiStarted: 
      sev = "ueUiStarted";
      digitDisplay.setNycMode(false);
      dispModeChanged = digitDisplay.autoSelectDigitDisplayMode();
      sundries.displayDoor_open(true);
      break;
    case UiEvent::ueUiStopped: 
      sev = "ueUiStopped";
      digitDisplay.setNycMode(false);
      if ( timeSource.getSelectedTimer() == -1 ) {
        dispModeChanged = digitDisplay.autoSelectDigitDisplayMode();
      }
      sundries.displayDoor_shut(true);
      break;
    case UiEvent::ueCancel: 
      sev = "ueCancel";
      //Serial.printf("main.loop() ueCancel, refProvider.timeSource.getSelectedTimer()=%d, digitDisplay.getActiveDigitDisplayMode()=%d\n", refProvider.timeSource.getSelectedTimer(), digitDisplay.getActiveDigitDisplayMode());
      dispModeChanged = digitDisplay.autoSelectDigitDisplayMode();
      //Serial.printf("...dispModeChanged=%d\n", dispModeChanged);
      if ( dispModeChanged ) {
        // tell sundries that the digit separator needs to be redrawn
        refProvider.sundries.redrawDigitSeparator();
      }
      break;
    case UiEvent::ueDecrease: 
      sev = "ueDecrease";
      maybeCycleDigitDisplayMode(-1);
      break;
    case UiEvent::ueIncrease: 
      sev = "ueIncrease";
      maybeCycleDigitDisplayMode(+1);
      break;
    case UiEvent::ueAltDecrease: 
      sev = "ueAltDecrease";
      break;
    case UiEvent::ueAltIncrease: 
      sev = "ueAltIncrease";
      break;
    case UiEvent::ueAccept: 
      sev = "ueAccept";
      break;
    case UiEvent::ueAltModeBegin: 
      sev = "ueAltModeBegin";
      break;
    case UiEvent::ueAltModeEnd: 
      sev = "ueAltModeEnd";
      break;
    case UiEvent::ueMenuActivating: 
      sev = "ueMenuActivating";
      break;
    case UiEvent::ueMenuDeactivating: 
      sev = "ueMenuDeactivating";
      break;
    }
    // Serial.print("UI.highLevelEvent=");
    // Serial.println(sev);

    // pass any UiEvent to the time source
    timeSource.processUiEvent(highLevelUiEvent);

  }

  //2023-11-26: attempting new wifi
  wifiExecutive.loop();

  WifiState newWifiState = wifiExecutive.getNewWifiState();
  if ( newWifiState != WS_null ) {
    Serial.printf("main-loop: newWifiState:%s\n", wifiExecutive.stateName(newWifiState));

    // pass any wifi-state-change to time source
    timeSource.processWifiStateChange(newWifiState);

    // if noticing wifi connect 1st time then try shut display door
    if ( !seenWifiConnect ) {
      if ( newWifiState == WS_connected ) {
        seenWifiConnect = true;
        Serial.printf("ui.getUiState()=%d\n", ui.getUiState());
        if ( ui.getUiState() != UiState::usActive ) {
          displayPanel.println("joined");
          sundries.displayDoor_shut(true);
        }
      }
    }

  }

  // tick the time source
  bool bDisableUI = timeSource.loop();

  if ( bDisableUI ) {
    ui.disable();
  } else {
    //Serial.println("re-enable UI");
    ui.enable();
  }

  // Update the time display (if enabled)
  digitDisplay.loop();
  displayPanel.loop();

  sundries.loop();

  // (FastLED fucks around with disabling interrupts and causes issues for wifi traffic)
  // this kludgy if/else is an attempt to have wifi work more reliably
  // while trying to connect/getting NTP time (at least the 1st time,
  // I'm not sure if/how the Time.Status changes when a resync is attempted)
  // If we have connection & timeset then allow FastLED to fuck things up
  // a lot more than the minimum, otherwise keep the fucking up to a
  // minimum in the hope that wif gets a better chance at doing its stuff)
  if ( sundries.getWifiPrioritised() ) {
    // if we don't have this then we don't see any digit updates (on loss/forget of wifi)
    FastLED.show();
    delay(1000 / UPDATES_PER_SECOND);
    //Serial.println("Avoiding FastLED.delay(), wifi priroritised");
  } else {
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }
  

  // deal with any serial cli stuff
  cli_loop();

}


// ************************************************************
// random setup / config and serial cli debug
// ************************************************************

void cli_prompt() {
  Serial.print(">");
}

// hacky test code to allow me to poke around
// (based on old code of mine that I used to set config and do other commands etc)

// ************************************************************
// config set/show helper for cli
// ************************************************************
void cli_showValue(String name, String sValue) {
  Serial.println(String(name) + ": [" + sValue + "]");
}

void cli_wifi_show_settings() {
  cli_showValue("hostname",  wifiExecutive.getWifiHostname());
  cli_showValue("ssid",  wifiExecutive.getWifiSSID());
  cli_showValue("pwd",  wifiExecutive.getWifiPWD());
}

void cli_wifi_print_status(Print& printDest) {
  printDest.println("wifi up:" + String(wifiExecutive.connected()));
  printDest.println("wifi state:" + String(wifiExecutive.stateName(wifiExecutive.getWifiState())));
}


void cli_loop() {
  bool bGotLine = getCommandLineFromSerialPort();
  if (bGotLine) {
    Serial.println();
    String cmd = getFirstWord();
    String parm1;
    String parm2;
    String parm3;

    if ( cmd.equalsIgnoreCase("rst") ) {
      esp_restart();
    } else if ( cmd.equalsIgnoreCase("ui") ) {
      parm1 = getNextWord();
      if ( parm1.equalsIgnoreCase("to") ) {
        parm2 = getNextWord();
        if ( !parm2.isEmpty() ) {
          uint16_t newTimeoutPeriodMS = atoi(parm2.c_str());
          ui.setUiTimeoutPeriodMS(newTimeoutPeriodMS);
        }
        Serial.printf("timeoutPeriodMS = %d\n", ui.getUiTimeoutPeriodMS());
      }
    } else if ( cmd.equalsIgnoreCase("buz") ) {
      parm1 = getNextWord();
      if ( parm1.equalsIgnoreCase("on") ) {
        sundries.buzzer_start();
      } else {
        sundries.buzzer_stop();
      }
    } else if ( cmd.equalsIgnoreCase("door") ) {
      parm1 = getNextWord();
      if ( !parm1.isEmpty() ) {
        sundries.displayDoor_setPresent(atoi(parm1.c_str()));
      }
      if ( sundries.displayDoor_getPresent() ) {
        Serial.println("door present");
      } else {
        Serial.println("door not present");
      }
    } else if ( cmd.equalsIgnoreCase("lm") ) {
      parm1 = getNextWord();
      if ( parm1.equalsIgnoreCase("mode") ) {
        parm2 = getNextWord();
        if ( !parm2.isEmpty() ) {
          BH1750::Mode newMode = (BH1750::Mode)atoi(parm2.c_str());
          sundries.lightMeter_setMode(newMode);
        }
        Serial.printf("mode = 0x%02X\n", sundries.lightMeter_getMode());
      } else if ( parm1.equalsIgnoreCase("mt") ) {
        parm2 = getNextWord();
        if ( !parm2.isEmpty() ) {
          uint8_t newMT = atoi(parm2.c_str());
          sundries.lightMeter_setMtRegVal(newMT);
        }
        Serial.printf("mt = %d\n", sundries.lightMeter_getMtRegVal());
      }
    } else if ( cmd.equalsIgnoreCase("tt") ) {
      parm1 = getNextWord();
      if ( parm1.equalsIgnoreCase("tz") ) {
        parm2 = getNextWord();
        if ( !parm2.isEmpty() ) {
          uint8_t newTzIdx = atoi(parm2.c_str());
          timeSource.setUtcOffsetIdx(newTzIdx);
        }
        uint8_t tzIdx = timeSource.getUtcOffsetIdx();
        Serial.printf("TZ-idx[%d]: %s\n", tzIdx, timeSource.getUtcOffsetLabel(tzIdx));
      } else if ( parm1.equalsIgnoreCase("dst") ) {
        parm2 = getNextWord();
        if ( !parm2.isEmpty() ) {
          bool newDst = atoi(parm2.c_str());
          timeSource.setDstActive(newDst);
        }
        bool dst = timeSource.getUtcOffsetIdx();
        Serial.printf("DST:%d:\n", dst);
      } else if ( parm1.equalsIgnoreCase("day") ) {
        parm2 = getNextWord();
        DebugForceDaylight debugForceDaylight;
        if ( !parm2.isEmpty() ) {
          debugForceDaylight = static_cast<DebugForceDaylight>(atoi(parm2.c_str()));
          timeSource.setForceInDaylight(debugForceDaylight);
        }
        debugForceDaylight = timeSource.getForceInDaylight();
        Serial.printf("debugForceDaylight:%d:\n", debugForceDaylight);
      } else if ( parm1.equalsIgnoreCase("reset") ) {        
        timeSource.clearConfig();
        Serial.println("timeSource config cleared");
      }
    } else if ( cmd.equalsIgnoreCase("dd") ) {
      parm1 = getNextWord();
      if ( parm1.equalsIgnoreCase("cc") ) {
        parm2 = getNextWord();
        if ( !parm2.isEmpty() ) {
          if ( parm2.equalsIgnoreCase("reset") ) {
            digitDisplay.setRgbCorrection(RGB_CORRECTION, true);
          } else {
            // expect parm2 = eg: FF80EE
            uint32_t rgbCorrectionVal = strtoul(parm2.c_str(), NULL, 16);
            digitDisplay.setRgbCorrection(rgbCorrectionVal, true);
          }
        }
      } else if ( parm1.equalsIgnoreCase("reset") ) {
        digitDisplay.clearConfig();
        Serial.println("digitDisplay config cleared");
      }
    } else if ( cmd.equalsIgnoreCase("wifi") ) {
      parm1 = getNextWord();
      if ( parm1.equalsIgnoreCase("hostname") ) {
        parm2 = getRemainder();
        if ( parm2 != "" ) {
          wifiExecutive.setWifiHostname(parm2);
        }
      } else if ( parm1.equalsIgnoreCase("ssid") ) {
        parm2 = getRemainder();
        if ( parm2 != "" ) {
          wifiExecutive.setWifiSSID(parm2);
        }
      } else if ( parm1.equalsIgnoreCase("pwd") ) {
        parm2 = getRemainder();
        if ( parm2 != "" ) {
          wifiExecutive.setWifiPWD(parm2);
        }
      // } else if ( parm1.equalsIgnoreCase("start") ) {
      //   wifi_start(Serial, wifi_start_timeout_secs);
      // } else if ( parm1.equalsIgnoreCase("stop") ) {
      //   wifi_stop(Serial);
      } else if ( parm1.equalsIgnoreCase("forget") ) {
        wifiExecutive.forgetCredentials();
      } else if ( parm1.equalsIgnoreCase("reset") ) {
        wifiExecutive.clear_prefs();
      } else if ( parm1.equalsIgnoreCase("diag") ) {        
        WiFi.printDiag(Serial);
      } 
      cli_wifi_show_settings();
      cli_wifi_print_status(Serial);

    }

    cli_prompt();
  }

}


