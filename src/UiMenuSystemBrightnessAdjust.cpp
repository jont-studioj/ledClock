#include "UiMenuSystemBrightnessAdjust.h"
#include "UiMenuWidgetScalarSlider.h"
#include "DigitDisplay.h"
#include "DisplayPanel.h"
#include "Sundries.h"

// ******************************************************************************
// digit brightness adjust slider
// ******************************************************************************
class DigitBrightnessSlider : public UiMenuWidgetScalarSlider {
  public:
    DigitBrightnessSlider(RefProvider &refProvider, const char *menuTitle, UserBrightnessAdjustMode adjustMode) :
      UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider(refProvider, menuTitle),
      adjustMode(adjustMode),
      doingDaylightBoost(adjustMode == UserBrightnessAdjustMode::ubamDaylightBoost)
      {
    }  
  private:
    int32_t getRangeLowerBound(int itemIdent) { return 0; }
    int32_t getRangeUpperBound(int itemIdent) { return doingDaylightBoost ? 75 : 100; }
    int32_t getRangeMinValue(int itemIdent) { return  doingDaylightBoost ? 0 : MIN_BRIGHTNESS; }
    int32_t getRangeMaxValue(int itemIdent) { return doingDaylightBoost ? 75 : 100; }
    int32_t getStepSizeSlow(int itemIdent) { return 1; }
    int32_t getStepSizeFast(int itemIdent) { return 5; }
    int32_t getValue(int itemIdent) {
      return doingDaylightBoost ? refProvider.digitDisplay.getDaylightBoost() : refProvider.digitDisplay.getUserBrightness();
    }
    int32_t setValue(int itemIdent, int32_t newValue) {
      rememberSomethingChanged(true); 
      // poke digitDisplay directly with the new value
      if ( doingDaylightBoost ) {
        refProvider.digitDisplay.setDaylightBoost(newValue);
      } else {
        refProvider.digitDisplay.setUserBrightness(newValue);
      }    
      return getValue(menuItemSelf->itemIdent); 
    }
    void activateThyself(bool bRepaint) {
      refProvider.digitDisplay.setUserBrightnessBegin(adjustMode);
      UiMenu::activateThyself(bRepaint);
    }
    void deactivateThyself(bool bRepaint) {
      refProvider.digitDisplay.setUserBrightnessEnd(adjustMode);
      UiMenu::deactivateThyself(bRepaint);
    }

    UserBrightnessAdjustMode adjustMode;
    bool doingDaylightBoost;
};
// ******************************************************************************

// ******************************************************************************
// displayPanel brightness adjust slider
// ******************************************************************************
class PanelBrightnessSlider : public UiMenuWidgetScalarSlider {
  public:
    PanelBrightnessSlider(RefProvider &refProvider, const char *menuTitle, UserBrightnessAdjustMode adjustMode) :
      UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider(refProvider, menuTitle),
      adjustMode(adjustMode),
      doingDaylightBoost(adjustMode == UserBrightnessAdjustMode::ubamDaylightBoost)
      {
    }
  private:
    int32_t getRangeLowerBound(int itemIdent) { return 0; }
    int32_t getRangeUpperBound(int itemIdent) { return doingDaylightBoost ? 75 : 100; }
    int32_t getRangeMinValue(int itemIdent) { return 5; }
    int32_t getRangeMaxValue(int itemIdent) { return doingDaylightBoost ? 75 : 100; }
    int32_t getStepSizeSlow(int itemIdent) { return 1; }
    int32_t getStepSizeFast(int itemIdent) { return 5; }
    int32_t getValue(int itemIdent) {
      return doingDaylightBoost ? refProvider.displayPanel.getDaylightBoost() : refProvider.displayPanel.getUserBrightness();
    }
    int32_t setValue(int itemIdent, int32_t newValue) {
      refProvider.sundries.displayDoor_shut(true);
      rememberSomethingChanged(true); 
      // poke displayPanel directly with the new value
      if ( doingDaylightBoost ) {
        refProvider.displayPanel.setDaylightBoost(newValue);
      } else {
        refProvider.displayPanel.setUserBrightness(newValue);
      }    
      return getValue(menuItemSelf->itemIdent); 
    }
    void activateThyself(bool bRepaint) {
      refProvider.displayPanel.setUserBrightnessBegin(adjustMode);
      UiMenu::activateThyself(bRepaint);
    }
    void deactivateThyself(bool bRepaint) {
      refProvider.displayPanel.setUserBrightnessEnd(adjustMode);
      refProvider.sundries.displayDoor_open(true);
      UiMenu::deactivateThyself(bRepaint);
    }

