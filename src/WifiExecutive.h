#ifndef WIFIEXECUTIVE_H
#define WIFIEXECUTIVE_H

#include "global_defines.h"

//#include "esp_now.h"
//#include "EspNowProto.h"

struct RefProvider;

enum WifiState {
  WS_null,
  WS_no_credentials,      //red flashing
  WS_disconnected,        //red
  WS_connecting,          //yellow
  WS_timed_out,           //amber
  WS_connected            //white
};


class WifiExecutive {
public:
  void begin(RefProvider *refProvider);


  bool connected();
  WifiState getWifiState();
  WifiState getNewWifiState();
  const char *stateName(WifiState state);

  bool haveCredentials();
  bool haveSSID();
  bool havePWD();
  
  void forgetCredentials();

  String getWifiHostname();
  void setWifiHostname(String value);
  String getWifiSSID();
  void setWifiSSID(String value);
  String getWifiPWD();
  void setWifiPWD(String value);
  void clear_prefs();

  void loop();

private:
  RefProvider *refProvider;

  WifiState wifiState = WS_null;
  WifiState newWifiState = WS_null;
  

  void newState(WifiState newWifiState);

  uint8_t connectAttempts;

  String client_hostname;
  String client_ssid;
  String client_pwd;

  const String default_hostname = "ledClock";
  const String default_SSID = "-- n/w undefined --";
  const String default_PWD = "-- pwd undefined --";
  
  // ************************************************************
  // pref stuff
  // ************************************************************
  const char *pref_namespace = "wifi";
  const char *pref_key_hostname = "hostname";
  const char *pref_key_ssid = "ssid";
  const char *pref_key_pwd = "pwd";

  void read_prefs();
  void write_prefs();

  void checkCredentials();
  void checkWifiState();
  void connect();

  void onConnect();
  void onDisconnect();

  // ********************************************************************************************************************************
  // ********************************************************************************************************************************
  // ********************************************************************************************************************************
  const uint16_t connect_TimeoutPeriodMS = 3000;
  uint32_t connect_timeoutTimeMS = 0;
  const uint16_t retry_TimeoutPeriodMS = 10000;
  uint32_t retry_timeoutTimeMS = 0;

  // // ********************************************************************************************************************************
  // // ********************************************************************************************************************************

};

// 
// enum ListenerState {
//   lsAbsent,
//   lsAwaitingAck,
//   lsRegistered
// };
// 
// struct ListenerData {
//   ListenerState state;
//   esp_now_peer_info_t peerInfo;
//   uint32_t ackTimeoutTime_MS;
// //  uint8_t listenerAddr[6];
//   ESPNOW_RemoteToMain lastRxData;
//   ESPNOW_MainToRemote lastTxData;
// };
// 
// 
// // non-instance stuff for wifi-now callback access etc
// //todo: potentially could have multiple listeners
// //      currently just one:
// static ListenerData listenerData;
// static uint8_t lastRxMacAddr[6];
// static uint lastRxLength;
// static ESPNOW_RemoteToMain lastRxData;
// 
// //static bool bHaveListener = false;
// //static bool bEspNowInited = false;
// const uint16_t listenerTimoutPeriod_MS = 100;
// static void listenerInit();
// static void initListenerData();
// static void listenerSend();
// static void listenerLoop(uint32_t nowMS);
// static void setListenerState(ListenerState newState);
// static void rememberListener();
// static void forgetListener();
// static void onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
// static void onSend(const uint8_t *mac_addr, esp_now_send_status_t status);
// static const char *getListenerStateName(ListenerState state);

#endif // WIFIEXECUTIVE_H
