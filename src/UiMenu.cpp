//2023-12-12 mark before ridding us of bAsActiveChildItem & recoding colours
#include "UiMenu.h"
#include "DisplayPanel.h"
#include "Sundries.h"       // for display panel scheme data (it's there, not here, for reasons)

UiMenu::UiMenu(RefProvider &refProvider, const char *menuTitle) : 
  refProvider(refProvider) {
    setTitle(menuTitle);
}

void UiMenu::setTitle(const char *menuTitle) {
  snprintf(title, sizeof(title), " %s ", menuTitle);
}

void UiMenu::initialise(struct UiMenuItem *parentMenuItem) {
  // remember the parent's menuItem that caused me to exist
  menuItemSelf = parentMenuItem;

  // initialise some vars
  for (uint8_t itemIdx = 0; itemIdx < MAX_QTY_MENU_ITEMS; itemIdx++) {
    uiMenuItems[itemIdx] = NULL;
  }

  topMenuItemIdx = -1;
  bDisplayed = false;
  bActive = false;
  bSomethingChanged = false;
  forgetFocusAndActiveChildMenu();  

  // initialise remaining content...
  // this is to a virtual method that is overridden by subclasses
  initMenuContent();
}

// default / empty / no-content (to be overridden by subclasses)
void UiMenu::initMenuContent() {

  // create the initial <<< back <<< / exit entry
  addBackMenuItem();

}

void UiMenu::setHierarchy(UiMenu *parentMenu) {
  this->parentMenu = parentMenu;
  // cascade hierarchy to all immediate child menus (they may in turn cascade it further down)
  for (uint8_t itemIdx = 0; itemIdx <= topMenuItemIdx; itemIdx++) {
    cascadeHierarchy( uiMenuItems[itemIdx] );
  }
}
//todo: now we're only using a single display, with not displayIdx, we might no longer need the cascade
void UiMenu::cascadeHierarchy(UiMenuItem *menuItem) {
  if ( menuItem != NULL ) {
    UiMenu *childMenu = menuItem->childMenu;
    if ( childMenu != NULL ) {
      childMenu->setHierarchy(this);
    }//endif this menu item has a child menu
  }
}

void UiMenu::addBackMenuItem() {
  addThisMenuItem(&itemEntryBack);
}

UiMenuItem *UiMenu::addMenuItem(
    uint8_t itemIdx,
    const char *caption,
    bool selected,
    int8_t optionGroup,
    int itemIdent,
    UiMenu *childMenu) {

  UiMenuItem *menuItem = NULL;
  if ( itemIdx < MAX_QTY_MENU_ITEMS ) {
    menuItem = new UiMenuItem;
    menuItem->itemIdx = itemIdx;
    menuItem->caption = caption;
    menuItem->selected = selected;
    menuItem->optionGroup = optionGroup;
    menuItem->itemIdent = itemIdent;
    menuItem->childMenu = childMenu;
    addThisMenuItem(menuItem);
  }
  return menuItem;
}

void UiMenu::addThisMenuItem(UiMenuItem *menuItem) {
  // note: the item index must've been set in the item before we're called
  uint8_t itemIdx = menuItem->itemIdx;
  if ( itemIdx < MAX_QTY_MENU_ITEMS) {
    uiMenuItems[itemIdx] = menuItem;
    if ( itemIdx > topMenuItemIdx ) {
      topMenuItemIdx = itemIdx;
    }
    
    // if this item has a child menu, then set its hierarchy now
    UiMenu *childMenu = menuItem->childMenu;
    if ( childMenu != NULL ) {
      childMenu->initialise(menuItem);
    }
    //Serial.printf("UiMenu::addThisMenuItem(%s)\n", menuItem->caption);
    cascadeHierarchy( menuItem );
  }
}


