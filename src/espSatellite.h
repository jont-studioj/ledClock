#ifndef ESPNOW_SATELLITE_H
#define ESPNOW_SATELLITE_H

//#include "global_defines.h"

#include <WiFi.h>

//#include "espSatellite.h"
#include "esp_now.h"
#include "EspNowProto.h"

enum ES_ListenerState {
  eslsAbsent,
  eslsAwaitingAck,
  eslsRegisteredIdle
};


struct ES_ListenerData {
  ES_ListenerState state;
  esp_now_peer_info_t peerInfo;
  uint32_t ackTimeoutTime_MS;
  uint8_t sendRetriesLeft;
  uint32_t keepAliveTimeoutTimeMS;
  uint32_t nextAllowableSendTimeMS;
  ESPNOW_RemoteToMain rxData;
  ESPNOW_MainToRemote txData;
};

// non-instance stuff for wifi-now callback access etc
//todo: potentially could have multiple listeners
//      currently just one:
static ES_ListenerData ES_listenerData;
static uint8_t ES_lastRxMacAddr[6];
static uint ES_lastRxLength;
//static ESPNOW_RemoteToMain ES_lastRxData;

static bool ES_stateUpdated;
//static ESPNOW_MainToRemote pendingStateData;

static ESPNOW_command ES_rxCommand;

//static bool bHaveListener = false;
static bool bEspNowInited = false;
const uint16_t ES_listenerAckTimoutPeriod_MS = 50;
const uint16_t ES_listenerSendRetryCount = 10;
const uint16_t ES_listenerKeepAliveTimoutPeriod_MS = keepAliveIntervalMS * 2;
static void ES_listenerInit();
static void ES_espNowStart();
static void ES_espNowStop();
static void ES_initListenerData();
static void ES_reprimeKeepAliveTimer();
static void ES_listenerPrimeSend(ESPNOW_command command, bool reprimeRetries);
static void ES_listenerSendLoop();
static void ES_listenerDoSend();
static void ES_listenerLoop(uint32_t nowMS);
static void ES_setListenerState(ES_ListenerState newState);
static void ES_rememberListener();
static void ES_forgetListener();
static void ES_onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
static void ES_onSend(const uint8_t *mac_addr, esp_now_send_status_t status);
static const char *ES_getListenerStateName(ES_ListenerState state);
static ESPNOW_command ES_getRxCommand();

static bool ES_haveListenerReady() {
  return (ES_listenerData.state == eslsRegisteredIdle);
}

static void ES_setWifiChannel(uint8_t value) {
  if ( value != ES_listenerData.txData.wifiChannel ) {
    ES_listenerData.txData.wifiChannel = value;
    ES_stateUpdated = true;
  }
}
static void ES_setTimerRunning(bool value) {
  if ( value != ES_listenerData.txData.timerRunning ) {
    ES_listenerData.txData.timerRunning = value;
    ES_stateUpdated = true;
  }
}
static void ES_setTimerRemainPer240(uint8_t value) {
  if ( value != ES_listenerData.txData.timerRemainPer240 ) {
    ES_listenerData.txData.timerRemainPer240 = value;
    ES_stateUpdated = true;
  }
}
static void ES_setFlashActive(bool value) {
  if ( value != ES_listenerData.txData.flashActive ) {
    ES_listenerData.txData.flashActive = value;
    ES_stateUpdated = true;
  }
}
static void ES_setBuzzActive(bool value) {
  if ( value != ES_listenerData.txData.buzzActive ) {
    ES_listenerData.txData.buzzActive = value;
    ES_stateUpdated = true;
  }
}
static void ES_setHueValue(uint8_t value) {
  if ( value != ES_listenerData.txData.hueValue ) {
    ES_listenerData.txData.hueValue = value;
    ES_stateUpdated = true;
  }
}
static void ES_setSatValue(uint8_t value) {
  if ( value != ES_listenerData.txData.satValue ) {
    ES_listenerData.txData.satValue = value;
    ES_stateUpdated = true;
  }
}

