#include "UiMenuWidgetDblHueSelector.h"
#include "FastLED.h"
#include "Sundries.h"

void UiMenuWidgetDblHueSelector::paintMenuContent(bool bRepaint, bool bAsActive) {
  oldCursorDblHue = -1;
  if ( bRepaint ) {
    drawRainbowRows();
    paintValue();
  }
}

void UiMenuWidgetDblHueSelector::paintValue() {
  int32_t clientValue = getClientValueClamped(0);

  uint16_t itemFgdColour = getFgdColour(true);
  uint16_t itemBkgColour = getBkgColour(false);

  refProvider.displayPanel.fillRect(textLt, textTp, textWd, textHt, refProvider.sundries.getDisplayScheme().menuBg);
  refProvider.displayPanel.setTextColor(itemFgdColour, itemBkgColour, true);
  paintValueLabel(clientValue);

  drawCursorForDblHue(clientValue);
}

void UiMenuWidgetDblHueSelector::drawCursorForDblHue(int16_t dblHue) {
  if ( oldCursorDblHue != -1 ) {
    paintCursorForDblHue(oldCursorDblHue, refProvider.sundries.getDisplayScheme().menuBg);
    oldCursorDblHue = -1;
  }
  if ( dblHue != -1 ) {
    paintCursorForDblHue(dblHue, TFT_WHITE);
    oldCursorDblHue = dblHue;
  }
}
void UiMenuWidgetDblHueSelector::paintCursorForDblHue(int16_t dblHue, uint32_t color) {
  uint8_t ltX;
  uint8_t rtX;
  if ( dblHue < 256 ) {
    ltX = cursorLoLtX;
    rtX = cursorLoRtX;
  } else {
    ltX = cursorHiLtX;
    rtX = cursorHiRtX;
    dblHue = dblHue - 256;
  }

  uint16_t displayY = rainbowY + ((myRowMax * dblHue) / 255) -1;

  refProvider.displayPanel.fillRect(ltX, displayY, cursorWd, 3, color);
  refProvider.displayPanel.fillRect(rtX, displayY, cursorWd, 3, color);
}

UiEvent UiMenuWidgetDblHueSelector::processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  int32_t oldValue = getClientValueClamped(0);
  int32_t newValue = getClientValueClamped(-1, bAltModeInput);
  if ( newValue != oldValue ) {
    setValue( menuItemSelf->itemIdent, newValue );
    paintValue();
  }
  return highLevelEvent;
}

UiEvent UiMenuWidgetDblHueSelector::processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  int32_t oldValue = getClientValueClamped(0);
  int32_t newValue = getClientValueClamped(+1, bAltModeInput);
  if ( newValue != oldValue ) {
    setValue( menuItemSelf->itemIdent, newValue );
    paintValue();
  }
  return highLevelEvent;
}


int32_t UiMenuWidgetDblHueSelector::getClientValueClamped(int32_t adjustSteps, bool bAltModeInput) {
  int32_t clientValue = getValue(menuItemSelf->itemIdent);
  int32_t stepSize = bAltModeInput ? getStepSizeFast(menuItemSelf->itemIdent) : getStepSizeSlow(menuItemSelf->itemIdent);
  clientValue += adjustSteps * stepSize;
  // Note: this code assumes min<max
  if ( clientValue < dblHueLowerBound ) {
    clientValue = dblHueLowerBound;
  } else if ( clientValue > dblHueUpperBound ) {
    clientValue = dblHueUpperBound;
  }
  return clientValue;
}

void UiMenuWidgetDblHueSelector::drawRainbowRows() {
  for (uint8_t row = myValueMin; row <= myRowMax; row++) {
    drawRainbowRow(row);
  }
  //refProvider.displayPanel.drawFastHLine(0, TFT_HEIGHT-1, TFT_WIDTH, TFT_WHITE);
}

void UiMenuWidgetDblHueSelector::drawRainbowRow(uint8_t row) {
  uint16_t rgb16 = rgb16FromRow(row);
  //Serial.printf("drawRainbowRow(%d):rgb16=0x%08x\n", row, rgb16);
  int32_t displayRow = rainbowY + row;
  refProvider.displayPanel.drawFastHLine(rainbowLoX, displayRow, rainbowWd, rgb16);
  refProvider.displayPanel.drawFastHLine(rainbowHiX, displayRow, rainbowWd, rgb16);
}

uint16_t UiMenuWidgetDblHueSelector::rgb16FromRow(uint8_t row) {
  uint8_t hue = (row * 255) / myRowMax;
  CRGB rgb;
  rgb.setHSV(hue & 0xff, 255, 255);
  uint16_t rgb16 = ((rgb.r & 0xF8) << 8) | ((rgb.g & 0xFC) << 3) | (rgb.b >> 3);
  return rgb16;
}
