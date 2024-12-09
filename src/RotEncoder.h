#ifndef ROT_ENCODER_H
#define ROT_ENCODER_H

#include <Arduino.h>

typedef enum {
	BUT_PRESSED_STATE_UNKNOWN	= 0,
	BUT_PRESSED_STATE_UP 			= 1,
	BUT_PRESSED_STATE_DOWN 		= 2
} ButtonPressedState;

typedef enum {
	BUT_EVENT_NONE						= 0,
	BUT_EVENT_PRESSED		 			= 1,
	BUT_EVENT_RELEASED				= 2
} ButtonEvent;

class RotEncoder;

static void IRAM_ATTR readEncoderISR();
static RotEncoder *rotEncoderInstance;

class RotEncoder {

private:
	

	// static void IRAM_ATTR readEncoderISR() {
  //   rotEncoderInstance->readEncoder_ISR();
	// }
  
	void setup(
		uint8_t encoder_APin, uint8_t encoder_APinMode, 
		uint8_t encoder_BPin, uint8_t encoder_BPinMode,
		uint8_t encoder_ButtonPin, uint8_t encoder_ButtonPinMode,
		uint8_t encoder_VccPin,
		uint8_t encoder_Steps,
		void (*ISR_rotation)(void), void (*ISR_button)(void)
	);

	portMUX_TYPE rotationMux = portMUX_INITIALIZER_UNLOCKED;
	portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;

	volatile long encoder0Pos = 0;

	volatile int8_t lastMovementDirection = 0; //1 right; -1 left
	volatile unsigned long lastMovementAt = 0;
	unsigned long rotaryAccelerationCoef = 150;

	bool _circleValues = false;
	bool bEnabled = true;

	uint8_t encoderAPin;// = AIESP32ROTARYENCODER_DEFAULT_A_PIN;
	uint8_t encoderBPin;// = AIESP32ROTARYENCODER_DEFAULT_B_PIN;
	uint8_t encoderButtonPin = 0;// = AIESP32ROTARYENCODER_DEFAULT_BUT_PIN;
	uint8_t encoderVccPin = 0;// = AIESP32ROTARYENCODER_DEFAULT_VCC_PIN;
	uint8_t encoderSteps = 1;//AIESP32ROTARYENCODER_DEFAULT_STEPS;

	long _minEncoderValue = -1 << 15;
	long _maxEncoderValue = 1 << 15;

	uint8_t old_AB;
	long lastReadEncoder0Pos;

	volatile int isr_button_pin_state_prev;
	volatile unsigned long isr_button_pin_state_change_millis;

	const uint8_t DEBOUNCE_THRESHOLD_MS = 50;

	ButtonPressedState buttonStateNow;
	ButtonPressedState buttonStateAtLastReportedEvent;
	ButtonEvent buttonEventReported;

	int8_t enc_states[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
//	void (*ISRrotation)();
//	void (*ISRbutton)();

public:
  static RotEncoder *getInstance(
    uint8_t encoder_APin, uint8_t encoder_APinMode, 
		uint8_t encoder_BPin, uint8_t encoder_BPinMode,
		uint8_t encoder_ButtonPin, uint8_t encoder_ButtonPinMode,
		uint8_t encoder_VccPin,
		uint8_t encoder_Steps);

	unsigned long getAcceleration() { return rotaryAccelerationCoef; }
	void setAcceleration(unsigned long acceleration) { rotaryAccelerationCoef = acceleration; }
	void disableAcceleration() { setAcceleration(0); }
	void setBoundaries(long minValue = -100, long maxValue = 100, bool circleValues = false);

	void enable();
	void disable();
	void reset(long newValue = 0);	

	long readEncoder();
	void setEncoderValue(long newValue);
	long encoderChanged();

	ButtonPressedState getButtonPressedState();
	ButtonEvent getButtonEvent();


	void IRAM_ATTR readEncoder_ISR();
	void IRAM_ATTR readButton_ISR();

};
#endif	//ROT_ENCODER_H
