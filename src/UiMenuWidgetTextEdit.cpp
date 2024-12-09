#include "UiMenuWidgetTextEdit.h"
#include "DisplayPanel.h"
#include "Sundries.h"

void UiMenuWidgetTextEdit::paintMenuContent(bool bRepaint, bool bAsActive) {
  UiMenu::paintMenuContent(bRepaint, bAsActive);
  //Serial.printf("UiMenuWidgetTextEdit::paintMenuContent() textWd=%d\n", textWd);
  paintCurrentText();
  paintCharacterSelector(selectorIdx);

}

void UiMenuWidgetTextEdit::paintCurrentText() {
  //Serial.println("UiMenuWidgetTextEdit::paintCurrentText()");

  bool bShowCaret = ( sectionFocus == sfSelector );
    
  uint16_t fgdColourLo = getFgdColour(false);
  uint16_t fgdColourHi = getFgdColour(true);
  uint16_t bkgColour = getBkgColour(false);
  
  uint16_t focusRingColour = fgdColourLo;

  TFT_eSprite sprite = TFT_eSprite(&refProvider.displayPanel);
  sprite.createSprite(textWd, textHt);
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextFont(textFontNo);
  sprite.setTextWrap(false, false);

  String displayText = textVal;
  int8_t idxFocusChar = -1;
  if ( bShowCaret ) {
    displayText = displayText + " ";
    idxFocusChar = textVal.length();
  }

  highlightedSpritePaint(sprite, textFontNo,
    textLt, textTp,
    true, false,
    bkgColour, fgdColourLo, fgdColourHi, focusRingColour,
    displayText, idxFocusChar
  );

  sprite.deleteSprite();
}

void UiMenuWidgetTextEdit::paintCharacterSelector(int8_t idxCurrentChar) {

  bool bHaveFocus = (sectionFocus == sfSelector);

  uint16_t fgdColourLo = bHaveFocus ? getFgdColour(true) : getBkgColour(true);
  uint16_t fgdColourHi = getFgdColour(true);
  uint16_t bkgColour = getBkgColour(false);

  uint16_t focusRingColour = refProvider.sundries.getDisplayScheme().focusRing;

  TFT_eSprite sprite = TFT_eSprite(&refProvider.displayPanel);
  sprite.createSprite(selectorWd, selectorHt);
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextFont(selectorFontNo);
  sprite.setTextWrap(true, false);

  highlightedSpritePaint(sprite, selectorFontNo,
    selectorLt, selectorTp,
    false, true,
    bkgColour, fgdColourLo, fgdColourHi, focusRingColour,
    selectorVal, idxCurrentChar
  );

  sprite.deleteSprite();
}

UiEvent UiMenuWidgetTextEdit::processMenuEnter(UiLowLevelEventSource lowLevelEventSource, bool bLongClick) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;
  
  switch (sectionFocus) {
  case sfMenu:
    highLevelEvent = UiMenuWidget::processMenuEnter(lowLevelEventSource, bLongClick);
    break;
  case sfSelector:
    if ( selectorIdx >= 0 ) {
      //String sSelectedChar = selectorVal.substring(selectorIdx, selectorIdx+1);
      char chr = selectorVal.c_str()[selectorIdx];
      if ( bLongClick ) {
        if ( chr == del ) {
          // delete all
          // if we're already zero length then treat as end editing
          if ( textVal.length() == 0 ) {
            endEditing();
          } else {
            textVal = "";
            paintCurrentText();
          }
        } else {
          // long click on not del interpreted as finish edit
          endEditing();
        }
      } else {
        // normal click
        if ( chr == del ) {
          // delete single char (backspace)
          if ( textVal.length() > 0 ) {
            textVal = textVal.substring(0, textVal.length()-1);
          }
        } else {
          textVal = textVal + String(chr);
        }

        // update text display, leave selector at its current position
        paintCurrentText();
      }//endif/else long click
    }//endif selectorIdx >= 0
  }//end switch

  return highLevelEvent;
}

UiEvent UiMenuWidgetTextEdit::processMenuDecrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  switch (sectionFocus) {
  case sfMenu:
    break;
  case sfSelector:
    if ( !bAltModeInput && (selectorIdx <= 0) ) {
      endEditing();
    } else {
      navigateSelector(bAltModeInput ? -5 : -1);
    }
    break;
  }

  return highLevelEvent;
}

UiEvent UiMenuWidgetTextEdit::processMenuIncrease(UiLowLevelEventSource lowLevelEventSource, bool bAltModeInput) {
  UiEvent highLevelEvent = UiEvent::ueNoEvent;

  switch (sectionFocus) {
  case sfMenu:
    startEditing();
    break;
  case sfSelector:
    navigateSelector(bAltModeInput ? +5 : +1);
    break;
  }

  return highLevelEvent;
}

void UiMenuWidgetTextEdit::navigateSelector(int8_t step) {
  selectorIdx += step;
  if ( selectorIdx < 0 ) {
    selectorIdx = 0;
  } else if ( selectorIdx >= selectorVal.length()-1 ) {
    selectorIdx = selectorVal.length()-1;
  }
  paintCharacterSelector(selectorIdx);
}

