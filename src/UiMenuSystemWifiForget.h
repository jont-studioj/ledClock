#ifndef UIMENU_SYSTEM_WIFI_FORGET_H
#define UIMENU_SYSTEM_WIFI_FORGET_H

#include "UiMenu.h"


class UiMenuSystemWifiForget: public UiMenu {
public:
  using UiMenu::UiMenu;

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);
  void menuItemOptionChange(UiMenuItem *menuItem);

private:

};

#endif // UIMENU_SYSTEM_WIFI_FORGET_H
