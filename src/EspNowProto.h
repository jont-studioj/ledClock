#ifndef ESPNOW_PROTO_H
#define ESPNOW_PROTO_H


enum ESPNOW_command {
  ENC_nop,                      // 0
  ENC_connect_request,          // 1 remote --> main
  ENC_connect_accept,           // 2 remote <-- main
  ENC_channel_redirect,         // 3 remote <-- main
  ENC_keep_alive,               // 4 remote --> main
  ENC_keep_alive_ack,           // 5 remote <-- main
  ENC_state_data,               // 6 remote <-- main
  ENC_state_data_ack,           // 7 remote --> main
  ENC_button_press_short,       // 8 remote --> main
  ENC_button_press_long         // 9 remote --> main
};

struct ESPNOW_RemoteToMain {
  uint8_t wifiChannel;
  ESPNOW_command command;
  char targetName[32];
};

struct ESPNOW_MainToRemote {
  uint8_t wifiChannel;
  ESPNOW_command command;
  bool timerRunning;
  uint8_t timerRemainPer240;
  bool flashActive;
  bool buzzActive;
  uint8_t hueValue;
  uint8_t satValue;
};

// interval between keepAlive messages 
//  - the keepAlive sender uses this interval between keepAlive message Tx
//  - the receiver allows for twice this value between keepAlive message Rx
const uint32_t keepAliveIntervalMS = 10000;   

const uint32_t minSendIntervalMS = 100;       // minimum interval between sends (applies to unsolicited sends only)


#endif // ESPNOW_PROTO_H