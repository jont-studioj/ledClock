#include "UiMenuSystem.h"
#include "UiMenuSystemBrightness.h"
#include "UiMenuSystemWifi.h"
#include "UiMenuSystemDoorServo.h"
#include "DigitDisplay.h"
#include "Sundries.h"

enum ItemIdx {
  idxBrightness = 1,
  idxWifi = 3,
  idxSoakTest = 4,
  idxDisplayDoor = 5,
  idxReboot = 6
};

void UiMenuSystem::initMenuContent() {
  addMenuItem(idxBrightness, "brightness", false, 0, 0, new UiMenuSystemBrightness(refProvider, "BRIGHTNESS"));
  addMenuItem(idxWifi, "wifi", false, 0, 0, new UiMenuSystemWifi(refProvider, "WIFI"));
  addMenuItem(idxSoakTest, "soak test", false, -1, 0, NULL);
  if ( refProvider.sundries.displayDoor_getPresent() ) {
    addMenuItem(idxDisplayDoor, "door servo", false, 0, 0, new UiMenuSystemDoorServo(refProvider, "DOOR SERVO ADJUST"));
  }
  addMenuItem(idxReboot, "reboot", false, -1, 0, NULL);
}

void UiMenuSystem::paintMenuContent(bool bRepaint, bool bAsActive) {
  // if we're not in fudgy config mode then hide the soak-test-option & door-servo menu
  // (yes, I know that leaves unreferenced objects, but don't care right now)
  // enable commissioning mode by holding the button down while booting
  if ( !refProvider.sundries.bCommissioningMode ) {
    uiMenuItems[idxSoakTest] = NULL;
    uiMenuItems[idxDisplayDoor] = NULL;
  }
  UiMenu::paintMenuContent(bRepaint, bAsActive);
}

void UiMenuSystem::menuItemOptionChange(UiMenuItem *menuItem) {
  uint8_t itemIdx = menuItem->itemIdx;
  switch (itemIdx) {
  case idxSoakTest:
    refProvider.digitDisplay.setSoakTestMode(menuItem->selected);
    break;
  case idxReboot:
    esp_restart();
    break;
  }

}

