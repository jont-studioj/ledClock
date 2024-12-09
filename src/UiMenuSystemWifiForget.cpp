#include "UiMenuSystemWifiForget.h"
#include "RefProvider.h"
#include "WifiExecutive.h"

enum ItemIdx {
  idxConfirm = 1,
  idxDisplayLine2 = 2,    // not a real menu item, just used as a placeholder to draw/print to
  idxDisplayLine3 = 3,    // not a real menu item, just used as a placeholder to draw/print to
  idxDisplayLine4 = 4,    // not a real menu item, just used as a placeholder to draw/print to
  idxDisplayLine5 = 5,    // not a real menu item, just used as a placeholder to draw/print to
  idxDisplayLine6 = 6     // not a real menu item, just used as a placeholder to draw/print to
};

void UiMenuSystemWifiForget::initMenuContent() {
  addMenuItem(idxConfirm, "confirm", false, -1, 0, NULL);
}

void UiMenuSystemWifiForget::paintMenuContent(bool bRepaint, bool bAsActive) {
//Serial.printf("UiMenuSystemWifiForget::paintMenuContent(%d, %d)\n", bRepaint, bAsActive);
  // if we have no credentials already, then make the "confirm" already look selected else not
  bool bHaveCredentials = refProvider.wifiExecutive.haveCredentials();
  uiMenuItems[idxConfirm]->selected = !bHaveCredentials;

  UiMenu::paintMenuContent(bRepaint, bAsActive);

  printToItemLine(idxDisplayLine3, 4, false, "-- network --");
  printToItemLine(idxDisplayLine4, 2, true, refProvider.wifiExecutive.getWifiSSID().c_str());

  if ( bHaveCredentials ) {
    printToItemLine(idxDisplayLine6, 4, false, "forget this");
  }

}

void UiMenuSystemWifiForget::menuItemOptionChange(UiMenuItem *menuItem) {
  uint8_t itemIdx = menuItem->itemIdx;
  if (itemIdx == idxConfirm) {
    // if going from not-selected to selected then this is the command to do the forget
    // (else we must've been selected already - meaning we have nothing to forget)
    if ( menuItem->selected ) {
      // must've gone unselected --> selected, this is the command to do the forget
Serial.println("do forget");
      refProvider.wifiExecutive.forgetCredentials();
      // and leave the item as selected
      // but repaint to show new state
      paintMenu(true, true);
    } else {
      // must already have forgotten (or we never had credentials), nothing to do
      // force selection back to selected
Serial.println("nothing to forget");      
      menuItem->selected = true;
    }
  }
}