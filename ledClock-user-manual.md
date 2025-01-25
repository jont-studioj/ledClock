# ledClock user manual

## General
The ledClock is an "internet clock", you cannot expressly set the time, it has to get the time from the internet:
- at boot-up to get the time initially
- about once an hour thereafter to check the time

For this reason the ledClock requires access to a wifi network which has access to the internet.

*Note: The ledClock only uses the 2.4GHz wifi band.*

Notes:
- immediately after boot-up, before the time is known to the clock, the digit displays will not show anything useful (and may even be a bit flickery)
- the whole thing looks a bit flakey until it has both wifi & internet connection *and* has successfully fetched the time at least once
- if there is no wifi connection then the display-panel will seem to be stuck (but the UI will still work and give you access to the menus - pretty essential as you will need to configure the wifi (unless you do it via the serial's CLI)
- the clock is hard-coded to get the time from `pool.ntp.org`

## User interface
The controls for the ledClock consist of:
- a knob
- and, oh, a plug you can yank out of the mains supply

The knob is a *rotary-encoder* it has the following capabilities:
- it turns freely (and indefinitely)
- it can be pressed, either briefly (a click) or held down
- in some situations rotating the knob while pressed is used to adjust values in bigger steps (to more quickly make adjustments to items that have a wide range of values)

<div style="page-break-after: always;"></div>

### **when not in the menu system:**
- #### normal-click (brief press & release)
  - to enter the menu system
  - to start/pause/restart a currently active timer
  - if a timer has expired then dismisses the timer alarm (stops it flashing/buzzing) and returns to the normal time display
- #### long-click (keep pressed)
  - cancel a currently active timer (whether paused or running)
- #### rotate forwards/backwards 
  - to cycle through time/date\</timer\> digit displays
    - after a timeout period of 10 seconds, the digit displays will return to either the time or a currently active timer
  - to adjust the remaining time of a currently active timer (if it's in the paused state)

### **when in the menu system:**
- #### normal-click (brief press & release)
  - to select the currently highlighted menu item, either to enter a submenu or to start adjusting some value
    - note: if the top item of a menu (its title) is highlighted then a click exits that menu (or the whole menu system itself, if at the main menu)
  - to activate one of the preset timers (on the main menu), this exits the menu system and puts the active timer into its paused state, ready for (a temporary) adjustment and/or starting
  - to finish adjusting some value
- #### long-click (keep pressed)
  - only used at the main menu when the currently highlighted item is one of the preset timers; used to enter a menu for that timer so you can adjust the timer's configured preset value & its buzzer duration
- #### rotate forwards/backwards 
  - to move the current menu highlight
  - to make adjustments to some selected value

<div style="page-break-after: always;"></div>

### Menus
#### General usage
There are various ways each menu item acts:
- enter a submenu (or exit this menu if on the menu title)
- enable/disable toggle (like a checkbox)
- enable/disable/select one of a mutually exclusive group (like an option but with ability to un-select all options)
- enable/disable toggle + submenu
  - this acts like a three-way toggle, click once to enable (if disabled), click again to enter submenu (for further options), click a third time immediately after exiting the submenu to disable (it's awkward, but it kinda works). 
  - if you navigate away from the item and return to it then the next click takes you into the submenu immediately. The item can only be disabled by clicking immediately after returning from the submenu and not navigating away.
- value adjust (like a slider)

#### Menu specifics
- #### main menu (for some reason titled "OPTIONS")
  - quick access to 5 preset timers (**t0**-**t4**); single click to activate the timer, long-press to adjust the timer's configuration (not detailed here)
    - when a timer is activated it starts in the paused state, you can adjust the timer period (without affecting the preset) by rotating the knob if required, click again to start/pause/restart timer
  - **configure >** enter the **configuration menu**
- #### configuration menu
  - 3 submenus for individual **time**/**date**/**timer** configuration menus
  - **display >** enter the display panel colour scheme menu (to choose menu colour scheme, not detailed here)
  - **system >** enter **system configuration** menu (to set brightness/wifi)

<div style="page-break-after: always;"></div>

- #### time/date/timer configuration menus - *general*:
  - **seriffed** toggle whether the digits should have serifs
  - **softened** toggle whether the digits should have soft corners
  - **effects >** enable/disable & enter digit effects menu (digit transition effects, not detailed here, just play with it)
  - **colour 1 >** submenu for start (of day/year/timer) hue & saturation
  - **colour 2 >** submenu for end (of day/year/timer) hue & saturation
    - the digit's hue/saturation at any given moment is determined by where we are in the day/year/timer, interpolating between colours 1 & 2. If you want just one colour throughout the whole period, you have to set colour 1 & 2 to the same hue & saturation values
  - **more... >**
    - **colour test** shows a slider that lets you preview the colour-1 --> colour-2 interpolation

- #### time menu --> more
  The **time menu** has extra items in the **more** menu:
    - **+1hour (dst)** toggle whether daylight savings is in force
    - **time zone** choose your time-zone (offset from UTC)

- #### system menu
  - **brightness >** enter **brightness menu**
  - **wifi >** enter **wifi** configuration menu
  - **reboot** does what is says on the tin

- #### brightness menu
  - **set min >** enter submenu to set the minimum brightness values to be used when the room is darkest, for both the digit-displays & display-panel
  - **set max >** enter submenu to set the maximum brightness values to be used when the room is brightest, for both the digit-displays & display-panel
  - **smoothing** adjust how smoothly/slowly the brightnesses change when the ambient lighting changes
  - **daylight+** enable/disable daylight boost & enter submenu to configure daylight boost values and configure how daylight is defined
 
 <div style="page-break-after: always;"></div>

- #### daylight+ menu
  *Note: adjust the digit & panel boost values actually during daylight hours, else you won't know how much to boost by, if at all*
  - **digits** adjust boost value for digit-displays
  - **panel** adjust boost value for display-panel
  - **latitude** set your location's latitude (degrees +/-ve)
  - **longitude** set your location's longitude (degrees +/-ve)
  - **sun altitude** set how far above (+ve) or below (-ve) the horizon that the sun needs to be (higher than) in order to be deemed "bright daylight"


  **A description of how the brightness system works is given further below.** 

- #### wifi menu
  This displays current wifi values and connection status, with one clickable submenu:
  - **configure >** enter submenu to configure or forget wifi settings

- #### wifi configure menu
  This all pretty self explanatory, noting that:
    - "n/w name" means the SSID of the local wifi network
    - "password" means whatever password value is used by the network
    - "my name" is the name of the ledClock and defines how it appears to the n/w (eg: in the router's list of DHCP leases)
 
<div style="page-break-after: always;"></div>

## How the brightness system works
*Note: for longevity of the LEDs (and so things don't melt) it is recommended to not run them extremely bright, best to make them as dim as you can whilst still being at an acceptable and comfortable viewing brightness*

*Note: recommended you set the digit-display brightness value first and then adjust the display-panel brightness to get a similar luminance.*

### automatic ambient adjust
You do not explicitly set a certain brightness for the digit-displays or display-panel. Instead you define how bright you want each to be when the ambient light is at its darkest and how bright you want each to be when the ambient light is at its brightest. As you set either the **minimum** or the **maximum** values the system remembers the ambient light level as measured at that moment - and also the brightness values you have defined. Once the minimum and maximum are set the system will interpolate between your lowest & highest brightness values according to the current ambient lighting conditions.

**So**:
  - set the minimums when the room is at its (expected) darkest
  - set the maximums when the room is at its (expected) brightest
  - you can go back and adjust one or the other later, should the room become much darker/brighter than before - or if you just want it brighter/darker

### daylight+ (boosts brightnesses during daylight hours)
For the most part this automatic ambient adjust works well. However there are some situations when things don't work so well... in particular if there is a window in the room which causes a strong reflection on the glass cloches making the digits difficult to discern. This is what the **daylight+** is all about - to allow you to have boosted brightness values during daylight hours.

<div style="page-break-after: always;"></div>


## serial interface & cli (for nerds)

The serial port is used for:
- displaying / setting selected configuration parameters
- view any debug log lines
- reprogramming (requires specialist knowledge & s/w)

Access to serial interface is via the usb-C connector on the main controller unit (remove the cap for access).

When using the serial port the ledClock will be powered by whatever the usb cable is connected to, eg: your PC, so:

**Always**: unplug the ledClock from the mains

**Very highly recommended**: disconnect the 5v power lead (black+red) that runs between the PSU and the distribution board (hub), otherwise the circuitry inside the PSU may act as a sink for the voltage being supplied down the usb cable from your PC. This might, just might, damage the PC but at the very least it could cause the supply voltage to drop and the LEDs may be dimmer than normal and/or cause other anomalies (don't forget to reconnect this lead afterwards).


Serial is **115200**, **8**, **1** (if you don't know what that means then you probably shouldn't be doing this)

Once the system is booted, hitting [return] should show a prompt:\
`>_`

You may see logging lines appear, depending on the state of the system - and the state of the code that runs on it.

### cli commands

There are only a few cli commands, mostly used for debugging.

In most cases where a value can be displayed and/or set, the general format is:\
`<command> [<new value>]`\
or\
`<command> <sub-command> [<new value>]`

If the `<new value>` is absent then the existing value is displayed and it does not get altered.

All commands/sub-commands are case sensitive.

`buz [on]` set the buzzer on (anything but `on` causes buzzer to stop)

`dd cc <new colour-correction value>` digit-display colour-correction; set (only, no display) LED colour correction value; format RRGGBB (or maybe GGRRBB, it might depend on the LED type, I forget). Hex pairs. Best not to touch. Was needed for leds WS2812 (and not WS2812B)

`dd cc reset` digit-display colour-correction; reset colour-correction to the default value defined for the led-type used (in case you ignored my "best not to touch" above)

<div style="page-break-after: always;"></div>

`door [<0|1>]` set door not-present/is-present; "is-present" only makes sense if the ledClock actually has a servo activating a door (in front of the display-panel), only one such ledClock exists and it probably isn't yours, so ignore this command unless you want to do something else with the servo signal. When the UI is activated the servo is given a certain defined pulse width and when the UI is de-activated the servo is given a different pulse width. When door-present is true a couple of extra menu items are added to the system-menu to allow tweaking of the servo pulse widths for door open and door closed.

`lm mode [<new mode>]` light-meter mode (exact meaning not detailed here, this is for proper nerds who will have to look at the code). Likely put in during the s/w development phase and got forgotten.

`lm mt [<new mt register value>]` light-meter MT register value (exact meaning not detailed here, this is for proper nerds who will have to look at the code). Likely put in during the s/w development phase and got forgotten.

`rst` restart (boot)

`tt tz [<new time-zone>]` the time's timezone (an index value, I think)

`tt dst [<0|1>]` whether daylight saving is in force

`tt day [<0|1>]` if true (`1`) then forces the clock to think we are in daylight regardless of the actual time

`tt reset` clear any stored time configuration (causes defaults to be used on next boot).

`ui to [<value ms>]` user-interface timeout in milliseconds, `0`=do not timeout (useful during development of UI etc)

`wifi` display wifi SSID, pwd & hostname (aka: my name, eg: "ledClock"), also shows wifi status. **Warning: password will be displayed in plaintext**

`wifi hostname [<new hostname>]` display/set how the ledClock appears to the n/w

`wifi ssid [<new ssid>]` display/set network SSID

`wifi pwd [<new password>]` display/set wifi password. **Warning: password will be displayed in plaintext**

`wifi forget` & `wifi reset` two different ways of forgetting and clearing the wifi credentials. I do not know why there are two different ways to do this, they are subtly different for reasons I cannot fathom right now.

`wifi diag` display some wifi diagnostics (not sure it's particularly revealing)
























