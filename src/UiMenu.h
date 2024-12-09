//2023-12-12 mark before ridding us of bAsActiveChildItem & recoding colours
#ifndef UIMENU_H
#define UIMENU_H

#include <Arduino.h>
#include "UiEnums.h"
#include "RefProvider.h"
#include "UiMenuItem.h"


struct ItemRect {
  uint8_t x;        // start-x, including or excluding outer focus-ring boundary
  uint8_t y;        // start-y, including or excluding outer focus-ring boundary
  uint8_t wd;       // width,   including or excluding outer focus-ring boundary
  uint8_t ht;       // height,  including or excluding outer focus-ring boundary
  uint8_t fontNo;   // font number (2 or 4)
};

// max 6 items, +1 for the zeroeth (exit/back) item
#define MAX_QTY_MENU_ITEMS 1+6

static struct UiMenuItem itemEntryBack = {
    .itemIdx = 0,
    .caption = "<<<<<<<<<<<<<<<<<<<<<",
    .selected = false,
    .optionGroup = 0,
    .itemIdent = 0,
    .childMenu = NULL
};


static struct UiMenuItem itemEntryLessDoneMore = {
    .itemIdx = 0,
    .caption = "[<<<<]  [done]  [>>>>]",
    .selected = false,
    .optionGroup = 0,
    .itemIdent = 0,
    .childMenu = NULL
};


class UiMenu {
public:
  UiMenu(RefProvider &refProvider, const char *menuTitle);

  bool bDisplayed;              // is this menu displayed
  bool bActive;                 // is this menu displayed & active (regardless of whether any child menu is displayed & is active)
  bool bInSitu = false;         // should this "subMenu" be displayed on the same displayPanel as the parent menu? (most often false)

  // the following gets called by addMenuItem(), to ensure that it gets called at some point 
  // (and addMenuItem() is a good time to do it)
  //
  // it should be used to remember the parent's menuItem that caused me to exist
  // (because we might want to save config values in it, not sure yet)
  // and then to initialise the remaining content (could be more menuItems, or something fancy)
  void initialise(struct UiMenuItem *parentMenuItem);

  //virtual: can be overridden by descendants
  virtual void initMenuContent();

  void setHierarchy(UiMenu *parentMenu);
  void cascadeHierarchy(UiMenuItem *menuItem);
  


  void addBackMenuItem();
  UiMenuItem *addMenuItem(
    uint8_t itemIdx,
    const char *caption,
    bool selected,
    int8_t optionGroup,
    int itemIdent,
    UiMenu *childMenu
  );
  void addThisMenuItem(UiMenuItem *menuItem);

  //virtual: can be overridden by descendants
  virtual void menuItemOptionChange(UiMenuItem *menuItem);

// after all the subclassing menus malarkey we can maybe get rid of this lot
  void rememberSomethingChanged(bool bPropagateIndication);
  void forgetSomethingChanged(bool bPropagateIndication);
  void setSomethingChanged(bool value, bool bPropagateIndication);
  bool didAnythingChange(bool bResetIndication, bool bPropagateIndication);

  void forgetFocusAndActiveChildMenu();

//todo: many of these could be private / protected
// does having these all public have an effect on the size of the vtable or whatever (and hence memory footprint)?

  //virtual: can be overridden by descendants
  virtual void showThyself(bool bRepaint);
  virtual void hideThyself(bool bRepaint);
  virtual void activateThyself(bool bRepaint);
  virtual void deactivateThyself(bool bRepaint);

  virtual UiEvent processItemEnter(int8_t itemIdx, bool bLongClick);
  void selectMenuItem(int8_t itemIdx);

  UiEvent childMenuDeactivating(UiLowLevelEventSource lowLevelEventSource);

  void paintMenu(bool bRepaint, bool bAsActive);
  void paintTopMenuItem();
  void paintMenuItem(UiMenuItem *menuItem);
  //virtual: can be overridden by descendants
  virtual void paintMenuContent(bool bRepaint, bool bAsActive);
  

//virtual: can be overridden by descendants
  virtual UiEvent processMenuExit(UiLowLevelEventSource lowLevelEventSource);
  virtual UiEvent processMenuEnter(UiLowLevelEventSource lowLevelEventSource, bool bLongClick);
  virtual UiEvent processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);
  virtual UiEvent processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);

protected:
  RefProvider &refProvider;
  //const char *title;
  char title[32];

  UiMenu *parentMenu;

  struct UiMenuItem *menuItemSelf;                      // the (parent's) menuItem that caused me to exist

  // list of menu items (of varying sorts)
  // item zero is most often "<<<back<<<" and doesn't represent a real item
  // (but may be repurposed for widget type menu things)
  int8_t topMenuItemIdx;
  UiMenuItem *uiMenuItems[MAX_QTY_MENU_ITEMS];

  // (0 = back/exit, -1 = none)
  int8_t itemIdxFocus;              // which of my items is currently-highlighted/has-focus-ring selection
  int8_t itemIdxActiveChildMenu;    // which of my items is currently the active one (if it's a child menu that is open/active)

  bool bSomethingChanged;

  void setTitle(const char *menuTitle);

  UiMenu *getActiveMenu();

  struct ItemRect getItemRect(uint8_t itemIdx, bool bInner);    // bInner: false=outer-focus-ring, true=inner-text

  void printToItemLine(uint8_t itemIdx, uint8_t font, bool bSelected, const char *text);

  uint16_t getItemFgdColour(UiMenuItem *menuItem);
  uint16_t getItemBkgColour(UiMenuItem *menuItem);
  uint16_t getFgdColour(bool bSelected);
  uint16_t getBkgColour(bool bSelected);

  void gainItemFocus(bool bRepaint, int8_t itemIdx);
  void loseItemFocus(bool bRepaint);
  virtual void showItemFocus(bool bRepaint);
  void hideItemFocus(bool bRepaint);
  void drawFocusRing(int8_t itemIdx, uint16_t colour);
  virtual bool wantSubMenuIndicator() { return true; }
  void showItemFocusSubMenuIndicator();
  void hideItemFocusSubMenuIndicator();
  void drawSubMenuIndicator(UiMenuItem *menuItem, uint16_t itemFgdColour, uint16_t itemBkgColour);
  bool couldEnterSubMenu(UiMenuItem *menuItem);
  bool subMenuWantsIndicator(UiMenuItem *menuItem);

private:
  bool bJustReturnedFromSelectedCheckboxSubmenuKludge;
  bool itemIsSelectedCheckboxWithSubmenuKludge(int8_t itemIdx);  

};

#endif // UIMENU_H