//virtual: can be overridden by descendants
// default is do nothing
void UiMenu::menuItemOptionChange(UiMenuItem *menuItem) {
// after all the subclassing menus malarkey we can maybe get rid of this
  rememberSomethingChanged(true);
}
// remember that something changed (with optional propagation of the flag setting)
void UiMenu::rememberSomethingChanged(bool bPropagateIndication) {
  setSomethingChanged(true, bPropagateIndication);
}
// forget that something changed (with optional propagation of the flag setting)
void UiMenu::forgetSomethingChanged(bool bPropagateIndication) {
  setSomethingChanged(false, bPropagateIndication);
}
// remember or forget that something changed (with optional propagation of the flag setting)
void UiMenu::setSomethingChanged(bool value, bool bPropagateIndication) {
  bSomethingChanged = value;
  if ( bPropagateIndication && (parentMenu != NULL ) ) {
    parentMenu->setSomethingChanged(value, bPropagateIndication);
  }
}
// check if we think that something changed, optionally resetting the flag (with optional propagation of that reset)
bool UiMenu::didAnythingChange(bool bResetIndication, bool bPropagateClearIndication) {
  bool result = bSomethingChanged;
  if ( bResetIndication ) {
    forgetSomethingChanged(bPropagateClearIndication);
  }
  return result;
}

void UiMenu::forgetFocusAndActiveChildMenu() {
  itemIdxFocus = -1;                    // make sure I don't think I have any item focussed
  itemIdxActiveChildMenu = -1;          // make sure I don't think I have any child menu selected

  bJustReturnedFromSelectedCheckboxSubmenuKludge = false;
}

// this can be overridden by sub-classes
// show myself
void UiMenu::showThyself(bool bRepaint) {
  if ( bDisplayed == false ) {
    forgetFocusAndActiveChildMenu();

    // open myself, possibly repaint now
    bDisplayed = true;
    paintMenu(bRepaint, false);           // false = not as an active menu
  }
}
// hide myself
void UiMenu::hideThyself(bool bRepaint) {
  bDisplayed = false;
}

// being activated when chosen by my parent
void UiMenu::activateThyself(bool bRepaint) {
  // open myself, painting my menu, make myself active, take focus
  showThyself(false);    // false, don't repaint

  bActive = true;

  // if no item has focus, then make it our [<<<back<<<] entry
  if ( itemIdxFocus == -1 ) {
    itemIdxFocus = 0;
  }
  paintMenu(bRepaint, true);           // true = as an active menu
}
// deactivating myself, returning focus back to parent
void UiMenu::deactivateThyself(bool bRepaint) {
  //Serial.printf("%s: deactivateThyself()\n", menuItemSelf->caption);
  // close myself, clear my display, release focus
  bActive = false;
  hideThyself(false);     // false, do not repaint
}

UiEvent UiMenu::processItemEnter(int8_t itemIdx, bool bLongClick) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  if ( itemIdx == 0 ) {
    // item zero always <<< back <<<, treat as menuExit
    deactivateThyself(true);
    highLevelEvent = UiEvent::ueMenuDeactivating;
  } else {
    if ( (itemIdx > 0) && (itemIdx <= topMenuItemIdx) ) {
      // if the focussed item has a child menu (or some other manipulatable item)
      // then we potentially activate that and drop into it (with ourselves losing the focus)
      //
      UiMenuItem *menuItem = uiMenuItems[itemIdx];

      if ( menuItem != NULL ) {
        // we see if we should enter a possible submenu only if the item
        // was already selected, or was not part of an optionGroup,
        // so we need to find that out before we select it now
        bool bShouldEnterSubMenu = couldEnterSubMenu(menuItem);

        // horrid hacky kludge to kinda support checkbox type items
        // with - or without - submenus
        // (those submenus only get activated if the item is already selected)
        // under these conditions, these checkbox-with-submenus feel like they
        // act like tri-state toggles but in fact are still only take bi-state
        // values and the "third state" is kinda virtual and represents the
        // state where we've entered the item's submenu
        // (it's kinda hard to explain, tbh. And I can't tell you how many
        //  Serial.print() debugging lines and multitude hacks I did to get
        //  this sodding thing to work the way I wanted. And I bet I end up
        //   not needing this feature anyway)
        bool bDoSelectMenuItem = true;
        if ( itemIsSelectedCheckboxWithSubmenuKludge(itemIdx) ) {
          if ( bJustReturnedFromSelectedCheckboxSubmenuKludge ) {
            bJustReturnedFromSelectedCheckboxSubmenuKludge = false;
            bShouldEnterSubMenu = false;
          } else {
            bDoSelectMenuItem = false;
          }
        }

        // now is the time to make this item selected (or toggle it)
        if ( bDoSelectMenuItem ) {
          selectMenuItem(itemIdx);
        }

        // if this item is an enterable submenu
        // we lose focus, activate that child menu, maybe...
        // but we only do that under the following rules:
        //  - the item is NOT a member of an optionGroup
        //  - OR the item is and it was already the selected one in that optionGroup
        if ( bShouldEnterSubMenu ) {
          loseItemFocus(true);
          UiMenu *childMenu = menuItem->childMenu;
          childMenu->activateThyself(true);
          itemIdxActiveChildMenu = itemIdx;
          // (note: we leave highLevelEvent as ueNoEvent)
        }//endif bShouldEnterSubMenu
      }//endif this item is in use
    }//endif there was a non-zero item focussed
    // (if there wasn't then we silently ignore event, not sure how this could happen, but hey)
  }//endif itemIdxFocus was 0/something else

  return highLevelEvent;
}


