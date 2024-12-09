#include "UiMenuSystemDoorServo.h"
#include "UiMenuWidgetScalarSlider.h"
#include "Sundries.h"
#include "UI.h"

// ******************************************************************************
// servo duty adjust slider
// ******************************************************************************
class ServoDutyAdjustSlider : public UiMenuWidgetScalarSlider {
  public:
    ServoDutyAdjustSlider(RefProvider &refProvider, const char *menuTitle, bool bAdjustOpenDuty) :
      UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider(refProvider, menuTitle),
      adjustingOpenDuty(bAdjustOpenDuty)
      {
    }  
  private:
    int32_t getRangeLowerBound(int itemIdent) { return refProvider.sundries.displayDoor_getMinDuty(); }
    int32_t getRangeUpperBound(int itemIdent) { return refProvider.sundries.displayDoor_getMaxDuty(); }
    int32_t getRangeMinValue(int itemIdent) { return  adjustingOpenDuty ? refProvider.sundries.displayDoor_getMinDuty() : refProvider.sundries.displayDoor_getMidDuty(); }
    int32_t getRangeMaxValue(int itemIdent) { return adjustingOpenDuty ? refProvider.sundries.displayDoor_getMidDuty() : refProvider.sundries.displayDoor_getMaxDuty(); }
    int32_t getStepSizeSlow(int itemIdent) { return 5; }
    int32_t getStepSizeFast(int itemIdent) { return 20; }
    int32_t getValue(int itemIdent) {
      return adjustingOpenDuty ? refProvider.sundries.displayDoor_getDutyOpen() : refProvider.sundries.displayDoor_getDutyShut();
    }
    int32_t setValue(int itemIdent, int32_t newValue) {
      rememberSomethingChanged(true); 
      // poke sundries directly with the new value
      if ( adjustingOpenDuty ) {
        refProvider.sundries.displayDoor_setDutyOpen(newValue);
      } else {
        refProvider.sundries.displayDoor_setDutyShut(newValue);
      }    
      return getValue(menuItemSelf->itemIdent); 
    }
    void activateThyself(bool bRepaint) {
      refProvider.sundries.displayDoor_tweakDutyValueBegin(adjustingOpenDuty);
      refProvider.ui->setUiTimeoutPeriodMS(60000);    // be more generous with ui timeout while playing with door servo
      UiMenu::activateThyself(bRepaint);
    }
    void deactivateThyself(bool bRepaint) {
      refProvider.sundries.displayDoor_tweakDutyValueEnd();
      refProvider.ui->setDefaultUiTimeoutPeriod();
      UiMenu::deactivateThyself(bRepaint);
    }

    bool adjustingOpenDuty;
};

enum ItemIdx {
  itemIdxSetPwmDutyOpen = 1,
  itemIdxSetPwmDutyShut = 2
};

void UiMenuSystemDoorServo::initMenuContent() {
  addMenuItem(itemIdxSetPwmDutyOpen, "open point", false, 0, 0, new ServoDutyAdjustSlider(refProvider, "SET OPEN POINT", true));
  addMenuItem(itemIdxSetPwmDutyShut, "shut point", false, 0, 0, new ServoDutyAdjustSlider(refProvider, "SET SHUT POINT", false));
}

void UiMenuSystemDoorServo::deactivateThyself(bool bRepaint) {
  //Serial.print("UiMenuSystemDoorServo::deactivateThyself()");

  if ( didAnythingChange(true, false) ) {
    //Serial.println(" - something changed, saving");
    refProvider.sundries.displayDoor_saveConfig();
  } else {
    //Serial.println(" - nothing changed");
  }
  UiMenu::deactivateThyself(bRepaint);
}
