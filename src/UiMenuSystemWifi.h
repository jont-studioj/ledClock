#ifndef UIMENU_SYSTEM_WIFI_H
#define UIMENU_SYSTEM_WIFI_H

#include "UiMenu.h"


class UiMenuSystemWifi: public UiMenu {
public:
  using UiMenu::UiMenu;

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);

private:
  // UiMenu *childMenuSetup;
  // UiMenu *childMenuForget;
  // 
  // const char *caption_wifi_setup = "setup n/w";
  // const char *caption_wifi_forget = "forget n/w";


};

#endif // UIMENU_SYSTEM_WIFI_H
