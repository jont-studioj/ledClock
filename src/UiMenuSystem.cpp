#include "UiMenuSystem.h"
#include "UiMenuSystemBrightness.h"
#include "UiMenuSystemWifi.h"
#include "UiMenuSystemDoorServo.h"

#include "Sundries.h"

enum ItemIdx {
  idxBrightness = 1,
  idxWifi = 3,
  idxDisplayDoor = 5,
  idxReboot = 6
};

void UiMenuSystem::initMenuContent() {
  addMenuItem(idxBrightness, "brightness", false, 0, 0, new UiMenuSystemBrightness(refProvider, "BRIGHTNESS"));
  addMenuItem(idxWifi, "wifi", false, 0, 0, new UiMenuSystemWifi(refProvider, "WIFI"));
  if ( refProvider.sundries.displayDoor_getPresent() ) {
    addMenuItem(idxDisplayDoor, "door servo", false, 0, 0, new UiMenuSystemDoorServo(refProvider, "DOOR SERVO ADJUST"));
  }
  addMenuItem(idxReboot, "reboot", false, -1, 0, NULL);
}

void UiMenuSystem::menuItemOptionChange(UiMenuItem *menuItem) {
  uint8_t itemIdx = menuItem->itemIdx;
  if ( itemIdx == idxReboot ) {
    esp_restart();
  }
}

