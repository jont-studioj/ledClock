#ifndef UIMENU_DIGIT_MODE_SETTINGS_MORE_H
#define UIMENU_DIGIT_MODE_SETTINGS_MORE_H

#include "UiMenu.h"
#include "DigitDisplay.h"

class UiMenuDigitModeSettingsMore: public UiMenu {
public:
  UiMenuDigitModeSettingsMore(RefProvider &refProvider, const char *menuTitle, DigitDisplayModeData *digitDisplayModeData);

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);
  
  void menuItemOptionChange(UiMenuItem *menuItem);
  
  void deactivateThyself(bool bRepaint);

private:
  DigitDisplayModeData *digitDisplayModeData;

};

#endif // UIMENU_DIGIT_MODE_SETTINGS_MORE_H