void UiMenu::selectMenuItem(int8_t itemIdx) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  // only bother if not zero <<<back<<< option
  if ( (itemIdx > 0) && (itemIdx <= topMenuItemIdx) ) {
    UiMenuItem *menuItem = uiMenuItems[itemIdx];
    if ( menuItem != NULL ) {
      int8_t optionGroup = menuItem->optionGroup;
      // we only need do anything if we're dealing with something with non-zero optionGroup
      // (only items that are non-zero optionGroup items are actually selectable)
      bool bThisItemChanged = false;
      // an optionGroup of -1 indicates a checkbox style toggle
      if ( optionGroup == -1 ) {
        // a checkbox type item always toggles on and off and counts as a change
        // (notwithstanding the horrid kludge done elsewhere for checkbox type items with submenus)
        menuItem->selected = !menuItem->selected;
        menuItemOptionChange(menuItem);
        bThisItemChanged = true;
      } else if ( optionGroup > 0 ) {
        // a radio-button type item, could/should be part of a group:
        // if there exists a different item in the same optionGroup that is
        // currently selected then delselect that old one (with visual indication)
        for (uint8_t otherItemIdx = 0; otherItemIdx <= topMenuItemIdx; otherItemIdx++) {
          // we're only interested in things that aren't the one being selected
          if ( otherItemIdx != itemIdx ) {
            UiMenuItem *otherMenuItem = uiMenuItems[otherItemIdx];
            if ( otherMenuItem != NULL ) {
              uint8_t otherOptionGroup = otherMenuItem->optionGroup;
              if ( otherOptionGroup == optionGroup ) {
                // if this other one had been selected, then 
                //  - deselect it
                //  - indicate that visually
                //  - remember we had a configChangeEvent regardless of what we end up doing with the "current" item
                if ( otherMenuItem->selected ) {
                  otherMenuItem->selected = false;
                  menuItemOptionChange(otherMenuItem);
                  // now repaint this other item to indicate its new state
                  paintMenuItem(otherMenuItem);
                }
              }//endif same optionGroup
            }//other item is in use
          }//endif this a different item to the one we're now selecting
        }//next otherItem
        // whether or not we deselected another item in the group,
        // we definitely want this one selected, and if it wasn't already
        // then we count this as a change
        if ( !menuItem->selected ) {
          menuItem->selected = true;
          menuItemOptionChange(menuItem);
          bThisItemChanged = true;
        }
      }//endif we're dealing with a checkbox or radio-button type optionGroup item
      if ( bThisItemChanged ) {
        // now repaint this item to indicate its new state
        paintMenuItem(menuItem);
      }
    }//endif this item is in use
  }//endif we're dealing with a non-zero menuItem

}

