#ifndef UIenums_H
#define UIenums_H

enum class UiState {
  usIdleEnabled,
  usIdleDisabled,
  usActive
  // uisMidAnimation
};

//                     [button / encoder / menu-item] action description
enum class UiEvent {
  // external / when ui is inactive or disabled (or activating/deactivating)
  ueNoEvent,
  ueUiStarted,        // [mode / click]                 (when ui usIdleEnabled) just started UI-menu activity (went to usActive)
  ueUiStopped,        // [power / n/a / "<exit>"]       (when ui usActive)      just stopped UI-menu activity (went to usIdleEnabled)
  uePowerToggle,      // [power / n/a]                  (when ui usIdleEnabled) toggle power (sort of, it really just powers the displays down/up)
  ueCancel,           // [power / n/a]                  (when ui usIdleDisabled) treat as: cancel alternate operation
  // external or internal (depending on uiState)
  ueDecrease,         // [<<<< / rotate-ccw]            (when ui != usActive) do decrease-action on alternate operation
  ueIncrease,         // [>>>> / rotate-cw]             (when ui != usActive) do ueIncrease-action on alternate operation
  ueAltDecrease,      // [mode+<<<< / click+rotate-ccw] (when ui != usActive) do alt-decrease-action on alternate operation
  ueAltIncrease,      // [mode+>>>> / click+rotate-cw]  (when ui != usActive) do alt-ueIncrease-action on alternate operation
  ueAccept,           // [mode / click]                 (when ui usIdleDisabled) treat as: enter/accept on alternate operation
  ueAltAccept,        // long click
  ueAltModeBegin,
  ueAltModeEnd,
//  ueValueChange,      // [various]                      (when ui usActive) some value changed, go ask menus/menuItems to try find out what it was
  // internal only
  ueMenuActivating,   // [mode / click]                 (when ui usActive)
  ueMenuDeactivating, // [power / n/a / "<exit>"]       exit current menu or all menu/ui activities (triggering uiEnded)
  ueShutDownUI

//  ueKludgeCheckboxSelected      // [various]                      (when ui usActive) some value changed, go ask menus/menuItems to try find out what it was
};

enum class UiLowLevelEvent {
  ulleNone,
//TODO: this looks like leftover from ips-clock and not relevant in this project  
  ulleExit,
  ulleEnter,      // button press/release
  ulleAltEnter,   // button long press/release
  ulleLeft,
  ulleRight
};

enum class UiLowLevelEventSource {
  ullesNone,
  ullesIdleTimeout,
  ullesButton,
  ullesEncoder,
  ullesCode
};

#endif // UIenums_H
