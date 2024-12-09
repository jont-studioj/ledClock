#ifndef UIMENU_WIDGET_SCALAR_SLIDER_H
#define UIMENU_WIDGET_SCALAR_SLIDER_H

#include "UiMenuWidget.h"
#include "DisplayPanel.h"

class UiMenuWidgetScalarSlider: public UiMenuWidget {
public:
  using UiMenuWidget::UiMenuWidget;

  void paintMenuContent(bool bRepaint, bool bAsActive);
  UiEvent processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput = false);
  UiEvent processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput = false);

protected:
  virtual int32_t getRangeLowerBound(int itemIdent) { return 0; }   // define scalar's range lower value, eg: 0       appears like: [        ]
  virtual int32_t getRangeUpperBound(int itemIdent) { return 1; }   // define scalar's range upper value, eg: 127     appears like: [########]
  virtual int32_t getRangeMinValue(int itemIdent) { return 0; }     // defines minimum value we can step to, eg: 15   appears like: [#       ]
  virtual int32_t getRangeMaxValue(int itemIdent) { return 1; }     // defines maximum value we can step to, eg: 123  appears like: [####### ]
  virtual int32_t getStepSizeSlow(int itemIdent) { return 1; }      // define scalar's range stepping value, eg: 1
  virtual int32_t getStepSizeFast(int itemIdent) { return 2; }      // define scalar's range stepping value, eg: 16

  virtual int32_t getValue(int itemIdent) { return 0; }
  virtual int32_t setValue(int itemIdent, int32_t newValue) { return getValue(menuItemSelf->itemIdent); }

  virtual void paintValueLabel(int32_t value) {
    // default label is numeric value centre-justified
    refProvider.displayPanel.setTextDatum(TC_DATUM);
    refProvider.displayPanel.drawNumber(value, textLt + (textWd / 2), textTp, fontNo);
    refProvider.displayPanel.setTextDatum(TL_DATUM);
  }

  static const uint8_t textLt = 0;
  static const uint8_t textWd = TFT_WIDTH;
  static const uint8_t textTp = clientTop + 2;
  static const uint8_t textHt = 26;

  static const uint8_t fontNo = 4;

private:

  static const uint8_t barBorderLt = 3;
  static const uint8_t barBorderWd = 129;
  static const uint8_t barBorderTp = textTp + textHt + 4;
  static const uint8_t barBorderHt = 30;

  static const uint8_t barFillLt = barBorderLt + 1;
  static const uint8_t barFillWd = barBorderWd - 2;
  static const uint8_t barFillTp = barBorderTp + 1;
  static const uint8_t barFillHt = barBorderHt - 2;

  static const uint8_t myValueMin = 0;
  static const uint8_t myValueMax = 127;
  static const uint8_t myValueRange = myValueMax - myValueMin;


  void paintValue();
  void drawBarBorder();
  void drawBarFill(int8_t value);

  int32_t getClientValueClamped(int32_t adjustSteps, bool bAltModeInput = false);
  uint8_t calcMyValueFromClientValue(int32_t clientValue);

};

#endif // UIMENU_WIDGET_SCALAR_SLIDER_H