UiEvent UiMenu::childMenuDeactivating(UiLowLevelEventSource lowLevelEventSource) {
  // an active child menu just closed, we regain focus etc
  // as we're regaining focus from a child menu, focus falls back onto the currently active item
  // (because that's where we must've come from)
  //  we need to totally repaint myself - unless we're exiting UI totally)

  bool bRepaint = lowLevelEventSource != UiLowLevelEventSource::ullesIdleTimeout;
  paintMenu(bRepaint, true);
  gainItemFocus(bRepaint, itemIdxActiveChildMenu);
  // kludge to support checkbox style items with or without submenus
  // (it's rather hacky and doesn't make for a great interface, tbh)
  bJustReturnedFromSelectedCheckboxSubmenuKludge = itemIsSelectedCheckboxWithSubmenuKludge(itemIdxActiveChildMenu);

  itemIdxActiveChildMenu = -1;
  return UiEvent::ueNoEvent;    // eat the event
}





// wholesale focus of myself... when passing focus from me to child and vice versa
// child closing/deactivating, I am regaining item focus ring
void UiMenu::gainItemFocus(bool bRepaint, int8_t itemIdx) {
  if ( itemIdxFocus != itemIdx ) {
    itemIdxFocus = itemIdx;
    showItemFocus(bRepaint);
  }
}
// child opening/activating, I am losing item focus ring
void UiMenu::loseItemFocus(bool bRepaint) {
  if ( itemIdxFocus != -1 ) {
    hideItemFocus(bRepaint);
    itemIdxFocus = -1;
  }
}

// item focus within myself, between my items...
void UiMenu::showItemFocus(bool bRepaint) {
  if ( bRepaint ) {
    drawFocusRing(itemIdxFocus, refProvider.sundries.getDisplayScheme().focusRing);
    showItemFocusSubMenuIndicator();
  }
}
void UiMenu::hideItemFocus(bool bRepaint) {
  if ( bRepaint ) {
    drawFocusRing(itemIdxFocus, refProvider.sundries.getDisplayScheme().menuBg);
    hideItemFocusSubMenuIndicator();
  }
}
void UiMenu::drawFocusRing(int8_t itemIdx, uint16_t colour) {
  if ( itemIdx != -1 ) {
    struct ItemRect itemRect = getItemRect(itemIdx, false);
    refProvider.displayPanel.drawRect(itemRect.x, itemRect.y, itemRect.wd, itemRect.ht, colour);
  }
}

void UiMenu::showItemFocusSubMenuIndicator() {
  if ( itemIdxFocus > 0 ) {
    UiMenuItem *menuItem = uiMenuItems[itemIdxFocus];
    if ( couldEnterSubMenu(menuItem) ) {
      uint16_t itemFgdColour = getItemFgdColour(menuItem);
      uint16_t itemBkgColour = getItemBkgColour(menuItem);
      drawSubMenuIndicator(menuItem, itemFgdColour, itemBkgColour);
    }
  }
}
void UiMenu::hideItemFocusSubMenuIndicator() {
  if ( itemIdxFocus > 0 ) {
    UiMenuItem *menuItem = uiMenuItems[itemIdxFocus];
    if ( couldEnterSubMenu(menuItem) ) {
      uint16_t itemBkgColour = getItemBkgColour(menuItem);
      drawSubMenuIndicator(menuItem, itemBkgColour, itemBkgColour);
    }
  }
}
void UiMenu::drawSubMenuIndicator(UiMenuItem *menuItem, uint16_t itemFgdColour, uint16_t itemBkgColour) {
  if ( (menuItem != NULL) && subMenuWantsIndicator(menuItem) ) {
    uint8_t itemIdx = menuItem->itemIdx;
    struct ItemRect itemRect = getItemRect(itemIdx, true);    // true = use padding to get the inner rect
    refProvider.displayPanel.setTextColor(itemFgdColour, itemBkgColour, true);
    uint8_t nudgeTextY = (itemIdx == 0) ? 0 : 2;
    refProvider.displayPanel.setCursor(itemRect.x + itemRect.wd - 7, itemRect.y + 4 + nudgeTextY, 2);
    refProvider.displayPanel.print(">");
  }//endif this item is in use
}

