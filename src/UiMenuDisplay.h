#ifndef UIMENU_DISPLAY_H
#define UIMENU_DISPLAY_H

#include "UiMenu.h"


class UiMenuDisplay: public UiMenu {
public:
  using UiMenu::UiMenu;

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);
  void menuItemOptionChange(UiMenuItem *menuItem);
  
private:

};

#endif // UIMENU_DISPLAY_H
