#include "UiMenuDigitModeSettingsColour.h"
#include "UiMenuWidgetDblHueSelector.h"
#include "UiMenuWidgetScalarSlider.h"

class HueSlider : public UiMenuWidgetDblHueSelector {
public:
  HueSlider(RefProvider &refProvider, const char *menuTitle, ColourData *colourData) :
    UiMenuWidgetDblHueSelector::UiMenuWidgetDblHueSelector(refProvider, menuTitle), 
    colourData(colourData) {
  }
    
private:
  ColourData *colourData;
  int32_t getStepSizeSlow(int itemIdent) { return 1; }
  int32_t getStepSizeFast(int itemIdent) { return 8; }
  int32_t getValue(int itemIdent) {
    //Serial.printf("getVal(): digitDisplayModeData->saturation=%d, pctVal=%d\n", digitDisplayModeData->saturation, pctVal);
    return colourData->hue;
  }
  int32_t setValue(int itemIdent, int32_t newValue) {
    uint16_t newHue = newValue;
    if ( newHue > 511 ) {
      newHue = 511;
    }
    //Serial.printf("setVal(%d): pctVal=%d, newSaturation=%d\n", newValue, pctVal, newSaturation);
    rememberSomethingChanged(true); 
    // save the new value in the digit-style data
    colourData->hue = newHue;

    // force digitDisplay to show the new value
    refProvider.digitDisplay.previewColourData(colourData);

    return getValue(menuItemSelf->itemIdent); 
  }

  void activateThyself(bool bRepaint) {
    //  Serial.println("HueSlider::activateThyself()");
    // start previewing hue...
    // start forcing the digitDisplay to show fully saturated hue
    refProvider.digitDisplay.forceFullSat(true);
    UiMenu::activateThyself(bRepaint);
  }

  void deactivateThyself(bool bRepaint) {
    //  Serial.println("HueSlider::deactivateThyself()");
    // stop previewing this hue:
    // stop forcing the digitDisplay to show fully saturated hue
    refProvider.digitDisplay.forceFullSat(false);
    UiMenu::deactivateThyself(bRepaint);
  }

};


class SaturationSlider : public UiMenuWidgetScalarSlider {
public:
  SaturationSlider(RefProvider &refProvider, const char *menuTitle, ColourData *colourData) :
    UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider(refProvider, menuTitle), 
    colourData(colourData),
    pctVal(colourData->sat / 2.55f) {
  }
    
private:
  ColourData *colourData;
  uint8_t pctVal;
  int32_t getRangeLowerBound(int itemIdent) { return 0; }
  int32_t getRangeUpperBound(int itemIdent) { return 100; }
  int32_t getRangeMinValue(int itemIdent) { return 0; }
  int32_t getRangeMaxValue(int itemIdent) { return 100; }
  int32_t getStepSizeSlow(int itemIdent) { return 1; }
  int32_t getStepSizeFast(int itemIdent) { return 4; }
  int32_t getValue(int itemIdent) {
    //Serial.printf("getVal(): digitDisplayModeData->saturation=%d, pctVal=%d\n", digitDisplayModeData->saturation, pctVal);
    return pctVal;
  }
  int32_t setValue(int itemIdent, int32_t newValue) {
    pctVal = newValue;
    uint8_t newSaturation = pctVal * 2.55f;
    //Serial.printf("setVal(%d): pctVal=%d, newSaturation=%d\n", newValue, pctVal, newSaturation);
    rememberSomethingChanged(true); 
    // save the new value in the digit-style data
    colourData->sat = newSaturation;

    // force digitDisplay to show the new value
    refProvider.digitDisplay.previewColourData(colourData);

    return getValue(menuItemSelf->itemIdent); 
  }
};

enum ItemIdent {
  itemIdentHue,
  itemIdentSat
};

enum ItemIdx {
  idxHue = 1,
  idxSat = 2
};

UiMenuDigitModeSettingsColour::UiMenuDigitModeSettingsColour(RefProvider &refProvider, const char *menuTitle, ColourData *colourData) : 
  UiMenu(refProvider, menuTitle),
  colourData(colourData) {
}


void UiMenuDigitModeSettingsColour::initMenuContent() {
  addMenuItem(idxHue, "hue", false, 0, itemIdentHue, new HueSlider(refProvider, "HUE", colourData));
  addMenuItem(idxSat, "saturation", false, 0, itemIdentSat, new SaturationSlider(refProvider, "SATURATION", colourData));
}


void UiMenuDigitModeSettingsColour::activateThyself(bool bRepaint) {
Serial.println("UiMenuDigitModeSettingsColour::activateThyself()");
  // force the digitDisplay to be previewing these colour settings
  refProvider.digitDisplay.previewColourData(colourData);
  UiMenu::activateThyself(bRepaint);
}


void UiMenuDigitModeSettingsColour::deactivateThyself(bool bRepaint) {
Serial.println("UiMenuDigitModeSettingsColour::deactivateThyself()");
  // stop previewing this colour
  refProvider.digitDisplay.previewColourData(NULL);

  // we rely on the parent menu unload to save our stuff
  
  UiMenu::deactivateThyself(bRepaint);
}


