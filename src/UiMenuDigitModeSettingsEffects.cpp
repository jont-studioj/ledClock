#include "UiMenuDigitModeSettingsEffects.h"

enum ItemIdent {
  itemIdentDigitStroke,
  itemIdentDigitUnstroke,
  itemIdentDigitGrow,
  itemIdentDigitShrink,
  itemIdentPixelFadeIn,
  itemIdentPixelFadeOut
};

enum ItemIdx {
  idxDigitStroke = 1,
  idxDigitUnstroke = 2,
  idxDigitGrow = 3,
  idxDigitShrink = 4,
  idxPixelFadeIn = 5,
  idxPixelFadeOut = 6
};

UiMenuDigitModeSettingsEffects::UiMenuDigitModeSettingsEffects(RefProvider &refProvider, const char *menuTitle, DigitDisplayModeData *digitDisplayModeData) : 
  UiMenu(refProvider, menuTitle),
  digitDisplayModeData(digitDisplayModeData) {
}

void UiMenuDigitModeSettingsEffects::initMenuContent() {
  addMenuItem(idxDigitStroke, "digit stroke", false, -1, itemIdentDigitStroke, NULL);
  addMenuItem(idxDigitUnstroke, "   \" unstroke", false, -1, itemIdentDigitUnstroke, NULL);
  addMenuItem(idxDigitGrow, "digit grow", false, -1, itemIdentDigitGrow, NULL);
  addMenuItem(idxDigitShrink, "   \"    shrink", false, -1, itemIdentDigitShrink, NULL);
  addMenuItem(idxPixelFadeIn, "fade in", false, -1, itemIdentPixelFadeIn, NULL);
  addMenuItem(idxPixelFadeOut, "fade out", false, -1, itemIdentPixelFadeOut, NULL);
}

void UiMenuDigitModeSettingsEffects::paintMenuContent(bool bRepaint, bool bAsActive) {
  uiMenuItems[idxDigitStroke]->selected = digitDisplayModeData->fxStroke;
  uiMenuItems[idxDigitUnstroke]->selected = digitDisplayModeData->fxUnstroke;
  uiMenuItems[idxDigitGrow]->selected = digitDisplayModeData->fxGrow;
  uiMenuItems[idxDigitShrink]->selected = digitDisplayModeData->fxShrink;
  uiMenuItems[idxPixelFadeIn]->selected = digitDisplayModeData->fxFadeIn;
  uiMenuItems[idxPixelFadeOut]->selected = digitDisplayModeData->fxFadeOut;
  UiMenu::paintMenuContent(bRepaint, bAsActive);
}


void UiMenuDigitModeSettingsEffects::menuItemOptionChange(UiMenuItem *menuItem) {
  UiMenu::menuItemOptionChange(menuItem);
  ItemIdent itemIdent = (ItemIdent)menuItem->itemIdent;

Serial.printf("UiMenuDigitModeSettingsEffects::menuItemOptionChange(%d)[%s] = %d\n", itemIdent, menuItem->caption, menuItem->selected);

  switch (itemIdent) {
  case itemIdentDigitStroke:
    digitDisplayModeData->fxStroke = menuItem->selected;
    digitDisplayModeData->fxGrow = false;
    deselectMutuallyExclusiveOther(idxDigitGrow);
    break;
  case itemIdentDigitUnstroke:
    digitDisplayModeData->fxUnstroke = menuItem->selected;
    digitDisplayModeData->fxShrink = false;
    deselectMutuallyExclusiveOther(idxDigitShrink);
    break;
  case itemIdentDigitGrow:
    digitDisplayModeData->fxGrow = menuItem->selected;
    digitDisplayModeData->fxStroke = false;
    deselectMutuallyExclusiveOther(idxDigitStroke);
    break;
  case itemIdentDigitShrink:
    digitDisplayModeData->fxShrink = menuItem->selected;
    digitDisplayModeData->fxUnstroke = false;
    deselectMutuallyExclusiveOther(idxDigitUnstroke);
    break;
  case itemIdentPixelFadeIn:
    digitDisplayModeData->fxFadeIn = menuItem->selected;
    break;
  case itemIdentPixelFadeOut:
    digitDisplayModeData->fxFadeOut = menuItem->selected;
    break;
  }
  refProvider.digitDisplay.applyActiveDigitDisplayModeSettings();
}

void UiMenuDigitModeSettingsEffects::deselectMutuallyExclusiveOther(uint8_t itemIdx) {
  UiMenuItem *menuItem = uiMenuItems[itemIdx];
  if ( menuItem->selected ) {
    menuItem->selected = false;
    paintMenuItem(menuItem);
  }
}

void UiMenuDigitModeSettingsEffects::activateThyself(bool bRepaint) {
Serial.println("UiMenuDigitModeSettingsEffects::activateThyself()");
  refProvider.digitDisplay.previewEffects(true);
  refProvider.ui->setUiTimeoutPeriodMS(60000);    // be more generous with ui timeout while playing with effects
  UiMenu::activateThyself(bRepaint);
}


void UiMenuDigitModeSettingsEffects::deactivateThyself(bool bRepaint) {
Serial.println("UiMenuDigitModeSettingsEffects::deactivateThyself()");
  refProvider.digitDisplay.previewEffects(false);
  refProvider.ui->setDefaultUiTimeoutPeriod();
  UiMenu::deactivateThyself(bRepaint);
}