    UserBrightnessAdjustMode adjustMode;
    bool doingDaylightBoost;
};
// ******************************************************************************


// ******************************************************************************
// latitude adjust slider
// ******************************************************************************
class LatitudeSlider : public UiMenuWidgetScalarSlider {
public:
  LatitudeSlider(RefProvider &refProvider, const char *menuTitle, UiMenuSystemBrightnessAdjust *container) :
    UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider(refProvider, menuTitle),
    container(container)
    {
  }  
    
private:
  int32_t getRangeLowerBound(int itemIdent) { return -90; }
  int32_t getRangeUpperBound(int itemIdent) { return +90; }
  int32_t getRangeMinValue(int itemIdent) { return -90; }
  int32_t getRangeMaxValue(int itemIdent) { return +90; }
  int32_t getStepSizeSlow(int itemIdent) { return 1; }
  int32_t getStepSizeFast(int itemIdent) { return 10; }
  int32_t getValue(int itemIdent) { 
    container->daylightChange();
    return refProvider.timeSource.getLatitude(); 
  }
  int32_t setValue(int itemIdent, int32_t newValue) {
    rememberSomethingChanged(true); 
    // poke timeSource directly with the new value
    refProvider.timeSource.setLatitude(newValue);
    return getValue(menuItemSelf->itemIdent); 
  }
  UiMenuSystemBrightnessAdjust *container;
};
// ******************************************************************************

// ******************************************************************************
// longitude adjust slider
// ******************************************************************************
class LongitudeSlider : public UiMenuWidgetScalarSlider {
public:
  LongitudeSlider(RefProvider &refProvider, const char *menuTitle, UiMenuSystemBrightnessAdjust *container) :
    UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider(refProvider, menuTitle),
    container(container)
    {
  }  
    
private:
  int32_t getRangeLowerBound(int itemIdent) { return -180; }
  int32_t getRangeUpperBound(int itemIdent) { return +180; }
  int32_t getRangeMinValue(int itemIdent) { return -180; }
  int32_t getRangeMaxValue(int itemIdent) { return +180; }
  int32_t getStepSizeSlow(int itemIdent) { return 1; }
  int32_t getStepSizeFast(int itemIdent) { return 10; }
  int32_t getValue(int itemIdent) { 
    container->daylightChange();
    return refProvider.timeSource.getLongitude(); 
  }
  int32_t setValue(int itemIdent, int32_t newValue) {
    rememberSomethingChanged(true); 
    // poke timeSource directly with the new value
    refProvider.timeSource.setLongitude(newValue);
    return getValue(menuItemSelf->itemIdent); 
  }
  UiMenuSystemBrightnessAdjust *container;
};
// ******************************************************************************

// ******************************************************************************
// sun-alt adjust slider
// ******************************************************************************
class SunAltitudeSlider : public UiMenuWidgetScalarSlider {
public:
  SunAltitudeSlider(RefProvider &refProvider, const char *menuTitle, UiMenuSystemBrightnessAdjust *container) :
    UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider(refProvider, menuTitle),
    container(container)
    {
  }  
    
private:
  int32_t getRangeLowerBound(int itemIdent) { return -5; }
  int32_t getRangeUpperBound(int itemIdent) { return +15; }
  int32_t getRangeMinValue(int itemIdent) { return -5; }
  int32_t getRangeMaxValue(int itemIdent) { return +15; }
  int32_t getStepSizeSlow(int itemIdent) { return 1; }
  int32_t getStepSizeFast(int itemIdent) { return 1; }
  int32_t getValue(int itemIdent) { 
    container->daylightChange();
    return refProvider.timeSource.getDaylightSunAlt(); 
  }
  int32_t setValue(int itemIdent, int32_t newValue) {
    rememberSomethingChanged(true); 
    // poke timeSource directly with the new value
    refProvider.timeSource.setDaylightSunAlt(newValue);
    return getValue(menuItemSelf->itemIdent); 
  }
  UiMenuSystemBrightnessAdjust *container;
};
// ******************************************************************************

