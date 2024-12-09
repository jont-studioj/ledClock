#ifndef UIMENU_SYSTEM_BRIGHTNESS_H
#define UIMENU_SYSTEM_BRIGHTNESS_H

#include "UiMenu.h"

class UiMenuSystemBrightness: public UiMenu {
public:
  using UiMenu::UiMenu;

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);
  void menuItemOptionChange(UiMenuItem *menuItem);

  void deactivateThyself(bool bRepaint);
      
private:

};

#endif // UIMENU_SYSTEM_BRIGHTNESS_H
