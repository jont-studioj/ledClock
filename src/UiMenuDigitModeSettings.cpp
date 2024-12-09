//MARK 2023-12-06
#include "UiMenuDigitModeSettings.h"
#include "UiMenuDigitModeSettingsEffects.h"
#include "UiMenuDigitModeSettingsColour.h"
#include "UiMenuDigitModeSettingsMore.h"

enum ItemIdent {
  itemIdentStyleSeriffed,
  itemIdentStyleSoftened,
  itemdIdentEffects,
  itemIdentColour1,
  itemIdentColour2,
  itemIdentMore
};

enum ItemIdx {
  idxStyleSeriffed = 1,
  idxStyleSoftened = 2,
  idxEfects = 3,
  idxColour1 = 4,
  idxColour2 = 5,
  idxMore
};


UiMenuDigitModeSettings::UiMenuDigitModeSettings(RefProvider &refProvider, const char *menuTitle, DigitDisplayModeData *digitDisplayModeData) : 
  UiMenu(refProvider, menuTitle),
  digitDisplayModeData(digitDisplayModeData) {
}

void UiMenuDigitModeSettings::initMenuContent() {
  addMenuItem(idxStyleSeriffed, "seriffed", digitDisplayModeData->seriffed, -1, itemIdentStyleSeriffed, NULL);
  addMenuItem(idxStyleSoftened, "softened", digitDisplayModeData->softened, -1, itemIdentStyleSoftened, NULL);
  addMenuItem(idxEfects, "effects", digitDisplayModeData->fxEnabled, -1, itemdIdentEffects, new UiMenuDigitModeSettingsEffects(refProvider, "EFFECTS", digitDisplayModeData));
  addMenuItem(idxColour1, "colour 1", false, 0, itemIdentColour1, new UiMenuDigitModeSettingsColour(refProvider, "COLOUR 1", &digitDisplayModeData->colourData1));
  addMenuItem(idxColour2, "colour 2", false, 0, itemIdentColour2, new UiMenuDigitModeSettingsColour(refProvider, "COLOUR 2", &digitDisplayModeData->colourData2));
  addMenuItem(idxMore, "more...", false, 0, itemIdentMore, new UiMenuDigitModeSettingsMore(refProvider, title, digitDisplayModeData));
}

void UiMenuDigitModeSettings::menuItemOptionChange(UiMenuItem *menuItem) {
  UiMenu::menuItemOptionChange(menuItem);
  ItemIdent itemIdent = (ItemIdent)menuItem->itemIdent;

  //Serial.printf("UiMenuDigitModeSettings::menuItemOptionChange(%d)[%s] = %d\n", itemIdent, menuItem->caption, menuItem->selected);

  switch (itemIdent) {
  case itemIdentStyleSeriffed:
    digitDisplayModeData->seriffed = menuItem->selected;
    refProvider.digitDisplay.applySettingsAndDisplayNow();
    break;
  case itemIdentStyleSoftened:
    digitDisplayModeData->softened = menuItem->selected;
    refProvider.digitDisplay.applySettingsAndDisplayNow();
    break;
  case itemdIdentEffects:
    digitDisplayModeData->fxEnabled = menuItem->selected;
    refProvider.digitDisplay.applyActiveDigitDisplayModeSettings();
    break;
  }
}

void UiMenuDigitModeSettings::activateThyself(bool bRepaint) {
  refProvider.digitDisplay.setActiveDigitDisplayMode(digitDisplayModeData->digitDisplayMode, true);
  UiMenu::activateThyself(bRepaint);
}

void UiMenuDigitModeSettings::deactivateThyself(bool bRepaint) {
Serial.print("UiMenuDigitModeSettings::deactivateThyself() ");
  if ( didAnythingChange(true, false) ) {
    Serial.println(" - something changed, saving");
    refProvider.digitDisplay.saveDigitDisplayModeData(digitDisplayModeData);
  } else {
    Serial.println(" - nothing changed");
  }
  refProvider.digitDisplay.autoSelectDigitDisplayMode();
  UiMenu::deactivateThyself(bRepaint);
}