bool UiMenu::couldEnterSubMenu(UiMenuItem *menuItem) {
  bool bCouldEnterSubMenu = false;
  if ( menuItem != NULL ) {
    if ( menuItem->childMenu != NULL ) {
      if ( menuItem->itemIdx == itemIdxFocus ) {
        // it is an item with a submenu, we're either in it already
        // or we could enter that..
        // but we would only enter the submenu under the following rules:
        //  - the item is NOT a member of an optionGroup (nor is a checkbox)
        //  - OR the item is and it is already in the selected state
        bCouldEnterSubMenu = (menuItem->optionGroup == 0) || menuItem->selected;
      }
    }
  }//endif this item is in use
  return bCouldEnterSubMenu;
}

bool UiMenu::subMenuWantsIndicator(UiMenuItem *menuItem) {
  bool bWantsIndicator = false;
  if ( menuItem != NULL ) {
    if ( menuItem->childMenu != NULL ) {
      bWantsIndicator = menuItem->childMenu->wantSubMenuIndicator();
    }
  }//endif this item is in use
  return bWantsIndicator;
}


struct ItemRect UiMenu::getItemRect(uint8_t itemIdx, bool bInner) {
  uint8_t paddingTop = bInner ? 2 : 0;
  uint8_t paddingBottom = bInner ? 1 : 0;
  uint8_t paddingLeft = bInner ? 2 : 0;
  uint8_t paddingRight = bInner ? 2 : 0;
  
  uint8_t paddingWd = paddingLeft + paddingRight;
  uint8_t paddingHt = paddingTop + paddingBottom;
  

  uint8_t x = paddingLeft;
  uint8_t y;
  uint8_t wd = TFT_WIDTH - paddingWd;
  uint8_t ht;
  uint8_t fontNo;
  if ( itemIdx == 0 ) {
    y = bInner ? 3 : 0;
    ht = 18+4+1 - paddingHt;
    fontNo = 2;
  } else {
    y = 27 + (itemIdx-1) * 35 + paddingTop;
    ht = 30+4 - paddingHt;
    fontNo = 4;
  }
  struct ItemRect rect = {x, y, wd, ht, fontNo};
  return rect;
}

void UiMenu::paintMenu(bool bRepaint, bool bMenuActive) {
  if ( bRepaint ) {
    // whether we're painting ourselves or blanking our display, we need to blank it
    refProvider.displayPanel.clearDisplay();
    if ( bDisplayed ) {
      // show the zero/exit item, but only if we are (to be) active
      // Serial.printf("paintMenu() bMenuActive =%d\n", bMenuActive);
      if ( bMenuActive ) {
        paintTopMenuItem();
      }
      // show the remaining menu content: (can be overridden by descendant classes)
      paintMenuContent(bRepaint, bMenuActive);

      // maybe show focus ring
      showItemFocus(bMenuActive);
    }
  }
}
void UiMenu::paintTopMenuItem() {
  struct ItemRect itemRect = getItemRect(0, true);    // true = use padding to get the inner rect

  DisplayPanelColourScheme scheme = refProvider.sundries.getDisplayScheme();

  refProvider.displayPanel.setTextColor(scheme.titleFg, scheme.titleBg, true);
  refProvider.displayPanel.setTextDatum(TC_DATUM);

  uint8_t fontNo = 2;   // hack
  refProvider.displayPanel.drawString("^^^^^^^^^^^^^^^^^^^", TFT_WIDTH / 2, itemRect.y, fontNo);    
  refProvider.displayPanel.drawString(title, TFT_WIDTH / 2, itemRect.y, fontNo);

  refProvider.displayPanel.setTextDatum(TL_DATUM);
}

