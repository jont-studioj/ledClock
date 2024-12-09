#ifndef UIMENU_DIGIT_MODE_SETTINGS_H
#define UIMENU_DIGIT_MODE_SETTINGS_H

#include "UiMenu.h"
#include "DigitDisplay.h"

class UiMenuDigitModeSettings: public UiMenu {
public:
  UiMenuDigitModeSettings(RefProvider &refProvider, const char *menuTitle, DigitDisplayModeData *digitDisplayModeData);

  void initMenuContent();

  void menuItemOptionChange(UiMenuItem *menuItem);
  
  void activateThyself(bool bRepaint);
  void deactivateThyself(bool bRepaint);

private:
  DigitDisplayModeData *digitDisplayModeData;

};

#endif // UIMENU_DIGIT_MODE_SETTINGS_H
