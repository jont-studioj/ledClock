the folder (bla-bla/ledClock/backup/)TFT_eSPI is a copy of the library @bodmer/TFT_eSPI@^2.5.23

when I updated my libraries this library went to 2.5.43 and things went wrong (including inability to find the #define TFT_BL)

I got lucky and had a backup and overwrote the updated one

I have pasted it also here in case I lose it again.

I expect there's a proper way of selecting a particular library versions but I don't know how and am playing safe.

...2024-12-09...I suspect it's probably as easy as changing the @^2.5.43 in the lib_deps section of platformio.ini:
*****************************************************
lib_deps = 
	fastled/FastLED@^3.5.0
	paulstoffregen/Time@^1.6.1
	fbiego/ESP32Time@^2.0.0
	claws/BH1750@^1.3.0
	buelowp/sunset@^1.1.7
	bodmer/TFT_eSPI @ ^2.5.43
*****************************************************
...but I'll leave things for now until I've committed/pushed etc and sort it out later