void UiMenu::paintMenuItem(UiMenuItem *menuItem) {
  if ( menuItem != NULL ) {

    uint8_t itemIdx = menuItem->itemIdx;
    
    struct ItemRect itemRect = getItemRect(itemIdx, true);    // true = use padding to get the inner rect

    uint16_t itemFgdColour = getItemFgdColour(menuItem);
    uint16_t itemBkgColour = getItemBkgColour(menuItem);

    refProvider.displayPanel.fillRect(itemRect.x, itemRect.y, itemRect.wd, itemRect.ht, itemBkgColour);
    refProvider.displayPanel.setTextColor(itemFgdColour, itemBkgColour, true);
    uint8_t nudgeTextY = (itemIdx == 0) ? 0 : 2;
    refProvider.displayPanel.setCursor(itemRect.x + 2, itemRect.y + nudgeTextY, itemRect.fontNo);
    refProvider.displayPanel.print(menuItem->caption);
    
    if ( couldEnterSubMenu(menuItem) ) {
      drawSubMenuIndicator(menuItem, itemFgdColour, itemBkgColour);
    }
    // redraw focus ring because if the text is a little too large we can lose the right edge
    // (I'm sure there's a better place to make this call (or even different function to call)
    //  but I gave up looking after spending an age on it and decided this would do.
    //  Yes, I realise that this is all my own code and I should know, but hey)
    if ( menuItem->itemIdx == itemIdxFocus ) {
      drawFocusRing(itemIdxFocus, refProvider.sundries.getDisplayScheme().focusRing);
    }
    
  }//endif this item is in use

}

void UiMenu::printToItemLine(uint8_t itemIdx, uint8_t font, bool bSelected, const char *text) {
  struct ItemRect itemRect = getItemRect(itemIdx, true);    // true = use padding to get the inner rect

  uint16_t itemFgdColour = getFgdColour(bSelected);
  uint16_t itemBkgColour = getBkgColour(bSelected);

  refProvider.displayPanel.fillRect(itemRect.x, itemRect.y, itemRect.wd, itemRect.ht, itemBkgColour);
  refProvider.displayPanel.setTextColor(itemFgdColour, itemBkgColour, true);

  uint8_t nudgeX = (font == 2 ? 5 : 2);
  uint8_t nudgeY = (font == 2 ? 6 : 4);
  refProvider.displayPanel.setCursor(itemRect.x + nudgeX, itemRect.y + nudgeY, font);
  refProvider.displayPanel.print(text);
}


//virtual: to be overridden by descendants
// this default paints all other menuItems in the normal manner
void UiMenu::paintMenuContent(bool bRepaint, bool bAsActive) {
  if ( bRepaint ) {  
    // show all item captions (with varying colouring depending on states)
    for (uint8_t itemIdx = 1; itemIdx <= topMenuItemIdx; itemIdx++) {
      paintMenuItem(uiMenuItems[itemIdx]);
    }//next menu iten
  }
}


uint16_t UiMenu::getItemFgdColour(UiMenuItem *menuItem) {
  uint16_t colour;
  if ( menuItem != NULL ) {
    colour = getFgdColour(menuItem->selected);
  } else {
    colour = refProvider.sundries.getDisplayScheme().menuBg;
  }//endif this item is/isn't in use
  return colour;
}
uint16_t UiMenu::getItemBkgColour(UiMenuItem *menuItem) {
  uint16_t colour;
  if ( menuItem != NULL ) {
    colour = getBkgColour(menuItem->selected);
  } else {
     colour = refProvider.sundries.getDisplayScheme().menuBg;
  }//endif this item is/isn't in use
  return colour;
}

uint16_t UiMenu::getFgdColour(bool bSelected) {
  DisplayPanelColourScheme scheme = refProvider.sundries.getDisplayScheme();
  return bSelected ? scheme.selectedFg : scheme.unselectedFg;
}
uint16_t UiMenu::getBkgColour(bool bSelected) {
  DisplayPanelColourScheme scheme = refProvider.sundries.getDisplayScheme();  
  return bSelected ? scheme.selectedBg : scheme.unselectedBg;
}


UiEvent UiMenu::processMenuExit(UiLowLevelEventSource lowLevelEventSource) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  UiMenu *activeMenu = getActiveMenu();
  if ( activeMenu != NULL ) {
    // if I was active then we are exiting me
    if ( activeMenu == this ) {
      deactivateThyself(true);
      highLevelEvent = UiEvent::ueMenuDeactivating;
    } else {
      highLevelEvent = activeMenu->processMenuExit(lowLevelEventSource);
      // if an immediate child is deactivating then I must be (re)gaining focus
      if ( highLevelEvent == UiEvent::ueMenuDeactivating ) {
        highLevelEvent = childMenuDeactivating(lowLevelEventSource);
      }//endif child menu deactivating
    }//endif it was/was-not me that had focus
  }//endif something had focus

  return highLevelEvent;
}