enum ItemIdx {
  idxAdjustDigitBrightness = 1,
  idxAdjustPanelBrightness = 2,
  idxSetLatitude = 3,
  idxSetLongitude = 4,
  idxSunAltitude = 5,         // higher than this we consider it (strong) daylight that requires us to boost the display
  idxDaylightPeriod = 6       // a dummy entry that does nothing except to display the daylight hours
};

UiMenuSystemBrightnessAdjust::UiMenuSystemBrightnessAdjust(RefProvider &refProvider, const char *menuTitle, UserBrightnessAdjustMode adjustMode) :
  UiMenu(refProvider, menuTitle),
  adjustMode(adjustMode) {
}

void UiMenuSystemBrightnessAdjust::initMenuContent() {
  addMenuItem(idxAdjustDigitBrightness, "digits", false, 0, 0, new DigitBrightnessSlider(refProvider, "DIGITS", adjustMode));
  addMenuItem(idxAdjustPanelBrightness, "panel", false, 0, 0, new PanelBrightnessSlider(refProvider, "PANEL", adjustMode));
  if ( adjustMode == ubamDaylightBoost ) {
    addMenuItem(idxSetLatitude, "latitude", false, 0, 0, new LatitudeSlider(refProvider, "LATITUDE", this));
    addMenuItem(idxSetLongitude, "longitude", false, 0, 0, new LongitudeSlider(refProvider, "LONGITUDE", this));
    addMenuItem(idxSunAltitude, "sun altitude", false, 0, 0, new SunAltitudeSlider(refProvider, "SUN ALTITUDE", this));
    addMenuItem(idxDaylightPeriod, "", false, 0, 0, NULL);
  }
}

void UiMenuSystemBrightnessAdjust::paintMenuContent(bool bRepaint, bool bAsActive) {
  updateDaylightPeriodText();
  UiMenu::paintMenuContent(bRepaint, bAsActive);
}

void UiMenuSystemBrightnessAdjust::deactivateThyself(bool bRepaint) {
  //Serial.print("UiMenuSystemBrightnessAdjust::deactivateThyself()");
  if ( didAnythingChange(true, false) ) {
    //Serial.println(" - something changed, saving");
    refProvider.digitDisplay.saveAmbientAdjustConfig();
    refProvider.displayPanel.saveAmbientAdjustConfig();
  }
  UiMenu::deactivateThyself(bRepaint);
}

void UiMenuSystemBrightnessAdjust::daylightChange() {
  updateDaylightPeriodText();
  paintMenuItem(uiMenuItems[idxDaylightPeriod]);
}

void UiMenuSystemBrightnessAdjust::updateDaylightPeriodText() {
  if ( adjustMode == ubamDaylightBoost ) {
    UiMenuItem *menuItem = uiMenuItems[idxDaylightPeriod];
    if ( refProvider.timeSource.haveValidDaylightHours() ) {
      uint16_t modDaylightBegins = refProvider.timeSource.getModDaylightBegins();
      uint8_t mmBegins = modDaylightBegins % 60;
      uint8_t hhBegins = modDaylightBegins / 60;

      uint16_t modDaylightEnds = refProvider.timeSource.getModDaylightEnds();
      uint8_t mmEnds = modDaylightEnds % 60;
      uint8_t hhEnds = modDaylightEnds / 60;
      
      sprintf(&daylightCaption[0], "%02d%02d-%02d%02d", hhBegins, mmBegins, hhEnds, mmEnds);
    } else {
      sprintf(&daylightCaption[0], "%s", "0000-0000");
    }
    menuItem->caption = daylightCaption;
  }
}