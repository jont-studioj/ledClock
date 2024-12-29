#include "UiMenuSystemBrightness.h"
#include "UiMenuWidgetScalarSlider.h"
#include "BrightnessAdjustModeEnum.h"
#include "UiMenuSystemBrightnessAdjust.h"
#include "TimeSource.h"
#include "Sundries.h"
#include "DigitDisplay.h"
#include "DisplayPanel.h"

// ******************************************************************************
// (digits only) gamma adjust slider
// ******************************************************************************
class SetGammaSlider : public UiMenuWidgetScalarSlider {
  public:
    using UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider;
    void activateThyself(bool bRepaint) {
      UiMenu::activateThyself(bRepaint);
      // set gamma adjust mode
      refProvider.digitDisplay.setDoingGammaValueAdjust(true);
    }
    void deactivateThyself(bool bRepaint) {
      // clear gamma adjust mode      
      refProvider.digitDisplay.setDoingGammaValueAdjust(false);
      UiMenu::deactivateThyself(bRepaint);
    }

  private:
    int32_t getRangeLowerBound(int itemIdent) { return 1; }
    int32_t getRangeUpperBound(int itemIdent) { return 50; }
    int32_t getRangeMinValue(int itemIdent) { return 1; }
    int32_t getRangeMaxValue(int itemIdent) { return 50; }
    int32_t getStepSizeSlow(int itemIdent) { return 1; }
    int32_t getStepSizeFast(int itemIdent) { return 5; }
    int32_t getValue(int itemIdent) {
      return refProvider.digitDisplay.getGammaValue() * 10;
    }
    int32_t setValue(int itemIdent, int32_t newValue) {
      rememberSomethingChanged(true); 
      // poke digitDisplay directly with the new value
      float newGammaValue = (float)(newValue / 10.0f);
      //Serial.printf("newValue=%d, float gammaValue: %f\n", newValue, newGammaValue);
      refProvider.digitDisplay.setGammaValue(newGammaValue);
      return getValue(menuItemSelf->itemIdent); 
    }
    void paintValueLabel(int32_t value) {
      refProvider.displayPanel.setTextDatum(TC_DATUM);
      float gammaValue = (float)(value / 10.0f);
      refProvider.displayPanel.drawFloat(gammaValue, 1, textLt + (textWd / 2), textTp, fontNo);
      refProvider.displayPanel.setTextDatum(TL_DATUM);
    }
};

// ******************************************************************************
// ambient adjust smoothing slider
// ******************************************************************************
class LightMeterSmoothingSlider : public UiMenuWidgetScalarSlider {
  public:
    using UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider;

  private:
    int32_t getRangeLowerBound(int itemIdent) { return 0; }
    int32_t getRangeUpperBound(int itemIdent) { return 9; }
    int32_t getRangeMinValue(int itemIdent) { return 0; }
    int32_t getRangeMaxValue(int itemIdent) { return 9; }
    int32_t getStepSizeSlow(int itemIdent) { return 1; }
    int32_t getStepSizeFast(int itemIdent) { return 1; }
    int32_t getValue(int itemIdent) {
      return refProvider.sundries.lightMeter_getSmoothingQtySamples() - 1;
    }
    int32_t setValue(int itemIdent, int32_t newValue) {
      rememberSomethingChanged(true); 
      // poke sundries directly with the new value
      refProvider.sundries.lightMeter_setSmoothingQtySamples(newValue + 1);
      return getValue(menuItemSelf->itemIdent); 
    }
};


enum ItemIdx {
  itemIdxSetMin = 1,
  itemIdxSetMax = 2,
  itemIdxSetGamma = 3,
  itemIdxSetSmoothing = 4,
  itemIdxSetDaylightBoost = 6
};

void UiMenuSystemBrightness::initMenuContent() {
  addMenuItem(itemIdxSetMin, "set min", false, 0, 0, new UiMenuSystemBrightnessAdjust(refProvider, "SET MINIMUMS", UserBrightnessAdjustMode::ubamMin));
  addMenuItem(itemIdxSetMax, "set max", false, 0, 0, new UiMenuSystemBrightnessAdjust(refProvider, "SET MAXIMUMS", UserBrightnessAdjustMode::ubamMax));

  addMenuItem(itemIdxSetGamma, "set gamma", false, 0, 0, new SetGammaSlider(refProvider, "SET GAMMA"));

  addMenuItem(itemIdxSetSmoothing, "smoothing", false, 0, 0, new LightMeterSmoothingSlider(refProvider, "SMOOTHING"));

  addMenuItem(itemIdxSetDaylightBoost, "daylight +", false, -1, 0, new UiMenuSystemBrightnessAdjust(refProvider, "DAYLIGHT +", UserBrightnessAdjustMode::ubamDaylightBoost));
}

void UiMenuSystemBrightness::paintMenuContent(bool bRepaint, bool bAsActive) {
  // if we're not in fudgy config mode then hide the gammAdjust menu
  // (yes, I know that leaves an unreferenced object, but don't care right now)
  // enable commissioning mode by holding the button down while booting
  if ( !refProvider.sundries.bCommissioningMode ) {
    //Serial.println("Hiding gamma adjust");
    uiMenuItems[itemIdxSetGamma] = NULL;
  }

  uiMenuItems[itemIdxSetDaylightBoost]->selected = refProvider.timeSource.getDaylightBoostEnabled();
  UiMenu::paintMenuContent(bRepaint, bAsActive);
}


void UiMenuSystemBrightness::menuItemOptionChange(UiMenuItem *menuItem) {
  uint8_t itemIdx = menuItem->itemIdx;
  if (itemIdx == itemIdxSetDaylightBoost) {
    UiMenu::menuItemOptionChange(menuItem);
    refProvider.timeSource.setDaylightBoostEnabled(menuItem->selected);
    refProvider.digitDisplay.checkDaylightBoost(false);
    refProvider.displayPanel.checkDaylightBoost(false);
  }
}

//todo: maybe move this to UiMenuSystemBrightness
void UiMenuSystemBrightness::deactivateThyself(bool bRepaint) {
  //Serial.print("UiMenuSystemBrightnessAdjust::deactivateThyself()");

  if ( didAnythingChange(true, false) ) {
    //Serial.println(" - something changed, saving");
    refProvider.digitDisplay.saveSundriesConfig();
    refProvider.digitDisplay.saveAmbientAdjustConfig();
    refProvider.displayPanel.saveAmbientAdjustConfig();
    refProvider.timeSource.saveDaylightData();
    refProvider.sundries.lightMeter_saveSmoothingQtySamples();
  } else {
    //Serial.println(" - nothing changed");
  }
  UiMenu::deactivateThyself(bRepaint);
}
