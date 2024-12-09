#ifndef UIMENU_WIDGET_TEXT_ENTRY_H
#define UIMENU_WIDGET_TEXT_ENTRY_H

#include "UiMenuWidget.h"
#include "DisplayPanel.h"

enum SectionFocus {
  sfMenu,
  sfSelector
};

class UiMenuWidgetTextEdit: public UiMenuWidget {
public:
  using UiMenuWidget::UiMenuWidget;

  void paintMenuContent(bool bRepaint, bool bAsActive);

  UiEvent processMenuEnter(UiLowLevelEventSource lowLevelEventSource, bool bLongClick);
  UiEvent processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);
  UiEvent processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput);

protected:
  virtual String getTextValue() { return ""; }
  virtual void setTextValue(String newValue) { };
  virtual String getValidCharacters() { return " abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ,.:;!?\"#$%&@'~-+*/=0123456789()<>^\\[]{}|`"; }

private:
  String textVal;

  static const uint16_t textLt = 0;
  static const uint16_t textWd = TFT_WIDTH * 2;
  static const uint16_t textTp = clientTop;
  static const uint16_t textHt = 30;
  static const uint8_t textFontNo = 4;

  String selectorVal;

  static const uint16_t selectorLt = 0;
  static const uint16_t selectorWd = TFT_WIDTH;
  static const uint16_t selectorTp = textTp + textHt + 2;
  static const uint16_t selectorHt = TFT_HEIGHT * 1.5;
  static const uint8_t selectorFontNo = 4;
  int8_t selectorIdx;

  const char del = 0x7f;

  SectionFocus sectionFocus;

  void paintCurrentText();
  void paintCharacterSelector(int8_t idxCurrentChar);

  void moveFocus(SectionFocus newSection);

  void startEditing();
  void endEditing();

  void activateThyself(bool bRepaint);
  void deactivateThyself(bool bRepaint);

  void navigateSelector(int8_t step);

  void highlightedSpritePaint(TFT_eSprite &sprite, uint8_t fontNo,
    int16_t dispLocX, int16_t dispLocY,
    bool bScrollX, bool bScrollY,
    int16_t bkgColour, uint16_t fgdColourLo, uint16_t fgdColourHi, uint16_t focusRingColour, 
    String text, int8_t idxFocusChar);

  int8_t spritePrintChar(TFT_eSprite &sprite, int8_t fontHt, int16_t bkgColour, uint16_t fgdColour, char c);
};

#endif // UIMENU_WIDGET_TEXT_ENTRY_H
