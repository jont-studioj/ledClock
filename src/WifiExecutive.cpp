#include "WifiExecutive.h"
#include "RefProvider.h"
#include <Preferences.h>
#include "Sundries.h"

#include <WiFi.h>
#include <esp_wifi.h>

// // for esp_now satellite (remote listener)
// #include "espSatellite.h"

void WifiExecutive::begin(RefProvider *refProvider) {
  this->refProvider = refProvider;
  
//todo: make use of this, some day
  connectAttempts = 0;

  read_prefs();

  newState(WS_disconnected);
  checkCredentials();

  WiFi.setHostname(client_hostname.c_str());

  WiFi.mode(WIFI_STA);
  //WiFi.mode(WIFI_AP_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);

  // to enable esp-now comms
  WiFi.setSleep(false);

  WiFi.printDiag(Serial);
  Serial.println(WiFi.macAddress());

  // // now we have inited Wifi...
  // // enable/initilize ESP-NOW / listener stuff
  refProvider->sundries.startEspNow();

}

bool WifiExecutive::connected() {
  return wifiState == WS_connected;
}

WifiState WifiExecutive::getWifiState() {
  checkWifiState();
  return wifiState;
}

WifiState WifiExecutive::getNewWifiState() {
  WifiState retVal = newWifiState;
  newWifiState = WS_null;
  return retVal;
}

String WifiExecutive::getWifiHostname() {
  return client_hostname;
}
void WifiExecutive::setWifiHostname(String value) {
  value.trim();
  if ( value.length() == 0 ) {
    value = default_hostname;
  }
  if ( client_hostname != value ) {
    client_hostname = value;
    write_prefs();
    // the following might not work have the full effect next reboot
    // but at least the value should be available to the getter
    WiFi.setHostname(client_hostname.c_str());
  }
}
String WifiExecutive::getWifiSSID() {
  return client_ssid;
}
void WifiExecutive::setWifiSSID(String value) {
  value.trim();
  if ( value.length() == 0 ) {
    value = default_SSID;
  }
  if ( client_ssid != value ) {
    client_ssid = value;
    write_prefs();
    checkCredentials();
  }
}
String WifiExecutive::getWifiPWD() {
  return client_pwd;
}
void WifiExecutive::setWifiPWD(String value) {
  value.trim();
  if ( value.length() == 0 ) {
    value = default_PWD;
  }
  if ( client_pwd != value ) {
    client_pwd = value;
    write_prefs();
    checkCredentials();
  }
}

void WifiExecutive::loop() {
  uint32_t nowMS = millis();

  checkWifiState();

  switch (wifiState) {
  case WS_no_credentials:
    //Serial.println("WifiExecutive::loop(): WS_no_credentials");
    if ( haveCredentials() ) {
      newState(WS_disconnected);
    }
    break;
  case WS_disconnected:
    //Serial.println("WifiExecutive::loop(): WS_disconnected");
    connect();
    break;
  case WS_connected:
    //Serial.println("WifiExecutive::loop(): WS_connected");
    // nothing to do
    break;
  case WS_timed_out:
    //Serial.println("WifiExecutive::loop(): WS_timed_out");
    //TODO: maybe we need max retry?
    if ( (retry_timeoutTimeMS != 0) && (nowMS > retry_timeoutTimeMS) ) {
      retry_timeoutTimeMS = 0;
      newState(WS_disconnected);
    }
    break;
  case WS_connecting:
    //Serial.println("WifiExecutive::loop(): WS_connecting");
    if ( (connect_timeoutTimeMS != 0) && (nowMS > connect_timeoutTimeMS) ) {
      connect_timeoutTimeMS = 0;
      retry_timeoutTimeMS = nowMS + retry_TimeoutPeriodMS;
      newState(WS_timed_out);
    }
    break;
  }

  // // service (static) listener stuff
  // ES_listenerLoop(nowMS);
}


void WifiExecutive::read_prefs() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(pref_namespace, true);
  client_hostname = preferences.getString(pref_key_hostname, default_hostname);
  client_ssid = preferences.getString(pref_key_ssid, default_SSID);
  client_pwd = preferences.getString(pref_key_pwd, default_PWD);
  preferences.end();
}
void WifiExecutive::write_prefs() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(pref_namespace, false);
  preferences.putString(pref_key_hostname, client_hostname);
  preferences.putString(pref_key_ssid, client_ssid);
  preferences.putString(pref_key_pwd, client_pwd);
  preferences.end();    
}
void WifiExecutive::clear_prefs() {
  Preferences &preferences = refProvider->preferences;
  preferences.begin(pref_namespace, false);
  preferences.clear();
  preferences.end();
}

