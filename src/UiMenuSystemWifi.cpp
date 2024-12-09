#include "UiMenuSystemWifi.h"
#include "UiMenuSystemWifiConfigure.h"
#include "RefProvider.h"
#include "WifiExecutive.h"

enum ItemIdx {
//  idxSetupOrForget = 1,
  idxConfigure = 1,
  idxDisplayLine2 = 2,
  idxDisplayLine3 = 3,    // not a real menu item, just used as a placeholder to draw/print to
  idxDisplayLine4 = 4,    // not a real menu item, just used as a placeholder to draw/print to
  idxDisplayLine5 = 5,    // not a real menu item, just used as a placeholder to draw/print to
  idxDisplayLine6 = 6     // not a real menu item, just used as a placeholder to draw/print to
};

void UiMenuSystemWifi::initMenuContent() {
  addMenuItem(idxConfigure, "configure", false, 0, 0, new UiMenuSystemWifiConfigure(refProvider, "CONFIGURE WIFI"));
}

void UiMenuSystemWifi::paintMenuContent(bool bRepaint, bool bAsActive) {
  UiMenu::paintMenuContent(bRepaint, bAsActive);

  printToItemLine(idxDisplayLine2, 4, false, "-- network --");
  printToItemLine(idxDisplayLine3, 2, true, refProvider.wifiExecutive.getWifiSSID().c_str());
  printToItemLine(idxDisplayLine4, 4, false, refProvider.wifiExecutive.stateName(refProvider.wifiExecutive.getWifiState()));
  printToItemLine(idxDisplayLine5, 4, false, "my name:");
  printToItemLine(idxDisplayLine6, 2, true, refProvider.wifiExecutive.getWifiHostname().c_str());

}
