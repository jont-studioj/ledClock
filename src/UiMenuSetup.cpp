#include "UiMenuSetup.h"

#include "UiMenuDigitModeSettings.h"
#include "DigitDisplay.h"
#include "UiMenuDisplay.h"
#include "UiMenuSystem.h"

enum ItemIdx {
  idxModeTime = 1,
  idxModeDate = 2,
  idxModeTimer = 3,
  idxDisplay = 5,
  idxSystem = 6
};

void UiMenuSetup::initMenuContent() {
  addMenuItem(idxModeTime, "time", false, 0, DigitDisplayModeTime, new UiMenuDigitModeSettings(refProvider, "TIME", refProvider.digitDisplay.getDigitDisplayModeData(DigitDisplayModeTime)));
  addMenuItem(idxModeDate, "date", false, 0, DigitDisplayModeDate, new UiMenuDigitModeSettings(refProvider, "DATE", refProvider.digitDisplay.getDigitDisplayModeData(DigitDisplayModeDate)));
  addMenuItem(idxModeTimer, "timer", false, 0, DigitDisplayModeTimer, new UiMenuDigitModeSettings(refProvider, "TIMER", refProvider.digitDisplay.getDigitDisplayModeData(DigitDisplayModeTimer)));

  addMenuItem(idxDisplay, "display", false, 0, 0, new UiMenuDisplay(refProvider, "DISPLAY"));
  addMenuItem(idxSystem, "system", false, 0, 0, new UiMenuSystem(refProvider, "SYSTEM"));
}