bool WifiExecutive::haveCredentials() {
  return haveSSID() && havePWD();
}
bool WifiExecutive::haveSSID() {
  return ( client_ssid != default_SSID);
}
bool WifiExecutive::havePWD() {
  return ( client_pwd != default_PWD );
}


void WifiExecutive::forgetCredentials() {
  
  client_ssid = default_SSID;
  client_pwd = default_PWD;
  write_prefs();

  // disconnect any current wifi connection
  WiFi.disconnect();
//WiFi.eraseAP();

  // update state
  checkWifiState();
}


void WifiExecutive::checkCredentials() {
  if ( !haveCredentials() ) {
    newState(WS_no_credentials);
  }
}

void WifiExecutive::checkWifiState() {
  checkCredentials();
  if ( haveCredentials() ) {
//todo: do I really want this IF? 
//If I am in connecting state then any call here will never get us out of that state
//      if ( wifiState != WS_connecting ) {
        wl_status_t wlStatus = WiFi.status();
//
        if ( wlStatus == WL_CONNECTED ) {
if ( wifiState != WS_connected ) {
  Serial.printf("WifiExecutive::checkWifiState(): wlStatus=%d\n", wlStatus);
}
          newState(WS_connected);
        } else {
// //this if is dodgy
//           if ( connectAttempts == 0 ) {
//             newState(WS_disconnected);
//           } else {
//             newState(WS_timed_out);
//           }
        }
//      }//endif currently attemping to connect
  }//endif/else we have credentials or not
}

void WifiExecutive::connect() {
  Serial.printf("%d WifiExecutive::connect()\n", millis());
  WiFi.begin(client_ssid.c_str(), client_pwd.c_str());
  connect_timeoutTimeMS = millis() + connect_TimeoutPeriodMS;
  newState(WS_connecting);
}

// ********************************************************************************************************************************
// ********************************************************************************************************************************
// ********************************************************************************************************************************

void WifiExecutive::newState(WifiState toWifiState) {
  if ( toWifiState != wifiState ) {
    Serial.printf("%d WifiExecutive: %s --> %s\n", millis(), stateName(wifiState) , stateName(toWifiState) );
    wifiState = toWifiState;
    newWifiState = wifiState;

    if ( wifiState == WS_connected ) {
      onConnect();
    } if ( wifiState == WS_disconnected ) {
      onDisconnect();
    }
  }
}

const char *WifiExecutive::stateName(WifiState state) {
  const char *name;
  switch (state) {
  case WS_null:
    name = "<null>";
    break;
  case WS_no_credentials:
    name = "no cred's";
    break;
  case WS_disconnected:
    name = "disc.";
    break;
  case WS_connected:
    name = "connected";
    break;
  case WS_timed_out:
    name = "timed out";
    break;
  case WS_connecting:
    name = "connect...";
    break;
  }
  return name;
}


void WifiExecutive::onConnect() {
//   Serial.println("WifiExecutive::onConnect()");
//   forgetListener();
//   const char *msg = "";
// 
}
void WifiExecutive::onDisconnect() {
  // Serial.println("WifiExecutive::onDisconnect()");
}



// ********************************************************************************************************************************
// ********************************************************************************************************************************
// ********************************************************************************************************************************

/*
for reference:

wl_status_t WiFiSTAClass::begin(const char* wpa2_ssid, wpa2_auth_method_t method, const char* wpa2_identity, const char* wpa2_username, const char *wpa2_password, const char* ca_pem, const char* client_crt, const char* client_key, int32_t channel, const uint8_t* bssid, bool connect)
{
    if(!WiFi.enableSTA(true)) {
        log_e("STA enable failed!");
        return WL_CONNECT_FAILED;
    }

    if(!wpa2_ssid || *wpa2_ssid == 0x00 || strlen(wpa2_ssid) > 32) {
        log_e("SSID too long or missing!");
        return WL_CONNECT_FAILED;
    }

    if(wpa2_identity && strlen(wpa2_identity) > 64) {
        log_e("identity too long!");
        return WL_CONNECT_FAILED;
    }

    if(wpa2_username && strlen(wpa2_username) > 64) {
        log_e("username too long!");
        return WL_CONNECT_FAILED;
    }

    if(wpa2_password && strlen(wpa2_password) > 64) {
        log_e("password too long!");
    }

*/