void ES_listenerInit() {
  bEspNowInited = false;
  ES_initListenerData();

  // init pending outgoing state data
  // this will get updated by the rest of the system
  // and holds onto the current state for forwarding to a remote/satellite
  ES_stateUpdated = false;
  ES_listenerData.txData.command = ENC_nop;
  ES_listenerData.txData.timerRemainPer240 = 0;
  ES_listenerData.txData.flashActive = false;
  ES_listenerData.txData.buzzActive = false;
  ES_listenerData.txData.hueValue = 0;
  ES_listenerData.txData.satValue = 0;
}

void ES_espNowStart() {

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  } else {
    bEspNowInited = true;
    if (esp_now_register_recv_cb(ES_onRecv) != ESP_OK) {
      Serial.println("Error registering recv_cb");
    }
    if (esp_now_register_send_cb(ES_onSend) != ESP_OK) {
      Serial.println("Error registering send_cb");
    }
  }

}
void ES_espNowStop() {
  if ( bEspNowInited ) {
    if (esp_now_unregister_recv_cb() != ESP_OK) {
      Serial.println("Error unregistering recv_cb");
    }
    if (esp_now_unregister_send_cb() != ESP_OK) {
      Serial.println("Error unregistering send_cb");
    }
    if ( esp_now_deinit() != ESP_OK) {
      Serial.println("Error de-initializing ESP-NOW");
    } else {
      bEspNowInited = false;
    }
  }
}


void ES_initListenerData() {
  uint8_t nullAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  ES_setListenerState(eslsAbsent);
  ES_listenerData.ackTimeoutTime_MS = 0;
  ES_listenerData.sendRetriesLeft = ES_listenerSendRetryCount;
  ES_listenerData.keepAliveTimeoutTimeMS = 0;
  ES_listenerData.nextAllowableSendTimeMS = 0;
  ES_listenerData.rxData.command = ENC_nop;
  ES_listenerData.txData.command = ENC_nop;

  memcpy(ES_listenerData.peerInfo.peer_addr, nullAddr, 6);
  ES_listenerData.peerInfo.channel = 0;
  ES_listenerData.peerInfo.encrypt = false;

  ES_lastRxLength = 0;

  ES_rxCommand = ENC_nop;

  ES_stateUpdated = false;

}

void ES_reprimeKeepAliveTimer() {
  // our keepAlive timeout is twice that of the expected send interval
  ES_listenerData.keepAliveTimeoutTimeMS = millis() + keepAliveIntervalMS * 2;
}

void ES_onRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len) {
  // only take this if we are expecting data, else we will overwrite our previous rxData
  if ( ES_lastRxLength == 0 ) {
    ES_lastRxLength = data_len;

    // remember sender's mac_addr
    memcpy(ES_lastRxMacAddr, mac_addr, 6);

    // save message body
    memcpy(&ES_listenerData.rxData, incomingData, sizeof(ESPNOW_RemoteToMain));

    
    // Serial.printf("ES_onRecv(): mac:%02X:%02X:%02X:%02X:%02X:%02X cmd=%d, targetName[%s]\n", 
    //   (int)(mac_addr[0]), (int)(mac_addr[1]), (int)(mac_addr[2]), (int)(mac_addr[3]), (int)(mac_addr[4]), (int)(mac_addr[5]),
    //   ES_listenerData.rxData.command, ES_listenerData.rxData.targetName
    // );

  }//endif rxLength was 0
}

void ES_onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.printf("ES_onSend(): mac:%02X:%02X:%02X:%02X:%02X:%02X status=%d\n", 
  //   (int)(mac_addr[0]), (int)(mac_addr[1]), (int)(mac_addr[2]), (int)(mac_addr[3]), (int)(mac_addr[4]), (int)(mac_addr[5]),
  //   status
  // );
}

void ES_listenerPrimeSend(ESPNOW_command command, bool reprimeRetries) {
  ES_listenerData.txData.command = command;
  if ( reprimeRetries ) {
    ES_listenerData.sendRetriesLeft = ES_listenerSendRetryCount;
  }
}

void ES_listenerSendLoop() {
  if ( ES_listenerData.txData.command != ENC_nop ) {
    // there is something to send, are we allowed to send it yet?
    uint32_t msNow = millis();
    if ( msNow > ES_listenerData.nextAllowableSendTimeMS ) {
      ES_listenerDoSend();
    } else {
      Serial.printf("too soon to send, need to wait:%dms\n", ES_listenerData.nextAllowableSendTimeMS - msNow);
    }//endif allowed to send
  }//endif something to send
}

