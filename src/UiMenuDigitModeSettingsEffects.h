#ifndef UIMENU_DIGIT_MODE_SETTINGS_EFFECTS_H
#define UIMENU_DIGIT_MODE_SETTINGS_EFFECTS_H

#include "UiMenu.h"
#include "DigitDisplay.h"

class UiMenuDigitModeSettingsEffects: public UiMenu {
public:
  UiMenuDigitModeSettingsEffects(RefProvider &refProvider, const char *menuTitle, DigitDisplayModeData *digitDisplayModeData);

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);

  void menuItemOptionChange(UiMenuItem *menuItem);
  
  void activateThyself(bool bRepaint);
  void deactivateThyself(bool bRepaint);
  
private:
  DigitDisplayModeData *digitDisplayModeData;

  void deselectMutuallyExclusiveOther(uint8_t itemIdx);

};

#endif // UIMENU_DIGIT_MODE_SETTINGS_EFFECTS_H
