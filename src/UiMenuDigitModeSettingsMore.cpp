#include "UiMenuDigitModeSettingsMore.h"
#include "UiMenuWidgetScalarSlider.h"

// ******************************************************************************
// colour / lerp test slider
// ******************************************************************************
class LerpTestSlider : public UiMenuWidgetScalarSlider {
public:
  using UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider;

  void activateThyself(bool bRepaint) { 
    refProvider.digitDisplay.setLerpTestVal(lerpValue);
    UiMenuWidgetScalarSlider::activateThyself(bRepaint);
  }
  virtual void deactivateThyself(bool bRepaint) {
    refProvider.digitDisplay.setLerpTestVal(-1);
    UiMenuWidgetScalarSlider::deactivateThyself(bRepaint);
  }
   
private:
  int32_t lerpValue = 50;
  int32_t getRangeLowerBound(int itemIdent) { return 0; }
  int32_t getRangeUpperBound(int itemIdent) { return 100; }
  int32_t getRangeMinValue(int itemIdent) { return 0; }
  int32_t getRangeMaxValue(int itemIdent) { return 100; }
  int32_t getStepSizeSlow(int itemIdent) { return 1; }
  int32_t getStepSizeFast(int itemIdent) { return 4; }
  int32_t getValue(int itemIdent) {
    return lerpValue;
  }
  int32_t setValue(int itemIdent, int32_t newValue) {
    lerpValue = newValue;
    refProvider.digitDisplay.setLerpTestVal(lerpValue);
    return lerpValue;
  }
};
// ******************************************************************************

// ******************************************************************************
// timezone/UTCoffset slider
// ******************************************************************************
class UtcOffsetSlider : public UiMenuWidgetScalarSlider {
public:
  using UiMenuWidgetScalarSlider::UiMenuWidgetScalarSlider;
    
private:
  int32_t getRangeLowerBound(int itemIdent) { return UTC_OFFSET_INDEX_MIN; }
  int32_t getRangeUpperBound(int itemIdent) { return UTC_OFFSET_INDEX_MAX; }
  int32_t getRangeMinValue(int itemIdent) { return UTC_OFFSET_INDEX_MIN; }
  int32_t getRangeMaxValue(int itemIdent) { return UTC_OFFSET_INDEX_MAX; }
  int32_t getStepSizeSlow(int itemIdent) { return 1; }
  int32_t getStepSizeFast(int itemIdent) { return 1; }
  int32_t getValue(int itemIdent) { return refProvider.timeSource.getUtcOffsetIdx(); }
  int32_t setValue(int itemIdent, int32_t newValue) {
    rememberSomethingChanged(true); 
    // poke timeSource directly with the new value
    refProvider.timeSource.setUtcOffsetIdx(newValue);
    return getValue(menuItemSelf->itemIdent); 
  }
  void paintValueLabel(int32_t value) {
    refProvider.displayPanel.drawString(refProvider.timeSource.getUtcOffsetLabel(value), textLt, textTp, fontNo);
  }

  void activateThyself(bool bRepaint) {
    refProvider.digitDisplay.setTempDisableFx(true);
    UiMenuWidgetScalarSlider::activateThyself(bRepaint);
  }
  void deactivateThyself(bool bRepaint) {
    refProvider.digitDisplay.setTempDisableFx(false);
    UiMenuWidgetScalarSlider::deactivateThyself(bRepaint);
  }

};
// ******************************************************************************

enum ItemIdent {
  itemIdentColourTest,
  itemIdentTimeModeDstActive,
  itemIdentTimeModeUtcOffset
};

enum ItemIdx {
  idxColourTest = 1,
  idxTimeModeDstActive = 3,
  idxTimeModeUtcOffet = 4
};


UiMenuDigitModeSettingsMore::UiMenuDigitModeSettingsMore(RefProvider &refProvider, const char *menuTitle, DigitDisplayModeData *digitDisplayModeData) : 
  UiMenu(refProvider, menuTitle),
  digitDisplayModeData(digitDisplayModeData) {
}

void UiMenuDigitModeSettingsMore::initMenuContent() {
  addMenuItem(idxColourTest, "colour test", false, 0, itemIdentColourTest, new LerpTestSlider(refProvider, "COLOUR TEST"));  
  // remaining menu items depend on which digitMode we're dealing with
  switch (digitDisplayModeData->digitDisplayMode) {
  case DigitDisplayModeTime:
    addMenuItem(idxTimeModeDstActive, "+1hour (dst)", false, -1, itemIdentTimeModeDstActive, NULL);
    addMenuItem(idxTimeModeUtcOffet, "time zone", false, 0, itemIdentTimeModeDstActive, new UtcOffsetSlider(refProvider, "TIME ZONE"));
    break;
  case DigitDisplayModeDate:
    //todo: which one selected to come from config    
    //    addMenuItem(4, "MMDD", true, 1, digitDisplayModeDateFormatMMDD, NULL);
    //    addMenuItem(5, "DDMM", false, 1, digitDisplayModeDateFormatDDMM, NULL);
    break;
  case DigitDisplayModeTimer:
    // not sure what more options I could possibly have for timer
    break;
  }

}

void UiMenuDigitModeSettingsMore::paintMenuContent(bool bRepaint, bool bAsActive) {
  switch (digitDisplayModeData->digitDisplayMode) {
  case DigitDisplayModeTime:
    // show whether dst active
    uiMenuItems[idxTimeModeDstActive]->selected = refProvider.timeSource.getDstActive();
    break;
  case DigitDisplayModeDate:
    break;
  case DigitDisplayModeTimer:
    break;
  }
  UiMenu::paintMenuContent(bRepaint, bAsActive);
}

void UiMenuDigitModeSettingsMore::menuItemOptionChange(UiMenuItem *menuItem) {
  UiMenu::menuItemOptionChange(menuItem);
  if ( menuItem->itemIdent == itemIdentTimeModeDstActive ) {
    refProvider.timeSource.setDstActive(menuItem->selected);
  }
}

void UiMenuDigitModeSettingsMore::deactivateThyself(bool bRepaint) {
  if ( didAnythingChange(true, false) ) {
    refProvider.timeSource.saveTimeOffsetData();
  }
  UiMenu::deactivateThyself(bRepaint);
}


