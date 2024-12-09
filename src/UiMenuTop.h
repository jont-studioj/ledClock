#ifndef UIMENU_TOP_H
#define UIMENU_TOP_H

#include "UiMenu.h"

class UiMenuTop: public UiMenu {
public:
  using UiMenu::UiMenu;

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);
  UiEvent processItemEnter(int8_t itemIdx, bool bLongClick);

private:

};

#endif // UIMENU_TOP_H
