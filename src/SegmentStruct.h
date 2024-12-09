#ifndef SEGMENTSTRUCT_H
#define SEGMENTSTRUCT_H
#include <Arduino.h>

/*
    led layout/offsets

    13 12 11 10 09 08 
    14             07 
    15             06 
    16             05
<-- 17 29 28 27 26 04 
    18             03 
    19             02 
    20             01 
    21 22 23 24 25 00 <-- 

*/

#define QTY_SEGMENTS 7

typedef struct {
  uint16_t offset;      // LED index offset of LEDs (within a digit)
  uint8_t qtyLeds;      // number of LEDs in this segment
  int8_t dir;           // -1, 0, +1 indicating direction
} SingleSegmentStruct;
// direction
//   -1 = segment is defined in a notional "backwards" direction
//    0 = segment has no significant direction (probably would only make sense if there was just 1 led (or very few leds)
//   +1 = segment is defined in a notional "forwards" direction
//  
// notional forwards =   left to right, top to bottom; offset represents left-most or top-most led of this segment
// notional backwards =  right to left, bottom to top; offset represents right-most or bottom-most led of this segment

typedef struct {
  uint16_t              startingLedIndex;       // index of start of 1st segment
} DigitDefStruct;

#endif// SEGMENTSTRUCT_H