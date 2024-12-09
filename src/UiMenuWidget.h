#ifndef UIMENU_WIDGET_H
#define UIMENU_WIDGET_H

#include "UiMenu.h"

class UiMenuWidget: public UiMenu {

public:
  using UiMenu::UiMenu;

  virtual void initMenuContent();
  virtual void paintMenuContent(bool bRepaint, bool bAsActive);

  virtual bool wantSubMenuIndicator() { return false; }

protected:
  static const uint8_t clientTop = 27;

};

#endif // UIMENU_WIDGET_H
