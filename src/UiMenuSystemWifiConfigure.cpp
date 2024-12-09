#include "UiMenuSystemWifiConfigure.h"
#include "UiMenuWidgetTextEdit.h"
#include "UiMenuSystemWifiForget.h"
#include "WifiExecutive.h"


class EditSSID : public UiMenuWidgetTextEdit  {
public:
  using UiMenuWidgetTextEdit::UiMenuWidgetTextEdit;
private:
  String getTextValue() {
    return refProvider.wifiExecutive.haveSSID() ? refProvider.wifiExecutive.getWifiSSID() : "";
  }
  void setTextValue(String newValue) { refProvider.wifiExecutive.setWifiSSID(newValue); }
};

class EditPWD : public UiMenuWidgetTextEdit  {
public:
  using UiMenuWidgetTextEdit::UiMenuWidgetTextEdit;
private:
  String getTextValue() {
    return refProvider.wifiExecutive.havePWD() ? refProvider.wifiExecutive.getWifiPWD() : "";
  }
  void setTextValue(String newValue) { refProvider.wifiExecutive.setWifiPWD(newValue); }
};

class EditHostname : public UiMenuWidgetTextEdit  {
public:
  using UiMenuWidgetTextEdit::UiMenuWidgetTextEdit;
private:
  String getTextValue() { return refProvider.wifiExecutive.getWifiHostname(); }
  void setTextValue(String newValue) { refProvider.wifiExecutive.setWifiHostname(newValue); }
};

enum ItemIdx {
  idxSetSSID = 1,
  idxSetPWD = 2,
  idxSetMyName = 4,
  idxForgetNW = 6
};

void UiMenuSystemWifiConfigure::initMenuContent() {
  addMenuItem(idxSetSSID, "n/w name", false, 0, 0, new EditSSID(refProvider, "N/W NAME"));
  addMenuItem(idxSetPWD, "password", false, 0, 0, new EditPWD(refProvider, "PASSWORD"));
  addMenuItem(idxSetMyName, "my name", false, 0, 0, new EditHostname(refProvider, "MY NAME"));
  addMenuItem(idxForgetNW, "forget n/w", false, 0, 0, new UiMenuSystemWifiForget(refProvider, "FORGET N/W"));
}