UiEvent UiMenu::processMenuEnter(UiLowLevelEventSource lowLevelEventSource, bool bLongClick) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  UiMenu *activeMenu = getActiveMenu();
  // if nothing was active then we must be actually activating myself
  if ( activeMenu == NULL ) {
    activateThyself(true);       // true = repaint now
    highLevelEvent = UiEvent::ueMenuActivating;
  } else {
    // if the activeMenu is me, then we deal with the event, else we pass it down
    if ( activeMenu == this ) {
      // we are the active dude (and should have some item focussed) so deal with the event ourselves
      highLevelEvent = processItemEnter(itemIdxFocus, bLongClick);
    } else {
      // active menu was some child of mine, pass the event down
      highLevelEvent = activeMenu->processMenuEnter(lowLevelEventSource, bLongClick);
      if ( highLevelEvent == UiEvent::ueMenuDeactivating ) {
        highLevelEvent = childMenuDeactivating(lowLevelEventSource);
      } else if ( highLevelEvent == UiEvent::ueMenuActivating ) {
        // just eat the event
        highLevelEvent = UiEvent::ueNoEvent;
      }//endif child menu deactivating/activating

    }//endif the focus was on me/some other item
  }//endif there was/wasn't a menu active already

  return highLevelEvent;
}

UiEvent UiMenu::processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  UiMenu *activeMenu = getActiveMenu();
  // if no menu was active then ignore the event
  if ( activeMenu != NULL ) {
    // if the activeMenu is me, then we deal with the event, else we pass it down
    if ( activeMenu == this ) {
      // we are the active dude (and should have some item focussed) so deal with the event ourselves
      if ( itemIdxFocus > 0 ) {
        hideItemFocus(true);
        itemIdxFocus--;
        showItemFocus(true);
      }
      bJustReturnedFromSelectedCheckboxSubmenuKludge = false;
    } else {
      // active menu was some child of mine, pass the event down
      highLevelEvent = activeMenu->processMenuDecrease(lowLevelEventSource, bAltModeInput);
    }//endif the focus was on me/some other item
  }//endif there was/wasn't a menu active already
  
  return highLevelEvent;
}

UiEvent UiMenu::processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  UiMenu *activeMenu = getActiveMenu();
  // if no menu was active then ignore the event
  if ( activeMenu != NULL ) {
    // if the activeMenu is me, then we deal with the event, else we pass it down
    if ( activeMenu == this ) {
      // we are the active dude (and should have some item focussed) so deal with the event ourselves
      if ( itemIdxFocus < topMenuItemIdx ) {
        hideItemFocus(true);
        itemIdxFocus++;
        showItemFocus(true);
      }
      bJustReturnedFromSelectedCheckboxSubmenuKludge = false;
    } else {
      // active menu was some child of mine, pass the event down
      highLevelEvent = activeMenu->processMenuIncrease(lowLevelEventSource, bAltModeInput);
    }//endif the focus was on me/some other item
  }//endif there was/wasn't a menu active already

  return highLevelEvent;
}

UiMenu *UiMenu::getActiveMenu() {
  UiMenu *result = NULL;

  // if I am active...
  if ( bActive ) {
    // if none of my child menus are active then the answer must be me
    if ( itemIdxActiveChildMenu == -1 ) {
      result = this;
    } else {
      // it must be the child (or one of its children) that's active 
      UiMenuItem *menuItem = uiMenuItems[itemIdxActiveChildMenu];
      if ( menuItem != NULL ) {
        result = menuItem->childMenu;
      }//endif this item is in use
    }//endif I have an active child menu
  }//endif I am active/open
  return result;
}

bool UiMenu::itemIsSelectedCheckboxWithSubmenuKludge(int8_t itemIdx) {
  bool result = false;
  if ( (itemIdx > 0) && (itemIdx <= topMenuItemIdx) ) {
    UiMenuItem *menuItem = uiMenuItems[itemIdx];
    if ( menuItem != NULL ) {
      result = ((menuItem->optionGroup == -1) && (menuItem->childMenu != NULL) && menuItem->selected);
    }//endif this item is in use
  }
  return result;
}