void UiMenuWidgetTextEdit::highlightedSpritePaint(TFT_eSprite &sprite, uint8_t fontNo,
    int16_t dispLocX, int16_t dispLocY,
    bool bScrollX, bool bScrollY,
    int16_t bkgColour, uint16_t fgdColourLo, uint16_t fgdColourHi, uint16_t focusRingColour, 
    String text, int8_t idxFocusChar) {

  sprite.fillSprite(bkgColour);
  sprite.setViewport(2, 2, sprite.width()-4, sprite.height()-4);

  sprite.setCursor(0, 0);

  bool bFoundHightlightChar = false;
  uint16_t hlcLT;
  uint16_t hlcTP;
  uint8_t hlcWD;
  uint8_t hlcHT = sprite.fontHeight(fontNo);

  int16_t rtMost = -1;
  int16_t btMost = -1;
    
  for ( uint8_t idx = 0; idx < text.length(); idx++ ) {
    //String sChar = text.substring(idx, idx+1);
    char chr = text.c_str()[idx];
    if ( idx == idxFocusChar ) {
      sprite.setTextColor(fgdColourHi, bkgColour, true);
      // remember the top-left position - and the width/height of the highlighted char
      bFoundHightlightChar = true;
      hlcLT = sprite.getCursorX();
      hlcTP = sprite.getCursorY();

      hlcWD = spritePrintChar(sprite, hlcHT, bkgColour, fgdColourHi, chr);

      //Serial.printf("A: hlcWD=%d, sprite.textWidth(\"%s\")=%d\n", hlcWD, sChar, sprite.textWidth(sChar));    

      // if cursorY has changed then this means that we wrapped to next line
      // so we need to re-calc our character bounds
      uint16_t newhlcTP = sprite.getCursorY();
      if ( newhlcTP != hlcTP ) {
        hlcTP = newhlcTP;
        hlcLT = 0;
      }
    } else {
      sprite.setTextColor(fgdColourLo, bkgColour, true);
      spritePrintChar(sprite, hlcHT, bkgColour, fgdColourLo, chr);
    }
  }

  sprite.resetViewport();

  //Serial.printf("shchar[%s] hlcLT=%d, hlcTP=%d, hlcWD=%d, hlcHT=%d\n", shchar, hlcLT, hlcTP, hlcWD, hlcHT);

  if ( bFoundHightlightChar ) {
    hlcWD +=4;
    hlcHT +=4;
    //Serial.printf("bFoundHightlightChar=%d, hlcWD=%d\n", bFoundHightlightChar, hlcWD);
    sprite.drawRect(hlcLT, hlcTP, hlcWD, hlcHT, focusRingColour);
    rtMost = hlcLT + hlcWD;
    btMost = hlcTP + hlcHT;
  }

  int32_t sx = 0;
  int32_t sy = 0;
  int32_t sw = TFT_WIDTH - dispLocX;
  int32_t sh = TFT_HEIGHT - dispLocY;
  //Serial.printf("rtMost=%d, btMost=%d\n", rtMost, btMost);
  if ( bScrollX && (rtMost >= sw) ) {
    sx = rtMost - sw;
  } 
  if ( bScrollY && (btMost >= sh) ) {
    sy = btMost - sh;
  }
  //Serial.printf("dispLocX=%d, dispLocY=%d, sx=%d, sy=%d, sw=%d, sh=%d\n", dispLocX, dispLocY, sx, sy, sw, sh);
  sprite.pushSprite(dispLocX, dispLocY, sx, sy, sw, sh);
}

int8_t UiMenuWidgetTextEdit::spritePrintChar(TFT_eSprite &sprite, int8_t fontHt, int16_t bkgColour, uint16_t fgdColour, char c) {
  int8_t retVal;
  //Serial.printf("c[%d], del[%d]\n", c, del);
  if ( c == del ) {
    //Serial.println("del");
    int16_t tlx = sprite.getCursorX()+1;
    int16_t tly = sprite.getCursorY()+1;
    int8_t wd = fontHt / 2;
    int8_t ht = fontHt - 4;
    //    sprite.fillRect(tlx, tly, wd, ht, fgdColour);
    //    sprite.drawLine(tlx, tly, tlx+wd, tly+ht, bkgColour);
    //    sprite.drawLine(tlx+wd, tly, tlx, tly+fontHt, bkgColour);
    sprite.fillTriangle(tlx, tly+(ht/2)+1, tlx+wd, tly+3, tlx+wd, tly+ht-1, fgdColour);
    retVal = wd+3;
    sprite.setCursor(tlx + retVal+2, tly);
  } else {
    sprite.print(c);
    retVal = sprite.textWidth(String(c));
    //Serial.printf("[%s]=%d\n", String(c), retVal);
  }
  return retVal;
}

void UiMenuWidgetTextEdit::moveFocus(SectionFocus newSection) {
  //Serial.println("moveFocus()");
  sectionFocus = newSection;
  // if menu then refocus menu item
  // else hide menu item focus
  if ( sectionFocus == sfMenu ) {
    gainItemFocus(true, 0);
  } else {
    loseItemFocus(true);
  }
}

void UiMenuWidgetTextEdit::startEditing() {
  //Serial.println("startEditing()");  
  moveFocus(sfSelector);
  if ( selectorIdx == -1 ) {
    selectorIdx = 0;
  }
  paintCurrentText();
  paintCharacterSelector(selectorIdx);
}

void UiMenuWidgetTextEdit::endEditing() {
  //Serial.println("endEditing()");    
  moveFocus(sfMenu);
  selectorIdx = -1;
  paintCurrentText();
  paintCharacterSelector(selectorIdx);
}

void UiMenuWidgetTextEdit::activateThyself(bool bRepaint) {
  //Serial.println("activateThyself()");
  selectorVal = del + getValidCharacters();
  textVal = getTextValue();
  textVal.trim();
  selectorIdx = -1;
  moveFocus(sfMenu);  
  UiMenuWidget::activateThyself(bRepaint);
}
void UiMenuWidgetTextEdit::deactivateThyself(bool bRepaint) {
  textVal.trim();
  setTextValue(textVal);
  UiMenuWidget::deactivateThyself(bRepaint);
}
