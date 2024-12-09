#ifndef UIMENU_SYSTEM_BRIGHTNESS_ADJUST_H
#define UIMENU_SYSTEM_BRIGHTNESS_ADJUST_H

#include "UiMenu.h"
#include "BrightnessAdjustModeEnum.h"

class UiMenuSystemBrightnessAdjust: public UiMenu {
public:
  UiMenuSystemBrightnessAdjust(RefProvider &refProvider, const char *menuTitle, UserBrightnessAdjustMode adjustMode);

  void initMenuContent();
  void paintMenuContent(bool bRepaint, bool bAsActive);

  void deactivateThyself(bool bRepaint);
  
  void daylightChange();

private:
  UserBrightnessAdjustMode adjustMode;

  char daylightCaption[10];   // 0000-0000
  void updateDaylightPeriodText();

};

#endif // UIMENU_SYSTEM_BRIGHTNESS_ADJUST_H
