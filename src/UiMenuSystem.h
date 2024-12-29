#ifndef UIMENU_SYSTEM_H
#define UIMENU_SYSTEM_H

#include "UiMenu.h"

class UiMenuSystem: public UiMenu {
public:
  using UiMenu::UiMenu;

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);
  void menuItemOptionChange(UiMenuItem *menuItem);
  
private:

};

#endif // UIMENU_SYSTEM_H