void ES_listenerDoSend() {
  if ( ES_listenerData.txData.command != ENC_nop ) {
    esp_err_t result = esp_now_send(ES_listenerData.peerInfo.peer_addr, (uint8_t *) &ES_listenerData.txData, sizeof(ESPNOW_MainToRemote));
    if ( result == ESP_OK ) {
      //Serial.printf("ES_listenerDoSend() cmd=%d\n", ES_listenerData.txData.command);
      if ( ES_listenerData.txData.command == ENC_state_data ) {
        ES_setListenerState(eslsAwaitingAck);
        ES_listenerData.ackTimeoutTime_MS = millis() + ES_listenerAckTimoutPeriod_MS;
      }
    } else {
      Serial.printf("esp_now_send(cmd=%d) failed\n", ES_listenerData.txData.command);
    }
    ES_lastRxLength = 0;
    ES_listenerData.txData.command = ENC_nop;
    ES_listenerData.nextAllowableSendTimeMS = millis() + minSendIntervalMS;
  }//endif something to send
}

void ES_listenerLoop(uint32_t nowMS) {

  if ( ES_lastRxLength == 0 ) {
    // if timeout is set then we must be awaiting an ack for something
    if ( ES_listenerData.ackTimeoutTime_MS != 0 ) {
      if ( nowMS > ES_listenerData.ackTimeoutTime_MS ) {
        ES_listenerData.ackTimeoutTime_MS = 0;
        // listener timeout, retry so many times then give up
        if ( ES_listenerData.sendRetriesLeft > 0 ) {
Serial.printf("ES_listenerLoop() timeout, sendRetriesLeft=%d\n", ES_listenerData.sendRetriesLeft);
          ES_listenerData.sendRetriesLeft--;
          // (re)send whatever data we currently have (it's the latest)
          Serial.printf("Retrying send, retries left=%d\n", ES_listenerData.sendRetriesLeft);
          ES_listenerPrimeSend(ENC_state_data, false);
//I think this is nopped out because we can instead rely on missing keepAlives to sever connection
//         } else {
//           // exceeded retries, give up
// Serial.println("ES_listenerLoop() timeout, retries exceeded, Dropping listener\n");
//           ES_forgetListener();
        }
      }
    } else {
      // not waiting for an ack, see if we're missing a keepAlive, if so then forget listener
      if ( ES_listenerData.keepAliveTimeoutTimeMS != 0 ) {
        if (nowMS > ES_listenerData.keepAliveTimeoutTimeMS ) {
          Serial.println("Didn't get a keepAlive recently, forgetting listener");
          ES_forgetListener();
        }
      }
      // see if we need to send state data
      // (only if we're not waiting for an ack)
      if (ES_listenerData.state == eslsRegisteredIdle) {
        if ( ES_stateUpdated ) {
          ES_listenerPrimeSend(ENC_state_data, true);
          ES_stateUpdated = false;
        }
      }
    }
  } else {
    // we have some data, see if it's connect request, an ack, keepAlive, or button data
    ESPNOW_RemoteToMain &rxData = ES_listenerData.rxData;
    ESPNOW_MainToRemote &txData = ES_listenerData.txData;

    ES_lastRxLength = 0;

    if ( rxData.command == ENC_connect_request ) {
      // accept if we are not already connected
      // and if requested target is us
      if ( strcmp(rxData.targetName, WiFi.getHostname()) == 0 ) {
        // target matches me...

        // check to see if caller is on the channel I am using
        // if not then issue a channel redirect else accept connection
        uint8_t myWifiChannel = WiFi.channel();
        bool bOnMyChannel = (rxData.wifiChannel == myWifiChannel);
        if ( !bOnMyChannel ) {
          Serial.printf("connect_request via channel:%d, while I'm on:%d, redirecting...\n", rxData.wifiChannel, myWifiChannel);
          // tell the remote the actual wifi channel I am on
          txData.wifiChannel = myWifiChannel;
          ES_listenerPrimeSend(ENC_channel_redirect, true);
// txData.command = ENC_channel_redirect;
// ES_listenerDoSend();
        } else {
          // caller is coming in on my channel, accept connection
          ES_rememberListener();
//txData.command = ENC_connect_accept;
          // tell the remote the actual wifi channel I am on
          txData.wifiChannel = myWifiChannel;
//ES_listenerDoSend();
          ES_listenerPrimeSend(ENC_connect_accept, true);
          ES_setListenerState(eslsRegisteredIdle);
          ES_reprimeKeepAliveTimer();
          // trigger initial state Tx
          ES_stateUpdated = true;
        }
      }//else just ignore
    } else {
//Serial.printf("not connect request... cmd=%d, state=%d\n", rxData.command, ES_listenerData.state);
      // not connect request...
      // anything else must match our existing listener (and we must be in some vaguely relevant state)
      if ( ES_listenerData.state != eslsAbsent ) {
        // make sure the incoming mac addr matches the current listener
        bool bMatches = true;
        for (uint16_t idx = 0; idx < 6; idx++) {
          if ( ES_lastRxMacAddr[idx] != ES_listenerData.peerInfo.peer_addr[idx] ) {
            bMatches = false;
Serial.println("macAddr match fail");            
            break;
          }
        }
        if ( bMatches ) {
          if ( ES_listenerData.state == eslsAwaitingAck ) {
            if ( rxData.command == ENC_state_data_ack ) {
              // remember no longer waiting for anything
              ES_listenerData.ackTimeoutTime_MS = 0;
              ES_setListenerState(eslsRegisteredIdle);
              ES_stateUpdated = false;
            }//if not an ack, simply ignore, let timeout happen
          } else {
            // if the incoming is a keepAlive then send its ack,
            // reprime our keepAlive timer and absorb the command
            if ( rxData.command == ENC_keep_alive ) {
              ES_reprimeKeepAliveTimer();
              ES_listenerData.state = eslsRegisteredIdle; // this cancels any ack wait state
              ES_listenerData.ackTimeoutTime_MS = 0;
// txData.command = ENC_keep_alive_ack;
// ES_listenerSend(false, false);
              ES_listenerPrimeSend(ENC_keep_alive_ack, false);
            } else {
              // any other incoming data (eg: button commands) is passed up	
              ES_rxCommand = rxData.command;
            }
          }
        }//does not match our listener, just ignore
      }//endif whether registered
    }//endif/else over commands
  }//endif/else got data

  // if we have outstanding sends, then maybe do it now
  ES_listenerSendLoop();

}

