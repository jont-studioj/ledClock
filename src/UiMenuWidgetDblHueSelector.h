#ifndef UIMENU_WIDGET_DBLHUE_SELECTOR_H
#define UIMENU_WIDGET_DBLHUE_SELECTOR_H

#include "UiMenuWidget.h"
#include "DisplayPanel.h"

class UiMenuWidgetDblHueSelector: public UiMenuWidget {
public:
  using UiMenuWidget::UiMenuWidget;

  void paintMenuContent(bool bRepaint, bool bAsActive);
  UiEvent processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);
  UiEvent processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);

protected:
  virtual int32_t getStepSizeSlow(int itemIdent) { return 1; }    // define scalar's range stepping value, eg: 1
  virtual int32_t getStepSizeFast(int itemIdent) { return 2; }    // define scalar's range stepping value, eg: 16

  virtual int32_t getValue(int itemIdent) { return 0; }
  virtual int32_t setValue(int itemIdent, int32_t newValue) { return getValue(menuItemSelf->itemIdent); }

  virtual void paintValueLabel(int32_t value) {
    // default label is numeric value centre-justified
    refProvider.displayPanel.setTextDatum(TC_DATUM);
    refProvider.displayPanel.drawNumber(value, textLt + (textWd / 2), textTp, fontNo);
    refProvider.displayPanel.setTextDatum(TL_DATUM);
  }


private:
  static const uint8_t textLt = 0;
  static const uint8_t textWd = TFT_WIDTH;
  static const uint8_t textTp = clientTop;
  static const uint8_t textHt = 26;

  static const uint8_t fontNo = 4;

  static const uint16_t maxHueVal = 256*2;

  static const uint8_t rainbowY = textTp + textHt + 2;
  static const uint8_t rainbowWd = 30;
  static const uint8_t rainbowLoX = 20;
  static const uint8_t rainbowHiX = TFT_WIDTH - rainbowLoX - rainbowWd;

  static const uint8_t cursorWd = 10;
  static const uint8_t cursorLoLtX = rainbowLoX - cursorWd -1;
  static const uint8_t cursorLoRtX = rainbowLoX + rainbowWd + 1;
  static const uint8_t cursorHiLtX = rainbowHiX - cursorWd -1;
  static const uint8_t cursorHiRtX = rainbowHiX + rainbowWd + 1;

  static const uint16_t myValueMin = 0;
  static const uint16_t myValueMax = 359;
  static const uint16_t myRowMax = 179;
  
  static const uint16_t myValueRange = myValueMax - myValueMin;

  static const uint16_t dblHueLowerBound = 0;
  static const uint16_t dblHueUpperBound = 511;
  
  void paintValue();
  void drawCursorForDblHue(int16_t dblHue);
  void paintCursorForDblHue(int16_t dblHue, uint32_t color);

  int32_t getClientValueClamped(int32_t adjustSteps, bool bAltModeInput = false);

  uint16_t rgb16FromRow(uint8_t row);


  void drawRainbowRows();
  void drawRainbowRow(uint8_t row);

  int16_t oldCursorDblHue;

};

#endif // UIMENU_WIDGET_DBLHUE_SELECTOR_H
