#ifndef UIMENU_DIGIT_MODE_SETTINGS_COLOUR_H
#define UIMENU_DIGIT_MODE_SETTINGS_COLOUR_H

#include "UiMenu.h"
#include "DigitDisplay.h"

class UiMenuDigitModeSettingsColour: public UiMenu {
public:
  UiMenuDigitModeSettingsColour(RefProvider &refProvider, const char *menuTitle, ColourData *colourData);

  void initMenuContent();

  void activateThyself(bool bRepaint);
  void deactivateThyself(bool bRepaint);

private:
  ColourData *colourData;

};

#endif // UIMENU_DIGIT_MODE_SETTINGS_COLOUR_H
