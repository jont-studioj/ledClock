#ifndef GLOBAL_DEFINES_H_
#define GLOBAL_DEFINES_H_

#include <stdint.h>
#include <Arduino.h>

#define DISPLAY_BACKLIGHT_PWM_FREQ 2000
#define DISPLAY_BACKLIGHT_PWM_LED_CHANNEL 0
#define DISPLAY_BACKLIGHT_PWM_RESOLUTION 12

//had to add this line after updating libraries...
// bodmer/TFT_eSPI@^2.5.23 
// -->
// bodmer/TFT_eSPI@^2.5.43
// and it went missing
// 4 allows some display but doesn't update afterwards
// 38 - no display at all
// now trying 12 - no display at all
//#define TFT_BL 12
//#define DISPLAY_BACKLIGHT_PWM_DIMMABLE true

#define DISPLAY_BACKLIGHT_PWM_PIN TFT_BL  // display back-light control pin


// display door servoe (pwm)
#define DISPLAY_DOOR_PWM_SERVO_PIN 33
//note: cannot share same timer as channel 0, 1,4 we're safe with channel 2
#define DISPLAY_DOOR_PWM_CHANNEL 2
#define DISPLAY_DOOR_PWM_FREQUENCY 50
#define DISPLAY_DOOR_PWM_TIMER_RESOLUTION 16
#define DISPLAY_DOOR_PWM_UNIT_COUNT_PER_FRAME (1 << DISPLAY_DOOR_PWM_TIMER_RESOLUTION)
#define DISPLAY_DOOR_PWM_UNIT_COUNT_PER_mS    (DISPLAY_DOOR_PWM_UNIT_COUNT_PER_FRAME * DISPLAY_DOOR_PWM_FREQUENCY / 1000)
#define DISPLAY_DOOR_PWM_TICKS_MIN            (DISPLAY_DOOR_PWM_UNIT_COUNT_PER_mS * 1)
#define DISPLAY_DOOR_PWM_TICKS_MAX            (DISPLAY_DOOR_PWM_UNIT_COUNT_PER_mS * 2)

#define DISPLAY_DOOR_PWM_MIN 3276
#define DISPLAY_DOOR_PWM_MAX DISPLAY_DOOR_PWM_MIN * 2

// buzzer (pwm)
#define BUZZER_PWM_PIN 32
//note: cannot share same timer as channel 0, 1, 2 we're safe with channel 4
#define BUZZER_PWM_CHANNEL 4
#define BUZZER_PWM_FREQUENCY 4
#define BUZZER_PWM_TIMER_RESOLUTION 8
//resolution 8 --> 0-255 duty


// todo: these maybe belong in digitdisplay
#define LED_PIN     25
//#define LED_TYPE    WS2812B
//#define COLOR_ORDER GRB

#define QTY_DIGITS 4
#define QTY_LEDS_PER_DIGIT 30
#define QTY_LEDS QTY_DIGITS * QTY_LEDS_PER_DIGIT


#define UPDATES_PER_SECOND 160


#define ROT_ENCODER_BUTTON  36
#define ROT_ENCODER_B       37
#define ROT_ENCODER_A       38
#define ROT_ENCODER_VCC     0
#define ROT_ENCODER_STEPS   1



#endif /* GLOBAL_DEFINES_H_ */
