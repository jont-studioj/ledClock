#include "UiMenuWidgetScalarSlider.h"
#include "Sundries.h"

void UiMenuWidgetScalarSlider::paintMenuContent(bool bRepaint, bool bAsActive) {
  if ( bRepaint ) {
    drawBarBorder();
    paintValue();
  }
}

void UiMenuWidgetScalarSlider::paintValue() {
  int32_t clientValue = getClientValueClamped(0);
  uint8_t myValue = calcMyValueFromClientValue(clientValue);

  uint16_t itemFgdColour = getFgdColour(true);
  uint16_t itemBkgColour = getBkgColour(false);

  refProvider.displayPanel.fillRect(textLt, textTp, textWd, textHt, refProvider.sundries.getDisplayScheme().menuBg);
  refProvider.displayPanel.setTextColor(itemFgdColour, itemBkgColour, true);
  paintValueLabel(clientValue);

  drawBarFill(myValue);
}

void UiMenuWidgetScalarSlider::drawBarBorder() {
  uint16_t itemFgdColour = getFgdColour(true);
  refProvider.displayPanel.drawRect(barBorderLt, barBorderTp, barBorderWd, barBorderHt, itemFgdColour);
}

void UiMenuWidgetScalarSlider::drawBarFill(int8_t myValue) {
  uint16_t itemBkgColour = getBkgColour(true);
  refProvider.displayPanel.fillRect(barFillLt, barFillTp, myValue, barFillHt, itemBkgColour);
  refProvider.displayPanel.fillRect(barFillLt + myValue, barFillTp, barFillWd-myValue, barFillHt, refProvider.sundries.getDisplayScheme().menuBg);
}


UiEvent UiMenuWidgetScalarSlider::processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  int32_t oldValue = getClientValueClamped(0);
  int32_t newValue = getClientValueClamped(-1, bAltModeInput);
  setValue( menuItemSelf->itemIdent, newValue );
  paintValue();
  return highLevelEvent;
}

UiEvent UiMenuWidgetScalarSlider::processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  int32_t oldValue = getClientValueClamped(0);
  int32_t newValue = getClientValueClamped(+1, bAltModeInput);
  setValue( menuItemSelf->itemIdent, newValue );
  paintValue();
  return highLevelEvent;
}


int32_t UiMenuWidgetScalarSlider::getClientValueClamped(int32_t adjustSteps, bool bAltModeInput) {
  int32_t clientValue = getValue(menuItemSelf->itemIdent);
  int32_t stepSize = bAltModeInput ? getStepSizeFast(menuItemSelf->itemIdent) : getStepSizeSlow(menuItemSelf->itemIdent);
  clientValue += adjustSteps * stepSize;
  // Note: this code assumes client's min<max
  if ( clientValue < getRangeMinValue(menuItemSelf->itemIdent) ) {
    clientValue = getRangeMinValue(menuItemSelf->itemIdent);
  } else if ( clientValue > getRangeMaxValue(menuItemSelf->itemIdent) ) {
    clientValue = getRangeMaxValue(menuItemSelf->itemIdent);
  }
  return clientValue;
}

uint8_t UiMenuWidgetScalarSlider::calcMyValueFromClientValue(int32_t clientValue) {
  int32_t clientRangeLowerBound = getRangeLowerBound(menuItemSelf->itemIdent);
  int32_t clientRangeUpperBound = getRangeUpperBound(menuItemSelf->itemIdent);
  uint32_t clientRange = clientRangeUpperBound - clientRangeLowerBound;
  if ( clientRange == 0 ) {
    clientRange = 1;
  }
  int32_t clientOffsetValue = clientValue - clientRangeLowerBound;
  uint8_t myValue = myValueMin + 1L * ( (myValueRange * clientOffsetValue) / clientRange);
  //Serial.printf("UiMenuWidgetScalarSlider(%d): clientRange=%d, clientOffsetValue=%d, myValue=%d, myValueMin=%d, myValueMax=%d\n", clientValue, clientRange, clientOffsetValue, myValue, myValueMin, myValueMax);
  if ( myValue < myValueMin ) {  
    myValue = myValueMin;
  } else if ( myValue > myValueMax ) {  
    myValue = myValueMax;
  }
  return myValue;
}
