#include "UiMenuDisplay.h"
#include "Sundries.h"

enum ItemIdx {
  schemeItemIdx_cyangrey = 1,
  schemeItemIdx_amber = 2
};

enum SchemeIdx {
  schemeIdx_cyangrey = 0,
  schemeIdx_amber = 1
};


void UiMenuDisplay::initMenuContent() {
  //todo: ought really to get these names from the structures, rather than duplicating the strings here
  addMenuItem(schemeItemIdx_cyangrey, "cyan/grey", false, 1, schemeIdx_cyangrey, NULL);
  addMenuItem(schemeItemIdx_amber, "amber", false, 1, schemeIdx_amber, NULL);
}


void UiMenuDisplay::paintMenuContent(bool bRepaint, bool bAsActive) {
  uint8_t displaySchemeIdx = refProvider.sundries.getDisplaySchemeIdx();
  uiMenuItems[schemeItemIdx_cyangrey]->selected = (displaySchemeIdx == schemeIdx_cyangrey);
  uiMenuItems[schemeItemIdx_amber]->selected = (displaySchemeIdx == schemeIdx_amber);
  UiMenu::paintMenuContent(bRepaint, bAsActive);
}

void UiMenuDisplay::menuItemOptionChange(UiMenuItem *menuItem) {
  uint8_t itemIdx = menuItem->itemIdx;
  if ( (itemIdx >= schemeItemIdx_cyangrey) && (itemIdx <= schemeItemIdx_amber) ) {
    if ( menuItem->selected ) {
      refProvider.sundries.setDisplaySchemeIdx(menuItem->itemIdent, true);
      paintMenu(true, true);
    }
  }
}