void ES_setListenerState(ES_ListenerState newState) {
  if ( newState != ES_listenerData.state ) {
    //Serial.printf("ES_setListenerState %s --> %s\n", ES_getListenerStateName(ES_listenerData.state), ES_getListenerStateName(newState));
    ES_listenerData.state = newState;
  }
}

void ES_rememberListener() {
  ES_forgetListener();

  memcpy(ES_listenerData.peerInfo.peer_addr, ES_lastRxMacAddr, 6);
  ES_listenerData.peerInfo.channel = 0;
  ES_listenerData.peerInfo.encrypt = false;
  if ( esp_now_add_peer(&ES_listenerData.peerInfo) != ESP_OK ) {
    Serial.println("ES_rememberListener failed");
  } else {
    Serial.printf("ES_rememberListener(): mac:%02X:%02X:%02X:%02X:%02X:%02X\n", 
        (int)(ES_lastRxMacAddr[0]), (int)(ES_lastRxMacAddr[1]), (int)(ES_lastRxMacAddr[2]), 
        (int)(ES_lastRxMacAddr[3]), (int)(ES_lastRxMacAddr[4]), (int)(ES_lastRxMacAddr[5])
    );
  }
}

void ES_forgetListener() {
  if ( ES_listenerData.state != eslsAbsent ) {
    esp_now_del_peer(ES_listenerData.peerInfo.peer_addr);
  }
  ES_initListenerData();
  Serial.println("ES_forgetListener()");
}

const char *ES_getListenerStateName(ES_ListenerState state) {
  const char *res = "";
  switch (state) {
  case eslsAbsent:
    res = "absent";
    break;
  case eslsAwaitingAck:
    res = "awaitingAck";
    break;
  case eslsRegisteredIdle:
    res = "registered";
    break;
  }
  return res;

}

static ESPNOW_command ES_getRxCommand() {
  ESPNOW_command result = ES_rxCommand;
  ES_rxCommand = ENC_nop;
  return result;
}

#endif // ESPNOW_SATELLITE_H
